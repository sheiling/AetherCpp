# ============================================================
# GraphicsViewPlugin — IGraphicsVew 接口实现（Qt 插件 DLL）
#
# ui/GraphicsViewImpl — 实现 interface/igraphicsView.h，
#   在 initialize() 中注册到 PluginManager 对象池
# ============================================================

set(PLUGIN_NAME GraphicsView)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
endif()

# 递归收集 plugins/graphicsview/ 及其子目录 (ui/) 下所有源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    graphicsviewplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}            # plugin root — graphicsviewplugin.h, graphicsviewdef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui         # ui/ — scene, items, widget, impl
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::messagecenter
)
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PLUGIN_NAME} PRIVATE Qt6::OpenGLWidgets)
endif()

# extensionsystem 已 PUBLIC 传播 common/include；此处显式声明便于 IDE 解析 interface/igraphicsView.h
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/common/include
)

fz2_target_msvc_options(${PLUGIN_NAME})

fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/graphicsview")

fz2_source_group_tree(graphicsview FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [GraphicsViewPlugin] Plugin target configured.")
