fz2_collect_sources(SOURCES HEADERS)

fz2_add_common_library(pressurepickup SHARED
    ${SOURCES}
    ${HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        PRESSUREPICKUP_LIBRARY
)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/libmodbus/include
)

target_link_libraries(${_tgt}
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::regaccess_modbus
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/device/pressurePickUp")

fz2_source_group_tree(pressurePickUp FILES ${SOURCES} ${HEADERS})

if(DEMO_PRESSUREPICKUP_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
