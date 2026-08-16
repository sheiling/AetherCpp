/**
 * @file   memory_helper.cpp
 * @brief  MemoryHelper 实现 — 基于 Windows CRT 调试堆
 *
 * Debug 构建（_DEBUG 已定义）：完整实现，使用 <crtdbg.h> 相关 API。
 * Release 构建（_DEBUG 未定义）：所有方法均编译为空操作（no-op），
 *   保证 DLL 在 Release 模式下仍能正常导出符号、不影响链接。
 */

#include <utility/exception_tool/memory_helper/memory_helper.h>

// =============================================================================
// Debug 实现
// =============================================================================
#if defined(_WIN32) && defined(_DEBUG)

// ── CRT 调试堆 API ──────────────────────────────────────────────────────────
// 注意：_CRTDBG_MAP_ALLOC 由 CMakeLists.txt 在 Debug 配置下通过编译器命令行定义
// （-D_CRTDBG_MAP_ALLOC），确保在所有 <stdlib.h> 包含之前生效，此处不重复定义。
#include <stdlib.h>
#include <crtdbg.h>

// ── Windows API（用于写 stderr 句柄 & OutputDebugString）────────────────────
#include <Windows.h>

// ── 标准库 ──────────────────────────────────────────────────────────────────
#include <map>
#include <mutex>
#include <atomic>
#include <sstream>
#include <ctime>
#include <iomanip>
// 注意：不引入 <cstdio>（FILE* / fputs / fflush）
// 日志文件 I/O 全部使用 Win32 WriteFile，以避免在 CRT 报告钩子内
// 重入 UCRT 的 stdio 缓冲层（会触发 fwrite.cpp:81 stream->_cnt 断言）。

