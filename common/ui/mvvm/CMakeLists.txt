# ============================================================
# mvvm 共享库
#
# 包含：
#   - ViewModel  — MVVM ViewModel 基类
#   - Command    — 命令模式基类 / FunctionalCommand
#   - DataBinding — 控件与 ViewModel 的双向绑定辅助类
#
# 编译为共享库（DLL），供插件及主程序直接链接使用。
# ============================================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(mvvm SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# 编译库自身时定义 MVVM_LIBRARY，触发 Q_DECL_EXPORT
target_compile_definitions(${_tgt} PRIVATE MVVM_LIBRARY)
target_compile_features(${_tgt} PUBLIC cxx_std_17)

# PUBLIC 包含路径：
#   ${CMAKE_CURRENT_SOURCE_DIR}/..  → common/ui/  → 允许 #include "mvvm/xxx.h"
#   ${CMAKE_CURRENT_SOURCE_DIR}     → common/ui/mvvm/ → 允许 #include "viewmodel.h"（直接包含）
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

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_target(${_tgt} "common/ui")

fz2_source_group_tree(mvvm FILES ${SOURCES} ${HEADERS})

message(STATUS "  [mvvm] Shared library target configured.")
