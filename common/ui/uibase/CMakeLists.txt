# ============================================================
# uibase 共享库
#
# 包含：
#   - ThemeManager / LanguageManager（原 corelib 部分）
#   - FramelessWindow / NavigationManager / PageWidget
#   - IMainWindowService 接口
#   - ExamplePages（SimplePage / SubPage / PageWithSubNav）
#   - 权限运行时（uibase/permission/：PermService）
#
# 编译为共享库（DLL），供所有插件共享单例（TM / LM）。
# ============================================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Svg)
# QJsonDocument 在 Qt5/Qt6 的 Core 模块中，无需额外链接

# ── 从 .ts 自动提取 TR() 查找用的 Qt context ─────────────────────────────────
file(GLOB _TR_APP_TS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/config/app_*.ts")
file(GLOB _TR_PLUGIN_TS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/plugins/*/i18n/*.ts")
file(GLOB _TR_COMMON_PLUGIN_TS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/common/plugins/*/i18n/*.ts")
set(_TR_ALL_TS ${_TR_APP_TS} ${_TR_PLUGIN_TS} ${_TR_COMMON_PLUGIN_TS})
set(_TR_CONTEXTS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/translation_contexts.generated.h")
set(OUT_HEADER "${_TR_CONTEXTS_HEADER}")
set(TS_FILES "${_TR_ALL_TS}")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateTranslationContexts.cmake")

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(uibase SHARED
    ${SOURCES}
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/ui_resources.qrc
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE UIBASE_LIBRARY)
target_compile_features(${_tgt} PUBLIC cxx_std_17)

# PUBLIC 包含路径：
#   ${CMAKE_CURRENT_SOURCE_DIR}/..  → common/ui/  → 允许 #include "uibase/xxx.h" 和 #include "corelib/xxx.h"
#   ${CMAKE_CURRENT_SOURCE_DIR}     → common/ui/uibase/  → 允许 #include "xxx.h"（直接包含）
#   permission/ 子目录 → 允许 permission 内 #include "permservice.h"
target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/permission>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(${_tgt}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Svg
)

if(WIN32)
    target_link_libraries(${_tgt} PUBLIC dwmapi user32 gdi32)
endif()

# ── Qt-Advanced-Stylesheets ──────────────────────────────────────────────────
fz2_target_link_acss(${_tgt})
fz2_copy_acss_styles(${_tgt})

fz2_target_msvc_options(${_tgt})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_target(${_tgt} "common/ui")

fz2_source_group_tree(uibase FILES ${SOURCES} ${HEADERS})

# ── corelib 别名（INTERFACE 库）────────────────────────────────────────────
# 原设计中 corelib 是独立库，现已合并到 uibase。
# 创建 INTERFACE 目标使插件可继续用 target_link_libraries(xxx PRIVATE ${COMMON_TARGET_NAMESPACE}::corelib)。
fz2_add_common_interface(corelib)
set(_corelib ${FZ2_COMMON_LIBRARY_TARGET})
target_link_libraries(${_corelib} INTERFACE ${_tgt})
set_target_properties(${_corelib} PROPERTIES FOLDER "common/ui")

message(STATUS "  [uibase] Shared library target configured.")
