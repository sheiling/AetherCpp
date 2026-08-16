# ============================================================
# CorePlugin — 核心插件（主窗口）
# 输出为 DLL，放置在 bin/<Config>/plugins/ 目录
# ============================================================

set(PLUGIN_NAME Core)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    coreplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

# include 路径由 extensionsystem / uibase 的 PUBLIC 传播，无需手动设置。
# 额外添加当前目录供本插件内部互引。
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::mvvm
    ${COMMON_TARGET_NAMESPACE}::config
)

if(WIN32)
    target_link_libraries(${PLUGIN_NAME} PRIVATE dwmapi user32 gdi32)
endif()

fz2_target_msvc_options(${PLUGIN_NAME})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/core")

fz2_source_group_tree(core FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [CorePlugin] Plugin target configured.")
