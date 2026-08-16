# ============================================================
# logger — 基于 spdlog 的框架日志模块
#
# 提供功能：
#   - 多日志级别（Trace / Debug / Info / Warn / Error / Critical）
#   - 自定义输出格式 Pattern
#   - 控制台彩色输出（Windows: WinColor；Linux/macOS: ANSI Color）
#   - 文件输出，日志路径按月份分级目录（logs/YYYY/MM/）
#   - 四种轮转策略：None / Daily / Size / Both（混合）
#   - 简洁宏接口：LOG_TRACE / LOG_DEBUG / LOG_INFO / LOG_WARN / LOG_ERROR / LOG_CRITICAL
#
# 依赖：
#   - spdlog（header-only，位于 common/3rdparty/spdlog）
#
# 对外目标：
#   - ${COMMON_LIB_PREFIX}logger（SHARED 库）
#   - ${COMMON_TARGET_NAMESPACE}::logger（别名，供下游 target_link_libraries 使用）
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── 收集源文件 ─────────────────────────────────────────────────────────────────
fz2_collect_sources(LOGGER_SOURCES LOGGER_HEADERS)

# ── 创建共享库目标 ─────────────────────────────────────────────────────────────
fz2_add_common_library(logger SHARED
    ${LOGGER_SOURCES}
    ${LOGGER_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_include_directories(${_tgt}
    PUBLIC
        # 对外暴露的公共头文件目录（使用方 #include <Logger.h>）
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        # spdlog（header-only，PUBLIC 因为 Logger.h 直接 include spdlog 头文件）
        ${CMAKE_SOURCE_DIR}/common/3rdparty/spdlog/include
)

# ── 编译宏定义 ─────────────────────────────────────────────────────────────────
target_compile_definitions(${_tgt}
    PRIVATE
        LOGGER_LIBRARY          # 编译库本身时导出符号（dllexport）
)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
target_compile_features(${_tgt} PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(${_tgt} PUBLIC /utf-8)
    target_compile_options(${_tgt} PRIVATE /W3)
else()
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra)
endif()

# ── 链接依赖 ───────────────────────────────────────────────────────────────────
# spdlog 以 header-only 方式集成，无需链接目标
# Windows 下需要链接 Threads（spdlog 内部使用 std::mutex / std::thread）
if(WIN32)
    # Windows 平台下的线程支持由运行时库提供，无需显式链接
else()
    find_package(Threads REQUIRED)
    target_link_libraries(${_tgt} PRIVATE Threads::Threads)
endif()

# ── 输出目录（使用 FZ2Helpers 函数，与项目其他目标保持一致） ──────────────────
fz2_setup_target(${_tgt} "common/utility/logger")

# ── VS 源文件分组 ──────────────────────────────────────────────────────────────
fz2_source_group_tree(logger FILES ${LOGGER_SOURCES} ${LOGGER_HEADERS})

# ── 构建 example（可选，通过 DEMO_LOGGER_EXAMPLE_BUILD 选项控制）─────────────────────
if(DEMO_LOGGER_EXAMPLE_BUILD)
    message(STATUS "    [logger/example] Configuring...")
    add_subdirectory(example)
endif()

message(STATUS "    [logger] Configured — spdlog header-only integration")
