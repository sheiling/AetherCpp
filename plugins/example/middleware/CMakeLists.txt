# ============================================================
# MiddlewarePlugin — Middleware 机制使用示例插件
#
# 展示 MVVM 框架的 Middleware 扩展：
#   属性管道  : LoggingMiddleware → AuditLogMiddleware
#   命令管道  : AuthCommandMiddleware → LoggingCommandMiddleware
# ============================================================

set(PLUGIN_NAME Middleware)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    middlewareplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # plugins/example/middleware/ 本地头文件
)
# mvvm / extensionsystem / uibase 的 include 路径通过 PUBLIC 自动传播

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::mvvm
)

fz2_target_msvc_options(${PLUGIN_NAME})

# ── 输出目录 + VS 文件夹 ──────────────────────────────────────────────────
fz2_setup_plugin(${PLUGIN_NAME} "plugins/example/middleware")

fz2_source_group_tree(middleware FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [MiddlewarePlugin] Plugin target configured.")
