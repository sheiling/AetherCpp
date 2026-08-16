fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(regaccess_modbus SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        REGACCESS_MODBUS_LIBRARY
)

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/3rdparty/libmodbus/include>
)

target_link_directories(${_tgt}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common/3rdparty/libmodbus/lib
)

target_link_libraries(${_tgt}
    PRIVATE
        modbus
)

add_custom_command(TARGET ${_tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/common/3rdparty/libmodbus/lib/modbus.dll"
        "$<TARGET_FILE_DIR:${_tgt}>"
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/comm/regaccess/modbus")

fz2_source_group_tree(modbus FILES ${SOURCES} ${HEADERS})

