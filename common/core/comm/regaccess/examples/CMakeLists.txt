fz2_collect_sources(SOURCES HEADERS)

add_executable(regaccess_example
    regaccess_example.cpp
)

target_include_directories(regaccess_example
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(regaccess_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::regaccess_keyence
        ${COMMON_TARGET_NAMESPACE}::regaccess_melsec
        ${COMMON_TARGET_NAMESPACE}::regaccess_modbus
        ${COMMON_TARGET_NAMESPACE}::tcp_comm
)

fz2_target_msvc_options(regaccess_example)
fz2_setup_target(regaccess_example "common/core/comm/regaccess/examples")

