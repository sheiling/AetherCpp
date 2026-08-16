set(CMAKE_FOLDER common/core/device)

if(NOT HW_SCANNER_BUILD)
    return()
endif()

fz2_collect_sources(SCANNER_SRCS SCANNER_HEADERS)

fz2_add_common_library(scanner SHARED ${SCANNER_SRCS} ${SCANNER_HEADERS})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE SCANNER_LIBRARY)
target_link_libraries(${_tgt} PRIVATE ${COMMON_TARGET_NAMESPACE}::serial_comm ${COMMON_TARGET_NAMESPACE}::tcp_comm ${COMMON_TARGET_NAMESPACE}::logger ${COMMON_TARGET_NAMESPACE}::messagecenter)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
)

fz2_setup_target(${_tgt} "common/core/device/scanner")

fz2_source_group_tree(scanner FILES ${SCANNER_SRCS} ${SCANNER_HEADERS})

if(DEMO_SCANNER_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
