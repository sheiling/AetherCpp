/**
 * @file   memory_helper_demo.cpp
 * @brief  MemoryHelper 完整使用示例
 *
 * 演示场景：
 *   Demo 1 — 基本泄漏检测：故意泄漏对象，程序退出时验证 CRT 自动报告
 *   Demo 2 — 快照对比（有泄漏）：TakeSnapshot + CompareSnapshots 定位区间泄漏
 *   Demo 3 — 快照对比（无泄漏）：确认正确释放后快照差异为零
 *   Demo 4 — DumpSince：输出自快照以来分配的全部对象（含文件/行号）
 *   Demo 5 — DumpSnapshotStats：输出快照统计摘要
 *   Demo 6 — 动态切换输出目标：演示只写文件 vs 同时写窗口+文件
 *   Demo 7 — 手动 DumpLeaks：在业务代码中间随时触发泄漏快报
 *
 * 编译注意：
 *   - 本文件开头定义了 MEMORY_HELPER_TRACK_NEW，使 new 操作携带文件名+行号
 *   - 仅在 Debug 构建（_DEBUG 已定义）下有实际效果
 */

// ── 必须在所有 #include 之前定义，以便 DBG_NEW 覆盖 new ─────────────────────
#define MEMORY_HELPER_TRACK_NEW
#include <utility/exception_tool/memory_helper/memory_helper.h>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>

using namespace common::utility;

// ─── 工具宏：打印带格式的分隔线 ──────────────────────────────────────────────
#define DEMO_BEGIN(name) \
    std::cout << "\n\033[1;36m" \
              << "┌─────────────────────────────────────────────┐\n" \
              << "│  " << name << "\n" \
              << "└─────────────────────────────────────────────┘\033[0m\n"

#define DEMO_OK(msg)   std::cout << "  \033[32m[OK]\033[0m  " << msg << "\n"
#define DEMO_INFO(msg) std::cout << "  \033[33m[  ]\033[0m  " << msg << "\n"
#define DEMO_LEAK(msg) std::cout << "  \033[31m[!!]\033[0m  " << msg << "\n"

// =============================================================================
// Demo 1 — 基本泄漏检测
//   故意不释放一个 int* 与一个 std::string*，程序退出时 CRT 将在报告中列出。
// =============================================================================
static void Demo1_BasicLeakDetect() {
    DEMO_BEGIN("Demo 1 — Basic Leak Detection (intentional leak)");

    // ── 故意泄漏 1：裸指针 int
    int* leakedInt = new int(42);
    (void)leakedInt;  // 不释放
    DEMO_LEAK("Intentionally leaked: new int(42) @ " + std::to_string(reinterpret_cast<uintptr_t>(leakedInt)));

    // ── 故意泄漏 2：裸指针 char 数组
    char* leakedBuf = new char[64];
    strcpy_s(leakedBuf, 64, "Hello, Leaked World!");
    (void)leakedBuf;  // 不释放
    DEMO_LEAK("Intentionally leaked: new char[64] @ " + std::to_string(reinterpret_cast<uintptr_t>(leakedBuf)));

    DEMO_INFO("Both allocations leaked. They will appear in the exit leak report.");
}

// =============================================================================
// Demo 2 — 快照对比（存在泄漏）
//   snap1 → 分配内存 → snap2 → CompareSnapshots 应检测到差异。
// =============================================================================
static void Demo2_SnapshotWithLeak() {
    DEMO_BEGIN("Demo 2 — Snapshot Comparison (LEAK detected)");

    // 基准快照
    auto snap1 = MemoryHelper::TakeSnapshot("demo2_before");

    // 分配但不释放
    std::string* s = new std::string("I am a leaked string");
    int* arr = new int[50];
    (void)s; (void)arr;
    DEMO_LEAK("Allocated: new std::string + new int[50]  (not freed)");

    // 结束快照
    auto snap2 = MemoryHelper::TakeSnapshot("demo2_after_leak");

    // 对比：应检测到差异
    bool hasLeak = MemoryHelper::CompareSnapshots(snap1, snap2);
    if (hasLeak) {
        DEMO_LEAK("CompareSnapshots => difference DETECTED (expected)");
    } else {
        DEMO_OK("CompareSnapshots => no difference (unexpected!)");
    }
}

// =============================================================================
// Demo 3 — 快照对比（无泄漏）
//   snap1 → 分配 → 释放 → snap3 → CompareSnapshots 应无差异。
// =============================================================================
static void Demo3_SnapshotNoLeak() {
    DEMO_BEGIN("Demo 3 — Snapshot Comparison (NO leak, correct free)");

    auto snap1 = MemoryHelper::TakeSnapshot("demo3_before");

    // 分配并正确释放
    {
        std::vector<double> vec(100, 3.14);
        std::string msg = "temporary string, will be freed";
        DEMO_INFO("Allocated: vector<double>(100) + std::string  [will be freed]");
    }  // vec 和 msg 在此析构，内存归还

    auto snap2 = MemoryHelper::TakeSnapshot("demo3_after_free");

    bool hasLeak = MemoryHelper::CompareSnapshots(snap1, snap2);
    if (!hasLeak) {
        DEMO_OK("CompareSnapshots => NO difference (expected, memory properly freed)");
    } else {
        DEMO_LEAK("CompareSnapshots => difference found (unexpected!)");
    }
}

