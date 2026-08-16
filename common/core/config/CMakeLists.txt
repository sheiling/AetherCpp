# ============================================================
# config — 基于 toml11 的框架配置模块
#
# 功能特性：
#   - load / reload / save         TOML 文件加载、热重载、写回
#   - get<T> / set<T>              点分路径类型安全读写（"server.port"）
#   - find<T>                      std::optional<T> 包装读取
#   - getArray<T>                  TOML array → std::vector<T>
#   - has / remove / keys          键存在检测 / 删除 / 枚举
#   - section(prefix)              作用域代理视图（ConfigSection）
#   - addChangeListener(pattern)   变更回调（支持通配符 "server.*"）
#   - setAutoReload(ms)            后台线程文件轮询热重载
#   - toTomlString()               序列化为 TOML 文本
#
# 依赖：
#   - toml11 v4（header-only，位于 common/3rdparty/toml11）
#   - C++17（std::shared_mutex / std::optional / std::filesystem）
#
# 对外目标：
#   - ${COMMON_LIB_PREFIX}config（SHARED 库）
#   - ${COMMON_TARGET_NAMESPACE}::config（别名，供下游 target_link_libraries 使用）
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── 收集源文件 ─────────────────────────────────────────────────────────────────
fz2_collect_sources(CONFIG_SOURCES CONFIG_HEADERS)

# ── 创建共享库目标 ─────────────────────────────────────────────────────────────
fz2_add_common_library(config SHARED
    ${CONFIG_SOURCES}
    ${CONFIG_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# ── 头文件目录 ─────────────────────────────────────────────────────────────────
target_include_directories(${_tgt}
    PUBLIC
        # 对外暴露的公共头文件目录（使用方 #include <Config.h>）
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        # toml11（header-only，PUBLIC 因为 Config.h 直接 include toml.hpp）
        ${CMAKE_SOURCE_DIR}/common/3rdparty/toml11/include
)

# ── 编译宏定义 ─────────────────────────────────────────────────────────────────
target_compile_definitions(${_tgt}
    PRIVATE
        CONFIG_LIBRARY          # 编译库本身时导出符号（dllexport）
)

# ── 编译特性 ───────────────────────────────────────────────────────────────────
target_compile_features(${_tgt} PUBLIC cxx_std_17)

fz2_target_msvc_options(${_tgt})
if(NOT MSVC)
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra)
endif()

# ── 链接依赖 ───────────────────────────────────────────────────────────────────
# toml11 以 header-only 方式集成，无需链接目标
# Windows 下线程支持由运行时库提供；Linux/macOS 需显式链接 Threads
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(${_tgt} PRIVATE Threads::Threads)
endif()

# ── 输出目录（与项目其他目标保持一致） ────────────────────────────────────────
fz2_setup_target(${_tgt} "common/core/config")

# ── VS 源文件分组 ──────────────────────────────────────────────────────────────
fz2_source_group_tree(config FILES ${CONFIG_SOURCES} ${CONFIG_HEADERS})

# ── 构建 example（可选，通过 DEMO_CONFIG_EXAMPLE_BUILD 选项控制）──────────────────────
if(DEMO_CONFIG_EXAMPLE_BUILD)
    message(STATUS "    [config/example] Configuring...")
    add_subdirectory(example)
endif()

message(STATUS "    [config] Configured — toml11 v4 header-only integration")
