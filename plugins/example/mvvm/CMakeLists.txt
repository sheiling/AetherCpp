# ============================================================
# MvvmPlugin — MVVM 示例插件
#
# 直接链接 mvvm 共享库（common/ui/mvvm），
# 仅包含本插件自身的业务代码。
#
# 包含两个示例页面：
#   1. UserView    — 基础 MVVM 演示（表单绑定、命令绑定）
#   2. DeviceView  — 增强 MVVM 演示
#        ・结构体字段绑定（bindStructField<T>）
#        ・QSlider / QDoubleSpinBox / QProgressBar
#        ・QDateEdit / QTimeEdit / QRadioButton
#        ・bindEnabled / bindVisible
# ============================================================

set(PLUGIN_NAME Mvvm)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    mvvmplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # plugins/example/mvvm/ 本地头文件
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

# ── 输出文件名：避免与 mvvm.lib 在 Windows 大小写不敏感文件系统上冲突 ─────
set_target_properties(${PLUGIN_NAME} PROPERTIES OUTPUT_NAME "MvvmPlugin")

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_plugin(${PLUGIN_NAME} "plugins/example/mvvm")

fz2_source_group_tree(mvvm FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [MvvmPlugin] Plugin target configured (UserView + DeviceView).")