// =============================================================================
// Demo 4 — DumpSince
//   拍摄快照后分配几个对象（包含文件/行号），调用 DumpSince 逐一列出。
// =============================================================================
static void Demo4_DumpSince() {
    DEMO_BEGIN("Demo 4 — DumpSince (list objects since snapshot)");

    auto snap = MemoryHelper::TakeSnapshot("demo4_baseline");

    // 分配几个不同类型的对象
    double* d       = new double(2.718281828);
    char*   buf     = new char[128];
    std::snprintf(buf, 128, "buffer allocated at line ~%d", __LINE__);
    int*    matrix  = new int[4 * 4]{};

    DEMO_INFO("Allocated: double*, char[128], int[4*4]");
    DEMO_INFO("Calling DumpSince to list them (check debug output / log file)...");

    MemoryHelper::DumpSince(snap);

    // 清理（Demo 4 自身不泄漏）
    delete d;
    delete[] buf;
    delete[] matrix;

    DEMO_OK("Memory freed after DumpSince.");
}

// =============================================================================
// Demo 5 — DumpSnapshotStats
//   分配一批对象后拍快照，输出该快照的堆统计摘要。
// =============================================================================
static void Demo5_SnapshotStats() {
    DEMO_BEGIN("Demo 5 — DumpSnapshotStats (heap statistics)");

    // 先分配一些持久对象（故意泄漏，以便快照中有东西）
    std::vector<int*> ptrs;
    for (int i = 0; i < 8; ++i) {
        ptrs.push_back(new int(i * 100));
    }
    DEMO_INFO("Allocated 8 x int* (not freed, to populate the snapshot)");

    auto snap = MemoryHelper::TakeSnapshot("demo5_with_data");
    MemoryHelper::DumpSnapshotStats(snap);

    // 此处故意不释放 ptrs，让 Demo 5 贡献一批泄漏到最终报告
    DEMO_LEAK("8 x int* intentionally leaked for final leak report.");
}

// =============================================================================
// Demo 6 — 动态切换输出目标
//   演示在运行期间将报告从 Window 切换到 File，再切回 Both。
// =============================================================================
static void Demo6_SwitchReportTarget() {
    DEMO_BEGIN("Demo 6 — Dynamic Report Target Switching");

    // ── 阶段 A：切换到仅文件输出
    DEMO_INFO("Switching to FILE-only output → memory_report_demo6.log");
    MemoryHelper::SetReportTarget(MemReportTarget::File, "memory_report_demo6.log");

    auto snapA = MemoryHelper::TakeSnapshot("demo6_file_only_A");
    int* p1 = new int(111);
    auto snapB = MemoryHelper::TakeSnapshot("demo6_file_only_B");
    MemoryHelper::CompareSnapshots(snapA, snapB);
    delete p1;

    // ── 阶段 B：切换回 Both
    DEMO_INFO("Switching back to BOTH (Window + File) → memory_leak.log");
    MemoryHelper::SetReportTarget(MemReportTarget::Both, "memory_leak.log");

    auto snapC = MemoryHelper::TakeSnapshot("demo6_both_C");
    int* p2 = new int(222);
    auto snapD = MemoryHelper::TakeSnapshot("demo6_both_D");
    MemoryHelper::CompareSnapshots(snapC, snapD);
    delete p2;

    DEMO_OK("Report target switching complete. Check memory_report_demo6.log and memory_leak.log.");
}

// =============================================================================
// Demo 7 — 手动 DumpLeaks
//   在业务代码中途主动调用 DumpLeaks()，立即查看当前堆泄漏情况。
// =============================================================================
static void Demo7_ManualDumpLeaks() {
    DEMO_BEGIN("Demo 7 — Manual DumpLeaks (mid-program snapshot)");

    // 分配但不释放（累积到此刻的全部泄漏）
    char* secret = new char[32];
    strcpy_s(secret, 32, "mid-program leaked data");
    DEMO_LEAK("Allocated: char[32] (not freed) — will appear in DumpLeaks below");

    DEMO_INFO("Calling MemoryHelper::DumpLeaks() now...");
    MemoryHelper::DumpLeaks();

    DEMO_OK("DumpLeaks complete. Also includes all prior intentional leaks from Demo 1/2/5.");
}

// =============================================================================
// main
// =============================================================================
int main(int argc, char** argv) {
    // ── 在 main() 第一行启用 CRT 泄漏检测（Both = 窗口 + 文件）────────────────
    MemoryHelper::EnableLeakDetect(MemReportTarget::Both, "memory_leak.log");

    std::cout << "╔═══════════════════════════════════════════════════╗\n"
              << "║        MemoryHelper Demo — All Scenarios          ║\n"
              << "║  Leak report will be dumped automatically on exit ║\n"
              << "╚═══════════════════════════════════════════════════╝\n";

    // ── 依次运行所有演示场景 ────────────────────────────────────────────────
    Demo1_BasicLeakDetect();
    Demo2_SnapshotWithLeak();
    Demo3_SnapshotNoLeak();
    Demo4_DumpSince();
    Demo5_SnapshotStats();
    Demo6_SwitchReportTarget();
    Demo7_ManualDumpLeaks();

    // ── 结束提示 ────────────────────────────────────────────────────────────
    std::cout << "\n\033[1;32m"
              << "╔═══════════════════════════════════════════════════════╗\n"
              << "║  All demos complete.                                  ║\n"
              << "║  Press Enter to exit.                                 ║\n"
              << "║  ► Check 'memory_leak.log' for the full report.      ║\n"
              << "║  ► VS Output Window also shows the CRT leak dump.    ║\n"
              << "╚═══════════════════════════════════════════════════════╝\n"
              << "\033[0m";

    std::getchar();

    // 程序退出时，CRT 自动调用 _CrtDumpMemoryLeaks()
    // （因为在 EnableLeakDetect 中设置了 _CRTDBG_LEAK_CHECK_DF）
    return 0;
}