namespace common {
namespace utility {

// =============================================================================
// 模块内部状态（匿名命名空间，仅本文件可见）
// =============================================================================
namespace {

// ── 快照存储条目 ─────────────────────────────────────────────────────────────
struct SnapshotEntry {
    _CrtMemState state;      ///< CRT 堆状态（_CrtMemCheckpoint 填充）
    std::string  label;      ///< 用户标签
    std::time_t  timestamp;  ///< 拍摄时间戳
};

// ── 全局状态 ─────────────────────────────────────────────────────────────────
static std::map<int, SnapshotEntry> s_snapshots;    ///< 快照映射表
static std::mutex                   s_mutex;         ///< 保护快照表与目标配置
static std::atomic<int>             s_nextId{0};     ///< 自增 ID
static MemReportTarget              s_target  = MemReportTarget::Window;
static std::string                  s_logFile = "memory_leak.log";
// 日志文件使用 Win32 HANDLE 而非 FILE*：
//   WriteFile 是纯 Win32 调用，不经过 UCRT stdio 缓冲层，
//   可安全地在 CRT 报告钩子（_CrtSetReportHook2 回调）内部调用，
//   不会触发 UCRT 的 stream->_cnt 一致性断言（fwrite.cpp:81）。
static HANDLE                       s_hLogFile = INVALID_HANDLE_VALUE;
static bool                         s_hookInstalled = false;

// ── 防止报告钩子递归调用 ─────────────────────────────────────────────────────
static thread_local bool            s_inHook = false;

// =============================================================================
// 工具函数
// =============================================================================

/// 获取当前时间字符串（格式：YYYY-MM-DD HH:MM:SS）
static std::string GetTimestamp() {
    std::time_t now = std::time(nullptr);
    struct tm tmInfo{};
    localtime_s(&tmInfo, &now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmInfo);
    return std::string(buf);
}

/// 打开（或重新打开）日志文件（追加模式，Win32 HANDLE）
///
/// 使用 CreateFileA + SetFilePointer(FILE_END) 实现追加写入。
/// 写入时调用 WriteFile，完全绕过 UCRT stdio 缓冲层，
/// 可安全地在 CRT 调试回调（_CrtSetReportHook2）内部调用。
static void OpenLogFile(const std::string& path) {
    // 先关闭旧文件
    if (s_hLogFile != INVALID_HANDLE_VALUE) {
        CloseHandle(s_hLogFile);
        s_hLogFile = INVALID_HANDLE_VALUE;
    }
    // 打开/创建日志文件（OPEN_ALWAYS：存在则打开，不存在则创建）
    s_hLogFile = CreateFileA(
        path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,        // 允许其他进程同时读取日志
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (s_hLogFile != INVALID_HANDLE_VALUE) {
        // 定位到文件末尾，实现追加语义
        SetFilePointer(s_hLogFile, 0, nullptr, FILE_END);
    } else {
        // 打开失败时降级到仅窗口输出
        OutputDebugStringA("[MemoryHelper] Warning: failed to open log file: ");
        OutputDebugStringA(path.c_str());
        OutputDebugStringA("\n");
    }
}

/// 将消息写入已配置的输出目标（线程安全由调用方保证或在钩子内使用 s_inHook 保护）
static void WriteOutput(const char* msg) {
    if (!msg || msg[0] == '\0') return;

    if (HasFlag(s_target, MemReportTarget::Window)) {
        // → VS 输出窗口（调试器可见）
        OutputDebugStringA(msg);

        // → stderr（控制台可见）；使用 Win32 WriteFile 规避 CRT 格式化递归
        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        if (hErr != INVALID_HANDLE_VALUE && hErr != nullptr) {
            DWORD written = 0;
            WriteFile(hErr, msg, static_cast<DWORD>(strlen(msg)), &written, nullptr);
        }
    }

    if (HasFlag(s_target, MemReportTarget::File) && s_hLogFile != INVALID_HANDLE_VALUE) {
        // 使用 Win32 WriteFile，不经过 UCRT stdio 层，CRT 钩子内安全调用
        DWORD written = 0;
        WriteFile(s_hLogFile, msg, static_cast<DWORD>(strlen(msg)), &written, nullptr);
    }
}

/// 输出带换行的字符串消息
static void WriteReport(const std::string& msg) {
    WriteOutput(msg.c_str());
}

// =============================================================================
// CRT 报告钩子
// =============================================================================
/**
 * @brief 拦截所有 CRT 调试输出（_CrtDbgReport），路由到我们的输出目标
 *
 * 返回 TRUE：抑制 CRT 默认处理（避免重复输出到 VS 调试窗口）。
 * 使用 thread_local s_inHook 防止递归调用。
 */
static int CrtReportHook(int /*reportType*/, char* message, int* returnValue) {
    if (returnValue) *returnValue = 0;
    if (s_inHook) return FALSE; // 防止递归
    s_inHook = true;
    WriteOutput(message);
    s_inHook = false;
    return TRUE; // 已处理，抑制 CRT 默认输出
}

/// 安装 CRT 报告钩子（只安装一次）
///
/// 策略：通过 _CrtSetReportHook2 安装回调；钩子内返回 TRUE 抑制 CRT 默认输出
/// （_CRTDBG_MODE_CB 在部分 MSVC 版本中不可用，故不使用它；
///  _CrtSetReportHook2 回调返回值即可完全控制后续输出行为）。
static void InstallReportHook() {
    if (s_hookInstalled) return;
    _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, CrtReportHook);
    s_hookInstalled = true;
}

} // anonymous namespace

// =============================================================================
// MemoryHelper — 公开接口实现
// =============================================================================

void MemoryHelper::EnableLeakDetect(MemReportTarget target, const std::string& logFile) {
    // 1. 启用 CRT 堆跟踪标志
    //    _CRTDBG_ALLOC_MEM_DF  — 开启调试堆分配（必须）
    //    _CRTDBG_LEAK_CHECK_DF — 程序退出时自动调用 _CrtDumpMemoryLeaks()
    int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flags |= (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetDbgFlag(flags);

    // 2. 设置输出目标并安装钩子
    SetReportTarget(target, logFile);

    // 3. 输出启动横幅
    std::ostringstream oss;
    oss << "\n"
        << "╔══════════════════════════════════════════════╗\n"
        << "║       Memory Leak Detection  ENABLED         ║\n"
        << "╚══════════════════════════════════════════════╝\n"
        << "  Time    : " << GetTimestamp() << "\n"
        << "  Target  : "
        << (HasFlag(target, MemReportTarget::Window) ? "[Window] " : "")
        << (HasFlag(target, MemReportTarget::File)   ? "[File: " + logFile + "]" : "")
        << "\n"
        << "  Note    : Leak report will be dumped automatically on exit.\n"
        << "────────────────────────────────────────────────\n\n";
    WriteReport(oss.str());
}

void MemoryHelper::SetReportTarget(MemReportTarget target, const std::string& logFile) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_target  = target;
    s_logFile = logFile;

    // 若目标包含文件，打开/重新打开日志文件（失败时 OpenLogFile 内部已有降级处理）
    if (HasFlag(target, MemReportTarget::File)) {
        OpenLogFile(logFile);
    } else {
        // 切换为不含文件的目标时，关闭已打开的文件句柄
        if (s_hLogFile != INVALID_HANDLE_VALUE) {
            CloseHandle(s_hLogFile);
            s_hLogFile = INVALID_HANDLE_VALUE;
        }
    }

    // 安装报告钩子（首次调用）
    InstallReportHook();
}

MemSnapshot MemoryHelper::TakeSnapshot(const std::string& label) {
    std::lock_guard<std::mutex> lock(s_mutex);

    SnapshotEntry entry;
    int id          = s_nextId.fetch_add(1);
    entry.label     = label.empty() ? ("snapshot_" + std::to_string(id)) : label;
    entry.timestamp = std::time(nullptr);

    // 记录当前堆状态（核心 CRT 调用）
    _CrtMemCheckpoint(&entry.state);
    s_snapshots[id] = std::move(entry);

    // 输出拍摄通知
    std::ostringstream oss;
    oss << "[MemoryHelper] ◆ Snapshot #" << id
        << " taken  label=\"" << s_snapshots[id].label << "\""
        << "  time=" << GetTimestamp() << "\n";
    WriteReport(oss.str());

    MemSnapshot snap;
    snap.id    = id;
    snap.label = s_snapshots[id].label;
    return snap;
}

void MemoryHelper::DumpSince(const MemSnapshot& snap) {
    if (!snap.valid()) {
        WriteReport("[MemoryHelper] DumpSince: invalid snapshot handle.\n");
        return;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_snapshots.find(snap.id);
    if (it == s_snapshots.end()) {
        WriteReport("[MemoryHelper] DumpSince: snapshot #"
            + std::to_string(snap.id) + " not found.\n");
        return;
    }

    std::ostringstream header;
    header << "\n[MemoryHelper] ─── DumpSince snapshot #" << snap.id
           << " (\"" << snap.label << "\") ────────────────────\n";
    WriteReport(header.str());

    // 输出自该快照以来分配的全部对象（内部会调用报告钩子）
    _CrtMemDumpAllObjectsSince(&it->second.state);

    WriteReport("[MemoryHelper] ─── DumpSince end ────────────────────────────\n\n");
}

bool MemoryHelper::CompareSnapshots(const MemSnapshot& snap1, const MemSnapshot& snap2) {
    if (!snap1.valid() || !snap2.valid()) {
        WriteReport("[MemoryHelper] CompareSnapshots: one or both snapshot handles are invalid.\n");
        return false;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    auto it1 = s_snapshots.find(snap1.id);
    auto it2 = s_snapshots.find(snap2.id);

    if (it1 == s_snapshots.end()) {
        WriteReport("[MemoryHelper] CompareSnapshots: snapshot #"
            + std::to_string(snap1.id) + " not found.\n");
        return false;
    }
    if (it2 == s_snapshots.end()) {
        WriteReport("[MemoryHelper] CompareSnapshots: snapshot #"
            + std::to_string(snap2.id) + " not found.\n");
        return false;
    }

    // 输出比较标题
    std::ostringstream header;
    header << "\n[MemoryHelper] ─── CompareSnapshots ────────────────────────\n"
           << "  Snapshot A : #" << snap1.id << " \"" << snap1.label << "\"\n"
           << "  Snapshot B : #" << snap2.id << " \"" << snap2.label << "\"\n"
           << "─────────────────────────────────────────────────────────────\n";
    WriteReport(header.str());

    _CrtMemState diff{};
    // _CrtMemDifference 返回非零表示存在差异
    int hasDiff = _CrtMemDifference(&diff, &it1->second.state, &it2->second.state);

    if (hasDiff) {
        WriteReport("[MemoryHelper] *** Memory differences DETECTED! ***\n");

        // ── 第一部分：统计摘要（块数量、总字节数、高水位等）──────────────────
        WriteReport("\n[MemoryHelper] << Statistics Summary >>\n");
        _CrtMemDumpStatistics(&diff);

        // ── 第二部分：逐块详细信息（文件名 + 行号 + 内容预览）────────────────
        // _CrtMemDumpAllObjectsSince 以 snap1 为基准，列出在它之后分配且
        // 当前仍未释放的每个堆块，每行格式示例：
        //   {123} normal block at 0x..., 8 bytes long.
        //    Data: <....> 2A 00 00 00 ...
        //   memory_helper_demo.cpp(57) : {122} normal block at 0x..., 64 bytes long.
        // 启用 MEMORY_HELPER_TRACK_NEW / _CRTDBG_MAP_ALLOC 后文件名和行号会完整显示。
        WriteReport("\n[MemoryHelper] << Per-Block Detail (file + line + content preview) >>\n");
        _CrtMemDumpAllObjectsSince(&it1->second.state);
    } else {
        WriteReport("[MemoryHelper] No memory differences found between the two snapshots.\n");
    }

    WriteReport("[MemoryHelper] ─── CompareSnapshots end ────────────────────\n\n");
    return hasDiff != 0;
}

void MemoryHelper::DumpSnapshotStats(const MemSnapshot& snap) {
    if (!snap.valid()) {
        WriteReport("[MemoryHelper] DumpSnapshotStats: invalid snapshot handle.\n");
        return;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_snapshots.find(snap.id);
    if (it == s_snapshots.end()) {
        WriteReport("[MemoryHelper] DumpSnapshotStats: snapshot #"
            + std::to_string(snap.id) + " not found.\n");
        return;
    }

    std::ostringstream header;
    header << "\n[MemoryHelper] ─── SnapshotStats #" << snap.id
           << " (\"" << snap.label << "\") ─────────────────────\n";
    WriteReport(header.str());

    // 输出统计摘要（内部调用报告钩子）
    _CrtMemDumpStatistics(&it->second.state);

    WriteReport("[MemoryHelper] ─── SnapshotStats end ───────────────────────\n\n");
}

void MemoryHelper::DumpLeaks() {
    std::ostringstream header;
    header << "\n[MemoryHelper] ─── Manual DumpLeaks  " << GetTimestamp()
           << " ────────────────\n";
    WriteReport(header.str());

    // 立即输出当前堆中所有未释放的块
    _CrtDumpMemoryLeaks();

    WriteReport("[MemoryHelper] ─── DumpLeaks end ───────────────────────────\n\n");
}

} // namespace utility
} // namespace common

// =============================================================================
// Release 空操作实现（保证 DLL 在 Release 模式下正常导出符号）
// =============================================================================
#else // !(_WIN32 && _DEBUG)

namespace common {
namespace utility {

void MemoryHelper::EnableLeakDetect(MemReportTarget, const std::string&) {}
void MemoryHelper::SetReportTarget(MemReportTarget, const std::string&) {}

MemSnapshot MemoryHelper::TakeSnapshot(const std::string& label) {
    MemSnapshot s;
    s.label = label;
    return s;
}

void MemoryHelper::DumpSince(const MemSnapshot&) {}

bool MemoryHelper::CompareSnapshots(const MemSnapshot&, const MemSnapshot&) {
    return false;
}

void MemoryHelper::DumpSnapshotStats(const MemSnapshot&) {}
void MemoryHelper::DumpLeaks() {}

} // namespace utility
} // namespace common

#endif // _WIN32 && _DEBUG
