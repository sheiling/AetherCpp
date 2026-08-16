# ============================================================
# memory_helper_demo — MemoryHelper 完整使用示例可执行程序
#
# 演示以下功能：
#   1. EnableLeakDetect     — 启用 CRT 泄漏检测，退出时自动报告
#   2. TakeSnapshot         — 内存状态快照
#   3. CompareSnapshots     — 快照差异对比（有泄漏 / 无泄漏两种场景）
#   4. DumpSince            — 输出自快照以来的所有分配对象
#   5. DumpSnapshotStats    — 快照统计摘要
#   6. SetReportTarget      — 动态切换输出目标（窗口 / 文件 / 两者）
#   7. DumpLeaks            — 手动触发泄漏快报
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── 可执行文件 ─────────────────────────────────────────────────────────────────
add_executable(memory_helper_demo
    memory_helper_demo.cpp
)

# ── 链接 memory_helper 库 ──────────────────────────────────────────────────────
# ${COMMON_TARGET_NAMESPACE}::memory_helper 已 PUBLIC 暴露 ${CMAKE_SOURCE_DIR}/common include，
# 因此无需再额外设置 target_include_directories。
target_link_libraries(memory_helper_demo
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::memory_helper
)

# ── C++ 标准 ───────────────────────────────────────────────────────────────────
target_compile_features(memory_helper_demo PRIVATE cxx_std_17)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(memory_helper_demo PRIVATE /utf-8 /W3)

    # Debug 构建：开启 _CRTDBG_MAP_ALLOC 以便 malloc/calloc/realloc 也携带行号
    target_compile_definitions(memory_helper_demo
        PRIVATE
            $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
    )
endif()

# ── 输出目录（与项目其他可执行文件保持一致）────────────────────────────────────
fz2_setup_target(memory_helper_demo
    "common/utility/exception_tool/memory_helper/examples")

message(STATUS "          [memory_helper_demo] Configured")
