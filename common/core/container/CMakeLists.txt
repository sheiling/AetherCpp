# ============================================================
# container — IoC 服务容器共享库（Laravel 风格）
#
# 功能特性：
#   - bind / singleton / instance   服务绑定三种生命周期
#   - extend                        装饰/代理已有绑定
#   - make<T>()                     依赖解析（支持递归解析）
#   - when/needs/give               上下文绑定（Contextual Binding）
#   - tag / tagged                  标签分组批量解析
#   - ServiceProvider               服务提供者生命周期
#   - Application                   应用级容器 + 全局单例
#   - Facade<T>                     静态外观访问入口
#
# 必须编译为共享库（DLL）：
#   若以静态库方式链接，Application 单例在各 DLL 中各有一份，
#   无法跨 DLL 共享，与 extensionsystem 的情况相同。
# ============================================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(container SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# 编译库自身时定义 CONTAINER_LIBRARY，触发 Q_DECL_EXPORT
target_compile_definitions(${_tgt}
    PRIVATE CONTAINER_LIBRARY
)

target_compile_features(${_tgt} PUBLIC cxx_std_17)

# 头文件路径（PUBLIC → 依赖方自动获得）
target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${_tgt}
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
)

fz2_target_msvc_options(${_tgt})

# ── 输出目录（Debug / Release / RelWithDebInfo）+ VS 解决方案文件夹 ────────────
fz2_setup_target(${_tgt} "common/core/container")

fz2_source_group_tree(container FILES ${SOURCES} ${HEADERS})

message(STATUS "  [container] Shared library target configured.")
