fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(regaccess_melsec SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        REGACCESS_MELSEC_LIBRARY
)

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${_tgt}
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::tcp_comm
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/comm/regaccess/melsec")

fz2_source_group_tree(melsec FILES ${SOURCES} ${HEADERS})
