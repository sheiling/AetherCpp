# ============================================
# ExtensionSystem 共享库（SHARED）
#
# 必须编译为共享库（DLL）：
#   插件 DLL 与主程序通过同一个 extensionsystem.dll 共享
#   PluginManager 单例，若编译为静态库则各 DLL 中
#   各有一份静态变量，单例无法跨 DLL 工作。
# ============================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(extensionsystem SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# 编译库自身时定义 EXTENSIONSYSTEM_LIBRARY，触发 Q_DECL_EXPORT
target_compile_definitions(${_tgt}
    PRIVATE EXTENSIONSYSTEM_LIBRARY
)

target_compile_features(${_tgt} PUBLIC cxx_std_17)

# 头文件路径（PUBLIC → 依赖方自动获得）
#   common/core/           → extensionsystem/pluginmanager.h
#   ${PROJECT_SOURCE_DIR}  → common/include/interface/igraphicsView.h 等公共接口
target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${_tgt}
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
    PRIVATE ${COMMON_TARGET_NAMESPACE}::messagecenter
)

fz2_target_msvc_options(${_tgt})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_target(${_tgt} "common/core/extensionsystem")

fz2_source_group_tree(extensionsystem FILES ${SOURCES} ${HEADERS})

message(STATUS "  [extensionsystem] Shared library target configured.")
