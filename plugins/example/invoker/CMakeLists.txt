# ============================================================
# InvokerPlugin — ExtensionSystem::Invoker 使用示例插件
#
# 包含两个演示子页面：
#   1. InvokerBasicView — invoke<void/QString/int> 基础用法
#   2. InvokerArgsView  — 双参数 & Invoker<> 类模板用法
#
# 目标服务：
#   CalculatorService  — 带多个 Q_INVOKABLE 方法的目标对象
# ============================================================

set(PLUGIN_NAME Invoker)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    invokerplugin.json
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
)

fz2_target_msvc_options(${PLUGIN_NAME})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_plugin(${PLUGIN_NAME} "plugins/example/invoker")

fz2_source_group_tree(invoker FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [InvokerPlugin] Plugin target configured.")
