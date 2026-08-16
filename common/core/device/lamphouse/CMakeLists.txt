set(CMAKE_FOLDER common/core/device)

if(NOT HW_LAMPHOUSE_BUILD)
    return()
endif()

fz2_collect_sources(LAMPHOUSE_SRCS LAMPHOUSE_HEADERS)

fz2_add_common_library(lamphouse SHARED ${LAMPHOUSE_SRCS} ${LAMPHOUSE_HEADERS})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE LAMPHOUSE_LIBRARY)
target_link_libraries(${_tgt} PRIVATE ${COMMON_TARGET_NAMESPACE}::serial_comm)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
)

fz2_setup_target(${_tgt} "common/core/device/lamphouse")
fz2_target_msvc_options(${_tgt})

fz2_source_group_tree(lamphouse FILES ${LAMPHOUSE_SRCS} ${LAMPHOUSE_HEADERS})

if(DEMO_LAMPHOUSE_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
