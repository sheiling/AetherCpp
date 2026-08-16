#ifndef __COMMON_UTILITY_MEMORY_HELPER_H__
#define __COMMON_UTILITY_MEMORY_HELPER_H__

/**
 * @file   memory_helper.h
 * @brief  基于 Windows CRT 调试堆的内存泄漏检测与快照对比工具
 *
 * 功能概述：
 *   1. EnableLeakDetect()  — 启用 CRT 堆跟踪；程序退出时自动输出内存泄漏报告
 *   2. TakeSnapshot()      — 在任意时刻对内存状态拍摄快照
 *   3. CompareSnapshots()  — 对比两个快照，输出内存差异（潜在泄漏点）
 *   4. DumpSince()         — 输出自指定快照之后的所有分配对象
 *   5. DumpSnapshotStats() — 输出指定快照的统计摘要
 *   6. DumpLeaks()         — 立即手动输出当前内存泄漏报告
 *   7. SetReportTarget()   — 动态切换报告输出目标（调试窗口 / 文件 / 两者）
 *
 * 平台与构建模式限制：
 *   - 仅支持 Windows MSVC（依赖 <crtdbg.h>）
 *   - Release 构建（未定义 _DEBUG）时所有方法均为空操作（no-op）
 *
 * 可选宏 MEMORY_HELPER_TRACK_NEW：
 *   在 .cpp 文件 **最顶部**（其它 include 之前）定义该宏，可使泄漏报告精确
 *   显示 new 操作的文件名与行号。请勿在公共头文件中定义，以免污染所有编译单元。
 *
 * 典型用法：
 * @code
 *   #define MEMORY_HELPER_TRACK_NEW
 *   #include <utility/exception_tool/memory_helper/memory_helper.h>
 *   using namespace common::utility;
 *
 *   int main() {
 *       // 1. 启用泄漏检测，输出到调试窗口和文件
 *       MemoryHelper::EnableLeakDetect(MemReportTarget::Both, "leak.log");
 *
 *       // 2. 拍摄基准快照
 *       auto snap1 = MemoryHelper::TakeSnapshot("baseline");
 *
 *       DoSomething();  // 执行业务代码
 *
 *       // 3. 对比快照
 *       auto snap2 = MemoryHelper::TakeSnapshot("after_work");
 *       MemoryHelper::CompareSnapshots(snap1, snap2);
 *
 *       // 4. 程序退出时自动输出完整泄漏报告
 *   }
 * @endcode
 */

#include <string>
#include <cstdint>

// ── Windows DLL 导入 / 导出 ────────────────────────────────────────────────────
#ifdef _WIN32
#  ifdef MEMORY_HELPER_LIBRARY
#    define MEMORY_HELPER_API __declspec(dllexport)
#  else
#    define MEMORY_HELPER_API __declspec(dllimport)
#  endif
#else
#  define MEMORY_HELPER_API
#endif

// ── 抑制 std::string 成员的 DLL 接口警告（C4251）────────────────────────────
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4251)
#endif

