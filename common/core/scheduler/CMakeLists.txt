# ============================================================
# scheduler — 自动化设备流程执行调度引擎
#
# 目录见 README.md；对外 API 在 api/，步骤与引擎见 step/、engine/ 等。
# ============================================================

cmake_minimum_required(VERSION 3.16)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# ── 递归收集源文件（排除 examples 等目录，见 fz2_collect_sources）──
fz2_collect_sources(SCHEDULER_SOURCES SCHEDULER_HEADERS)

# ── 创建共享库目标 ──────────────────────────────────────────
fz2_add_common_library(scheduler SHARED
    ${SCHEDULER_SOURCES}
    ${SCHEDULER_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

set_target_properties(${_tgt} PROPERTIES AUTOMOC ON)

# ── 编译宏 ──────────────────────────────────────────────────
target_compile_definitions(${_tgt}
    PRIVATE
        SCHEDULER_LIBRARY       # 触发 dllexport
)

# ── 编译标准 ────────────────────────────────────────────────
target_compile_features(${_tgt} PUBLIC cxx_std_17)

# ── 头文件路径 ──────────────────────────────────────────────
target_include_directories(${_tgt}
    PUBLIC
        # 使用方通过 <scheduler/...> 或 <core/scheduler/...> 包含
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
        # 项目根目录（支持 <common/3rdparty/nlohmann/json.hpp> 等全路径引用）
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/3rdparty>
)

# ── 链接依赖 ────────────────────────────────────────────────
target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::logger
        ${COMMON_TARGET_NAMESPACE}::messagecenter
        Qt${QT_VERSION_MAJOR}::Core
)

# Windows 线程支持
if(WIN32)
    # 线程由 MSVC 运行时提供，无需显式链接
else()
    find_package(Threads REQUIRED)
    target_link_libraries(${_tgt} PRIVATE Threads::Threads)
endif()

# ── 编译选项 ────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${_tgt} PRIVATE /utf-8 /W3
        # 禁用部分不关键的警告
        /wd4251   # class 成员需要 DLL 接口
        /wd4267   # size_t → int 隐式转换
    )
else()
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# ── 输出目录 ────────────────────────────────────────────────
fz2_setup_target(${_tgt} "common/core/scheduler")

# ── VS 源文件分组 ────────────────────────────────────────────
fz2_source_group_tree(scheduler FILES ${SCHEDULER_SOURCES} ${SCHEDULER_HEADERS})

# ── 示例程序（可选）─────────────────────────────────────────
if(DEMO_SCHEDULER_EXAMPLE_BUILD)
    message(STATUS "    [scheduler/examples] Configuring...")
    add_subdirectory(examples)
endif()

message(STATUS "    [scheduler] Shared library configured.")
