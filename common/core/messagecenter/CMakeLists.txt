# ============================================
# MessageCenter 共享库（SHARED）
#
# 必须编译为共享库（DLL）：
#   各模块通过同一个 messagecenter.dll 共享
#   MessageCenter 单例，若编译为静态库则各 DLL 中
#   各有一份静态变量，单例无法跨 DLL 工作。
# ============================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(messagecenter SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE MESSAGECENTER_LIBRARY
)

target_compile_features(${_tgt} PUBLIC cxx_std_17)

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${_tgt}
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/messagecenter")

fz2_source_group_tree(messagecenter FILES ${SOURCES} ${HEADERS})

message(STATUS "  [messagecenter] Shared library target configured.")
