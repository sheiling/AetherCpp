# ============================================================
# http_comm — 基于 libhv 封装的 HTTP 客户端/服务端
# 说明：
#   - 编译为动态库（SHARED），供其他模块/插件复用
#   - 按配置(Debug/Release/RelWithDebInfo) 自动链接对应版本的 libhv
# ============================================================

fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(http_comm SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

if(MSVC)
    # 自动从导出的符号生成 import lib，便于其他目标链接 http_comm.dll
    set_target_properties(${_tgt} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_compile_features(${_tgt} PUBLIC cxx_std_17)

target_compile_definitions(${_tgt}
    PRIVATE
        HTTP_COMM_LIBRARY
)

set(LIBHV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/libhv")

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${LIBHV_ROOT}/include>
)

target_link_directories(${_tgt} PRIVATE
    $<$<CONFIG:Debug>:${LIBHV_ROOT}/lib/Debug>
    $<$<CONFIG:Release>:${LIBHV_ROOT}/lib/Release>
    $<$<CONFIG:RelWithDebInfo>:${LIBHV_ROOT}/lib/RelWithDebInfo>
)

target_link_libraries(${_tgt} PRIVATE hv)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/comm/http")

fz2_source_group_tree(http FILES ${SOURCES} ${HEADERS})

set(LIBHV_DLL
    $<$<CONFIG:Debug>:${LIBHV_ROOT}/bin/Debug/hv.dll>
    $<$<CONFIG:Release>:${LIBHV_ROOT}/bin/Release/hv.dll>
    $<$<CONFIG:RelWithDebInfo>:${LIBHV_ROOT}/bin/RelWithDebInfo/hv.dll>
)

add_custom_command(TARGET ${_tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIBHV_DLL}
        $<TARGET_FILE_DIR:${_tgt}>
    COMMENT "Copying libhv runtime DLL to http_comm output directory"
)

message(STATUS "    [http_comm] Shared library target configured (link with libhv).")

# 示例程序
if(DEMO_HTTP_COMM_EXAMPLE_BUILD)
    message(STATUS "    [http_comm/examples] Configuring...")
    add_subdirectory(examples)
endif()