namespace common {
namespace utility {

// =============================================================================
// MemReportTarget — 报告输出目标标志（可按位 OR 组合）
// =============================================================================
/**
 * @brief 报告输出目标，支持按位 OR 组合
 */
enum class MemReportTarget : uint32_t {
    None   = 0x00u,  ///< 不输出
    Window = 0x01u,  ///< 调试窗口（OutputDebugString）及 stderr
    File   = 0x02u,  ///< 输出到日志文件
    Both   = 0x03u   ///< 同时输出到窗口和文件
};

/// 对 MemReportTarget 进行按位 OR 操作
inline MemReportTarget operator|(MemReportTarget a, MemReportTarget b) {
    return static_cast<MemReportTarget>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

/// 检查 target 是否包含指定的 flag
inline bool HasFlag(MemReportTarget target, MemReportTarget flag) {
    return (static_cast<uint32_t>(target) & static_cast<uint32_t>(flag)) != 0u;
}

// =============================================================================
// MemSnapshot — 内存状态快照句柄
// =============================================================================
/**
 * @brief 内存状态快照句柄（从 MemoryHelper::TakeSnapshot() 获取）
 *
 * 内部 id 对应保存在 MemoryHelper 内部映射表中的 _CrtMemState；
 * 不要手动构造该结构体，请始终通过 TakeSnapshot() 获取。
 */
struct MEMORY_HELPER_API MemSnapshot {
    int         id    = -1;   ///< 内部快照 ID，-1 表示无效句柄
    std::string label;        ///< 用户自定义描述标签

    /// 是否为有效快照
    bool valid() const noexcept { return id >= 0; }
};

// =============================================================================
// MemoryHelper — 主工具类（所有方法均为静态）
// =============================================================================
/**
 * @brief CRT 内存泄漏检测与快照对比工具
 *
 * 所有方法均为静态，无需实例化。
 * Release 构建（!_DEBUG）中所有方法均为空操作（no-op）。
 */
class MEMORY_HELPER_API MemoryHelper {
public:
    // ── 初始化与配置 ────────────────────────────────────────────────────────

    /**
     * @brief 启用 CRT 内存泄漏检测
     *
     * 调用后 CRT 开始跟踪所有堆分配；程序正常退出时自动输出泄漏报告。
     * 建议在 main() 的第一行调用。
     *
     * @param target   报告输出目标（默认输出到调试窗口）
     * @param logFile  日志文件路径，仅当 target 包含 File 时有效
     *                 （默认 "memory_leak.log"，相对于程序工作目录）
     */
    static void EnableLeakDetect(
        MemReportTarget    target  = MemReportTarget::Window,
        const std::string& logFile = "memory_leak.log");

    /**
     * @brief 动态调整报告输出目标（可在程序运行期间随时调用）
     *
     * @param target   新的输出目标
     * @param logFile  日志文件路径（target 包含 File 时有效）
     */
    static void SetReportTarget(
        MemReportTarget    target,
        const std::string& logFile = "memory_leak.log");

    // ── 快照操作 ────────────────────────────────────────────────────────────

    /**
     * @brief 拍摄当前内存状态快照
     *
     * 内部调用 _CrtMemCheckpoint() 记录当前堆状态，并与自增 ID 关联。
     *
     * @param label  可选描述标签（便于日志识别）
     * @return       快照句柄，id >= 0 表示成功
     */
    static MemSnapshot TakeSnapshot(const std::string& label = "");

    /**
     * @brief 输出自指定快照之后分配的所有堆对象
     *
     * 内部调用 _CrtMemDumpAllObjectsSince()。
     * 输出包含分配块地址、大小及（启用 MEMORY_HELPER_TRACK_NEW 时的）
     * 源文件名与行号。
     *
     * @param snap  TakeSnapshot() 返回的快照句柄
     */
    static void DumpSince(const MemSnapshot& snap);

    /**
     * @brief 对比两个快照，输出内存差异报告（含文件名 + 行号）
     *
     * 当存在差异时，分两部分输出：
     *   1. 统计摘要（_CrtMemDumpStatistics）— 差异块数/字节数/高水位等
     *   2. 逐块详细信息（_CrtMemDumpAllObjectsSince）— 每个未释放块的地址、
     *      大小、内容预览；启用 MEMORY_HELPER_TRACK_NEW 或 _CRTDBG_MAP_ALLOC
     *      后还会显示精确的源文件名与行号。
     *
     * 正常使用流程：snap1 在操作前拍摄，snap2 在操作后拍摄；
     * 若存在差异则表示该区间内发生了内存泄漏。
     *
     * @param snap1  较早的快照（基准）
     * @param snap2  较晚的快照（比较目标）
     * @return       true — 存在内存差异（潜在泄漏）；false — 无差异
     */
    static bool CompareSnapshots(const MemSnapshot& snap1, const MemSnapshot& snap2);

    /**
     * @brief 输出指定快照的堆内存统计摘要
     *
     * 内部调用 _CrtMemDumpStatistics()，输出：
     *   总分配次数 / 总分配字节数 / 最高水位 / 空闲块数量等。
     *
     * @param snap  TakeSnapshot() 返回的快照句柄
     */
    static void DumpSnapshotStats(const MemSnapshot& snap);

    // ── 手动报告 ────────────────────────────────────────────────────────────

    /**
     * @brief 立即手动输出当前内存泄漏报告
     *
     * 内部调用 _CrtDumpMemoryLeaks()。
     * 注意：EnableLeakDetect() 启用的自动检测在程序退出时同样会调用此报告，
     * 此函数用于在运行期间的任意时刻主动获取当前泄漏情况。
     */
    static void DumpLeaks();
};

} // namespace utility
} // namespace common

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

// =============================================================================
// 可选宏：为 new/delete 附加文件名与行号信息
// =============================================================================
/**
 * 在包含本头文件 **之前** 定义 MEMORY_HELPER_TRACK_NEW，CRT 将在泄漏报告中
 * 精确显示每次 new 操作的源文件名与行号。
 *
 * 注意事项：
 *   - 只在 .cpp 实现文件顶部定义，切勿在 .h 公共头文件中定义
 *   - 仅在 _DEBUG 构建下有效；Release 构建中此宏无效果
 *
 * 示例（memory_helper_demo.cpp）：
 * @code
 *   #define MEMORY_HELPER_TRACK_NEW
 *   #include <utility/exception_tool/memory_helper/memory_helper.h>
 * @endcode
 */
#ifdef MEMORY_HELPER_TRACK_NEW
#  ifdef _DEBUG
#    include <crtdbg.h>
#    ifndef DBG_NEW
#      define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#      define new DBG_NEW
#    endif
#  endif
#endif

#endif // __COMMON_UTILITY_MEMORY_HELPER_H__
