# ============================================================
# ui_component — 跨插件通用主题控件库
#
# 基于 ThemeManager（ACSS）的表单/布局/反馈控件；命名空间 ui::。
# ============================================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(ui_component SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE UI_COMPONENT_LIBRARY)
target_compile_features(${_tgt} PUBLIC cxx_std_17)

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${_tgt}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        ${COMMON_TARGET_NAMESPACE}::uibase
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/ui")

fz2_source_group_tree(ui_component FILES ${SOURCES} ${HEADERS})

message(STATUS "  [ui_component] Shared library target configured.")
