# ============================================================
# memory_helper — 基于 CRT 的内存泄漏检测与快照对比模块
#
# 提供功能：
#   1. 启用 CRT 调试堆跟踪，程序退出时自动输出泄漏报告
#   2. 任意时刻拍摄内存状态快照（TakeSnapshot）
#   3. 对比两个快照（CompareSnapshots），定位泄漏区间
#   4. 转储自快照以来的分配对象（DumpSince）
#   5. 输出快照统计摘要（DumpSnapshotStats）
#   6. 报告兼容输出到调试窗口（OutputDebugString / stderr）或文件
#
# 平台与构建模式：
#   - 仅支持 Windows MSVC（依赖 <crtdbg.h>）
#   - Release 构建中方法均为空操作（no-op），DLL 符号正常导出
#
# 对外目标：
#   - ${COMMON_LIB_PREFIX}memory_helper（SHARED 库）
#   - ${COMMON_TARGET_NAMESPACE}::memory_helper（别名，供下游 target_link_libraries 使用）
#
# 包含路径约定：
#   #include <utility/exception_tool/memory_helper/memory_helper.h>
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── 源文件与头文件 ─────────────────────────────────────────────────────────────
set(MEMORY_HELPER_HEADERS
    memory_helper.h
)

set(MEMORY_HELPER_SOURCES
    memory_helper.cpp
)

# ── 创建共享库目标 ─────────────────────────────────────────────────────────────
fz2_add_common_library(memory_helper SHARED
    ${MEMORY_HELPER_SOURCES}
    ${MEMORY_HELPER_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# ── 头文件搜索路径 ─────────────────────────────────────────────────────────────
target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
        $<INSTALL_INTERFACE:include>
)

# ── 编译宏定义 ─────────────────────────────────────────────────────────────────
target_compile_definitions(${_tgt}
    PRIVATE
        MEMORY_HELPER_LIBRARY        # 编译本库时导出符号（dllexport）
)

# Debug 配置下开启 _CRTDBG_MAP_ALLOC，将 malloc/free 映射到调试版本
# （为 C 风格分配提供文件名/行号追踪）
if(MSVC)
    target_compile_definitions(${_tgt}
        PRIVATE
            $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
    )
endif()

# ── C++ 标准 ───────────────────────────────────────────────────────────────────
target_compile_features(${_tgt} PUBLIC cxx_std_17)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${_tgt} PRIVATE /utf-8 /W3)
endif()

fz2_setup_target(${_tgt} "common/utility/exception_tool/memory_helper")

fz2_source_group_tree(memory_helper FILES ${MEMORY_HELPER_SOURCES} ${MEMORY_HELPER_HEADERS})

# ── 构建示例（可选，通过 DEMO_MEMORY_HELPER_EXAMPLE_BUILD 控制）─────────────────────
if(DEMO_MEMORY_HELPER_EXAMPLE_BUILD)
    message(STATUS "        [memory_helper/examples] Configuring...")
    add_subdirectory(examples)
endif()

message(STATUS "        [memory_helper] Configured — CRT debug heap integration")
