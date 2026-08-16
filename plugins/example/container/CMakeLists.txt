# ============================================================
# ContainerPlugin — Container IoC 服务容器使用示例插件
#
# 包含两个演示子页面：
#   1. ContainerTypeView  — bind/singleton/instance/extend/contextual/tag 类型绑定
#   2. ContainerNamedView — singletonNamed/makeNamed/call/get/set 命名绑定
#
# 依赖服务：
#   container        — IoC 服务容器核心库（common/core/container）
#   extensionsystem  — 插件系统
#   uibase           — UI 基础库（PageWidget、PageWithSubNav 等）
# ============================================================

set(PLUGIN_NAME ContainerPlugin)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    containerplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::container
)

fz2_target_msvc_options(${PLUGIN_NAME})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_plugin(${PLUGIN_NAME} "plugins/example/container")

fz2_source_group_tree(container FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [ContainerPlugin] Plugin target configured.")
