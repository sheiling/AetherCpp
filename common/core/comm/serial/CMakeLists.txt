# ============================================================
# serial_comm - 串口通信封装（基于 thirdparty serial）
# ============================================================

set(SERIAL_3RDPARTY_ROOT "${CMAKE_SOURCE_DIR}/common/3rdparty/serial")

fz2_collect_sources(SERIAL_LOCAL_SRCS SERIAL_LOCAL_HEADERS)

set(SERIAL_COMM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/serial_master.cpp
    ${SERIAL_3RDPARTY_ROOT}/src/serial.cc
)

if(WIN32)
    list(APPEND SERIAL_COMM_SOURCES
        ${SERIAL_3RDPARTY_ROOT}/src/impl/win.cc
        ${SERIAL_3RDPARTY_ROOT}/src/impl/list_ports/list_ports_win.cc
    )
elseif(APPLE)
    list(APPEND SERIAL_COMM_SOURCES
        ${SERIAL_3RDPARTY_ROOT}/src/impl/unix.cc
        ${SERIAL_3RDPARTY_ROOT}/src/impl/list_ports/list_ports_osx.cc
    )
else()
    list(APPEND SERIAL_COMM_SOURCES
        ${SERIAL_3RDPARTY_ROOT}/src/impl/unix.cc
        ${SERIAL_3RDPARTY_ROOT}/src/impl/list_ports/list_ports_linux.cc
    )
endif()

fz2_add_common_library(serial_comm SHARED
    ${SERIAL_COMM_SOURCES}
    ${SERIAL_LOCAL_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_features(${_tgt} PUBLIC cxx_std_17)

target_compile_definitions(${_tgt}
    PRIVATE
        SERIAL_COMM_LIBRARY
)

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/3rdparty>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${SERIAL_3RDPARTY_ROOT}/include
)

if(WIN32)
    target_link_libraries(${_tgt} PRIVATE setupapi advapi32)
endif()

if(MSVC)
    set_target_properties(${_tgt} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/comm/serial")

set(_SERIAL_3RDPARTY_SRCS ${SERIAL_COMM_SOURCES})
list(FILTER _SERIAL_3RDPARTY_SRCS INCLUDE REGEX "^${SERIAL_3RDPARTY_ROOT}/.*")
fz2_source_group_tree(serial_comm FILES ${SERIAL_LOCAL_SRCS} ${SERIAL_LOCAL_HEADERS})
fz2_source_group_tree(serial_comm ROOT "${SERIAL_3RDPARTY_ROOT}" FILES ${_SERIAL_3RDPARTY_SRCS})

message(STATUS "    [serial_comm] Shared library target configured.")

if(DEMO_SERIAL_COMM_EXAMPLE_BUILD)
    message(STATUS "    [serial_comm/examples] Configuring...")
    add_subdirectory(examples)
endif()
