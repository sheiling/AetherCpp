set(CMAKE_FOLDER common/core/device)

if(NOT HW_SOFTWARE_SCREEN_BUILD)
    return()
endif()

fz2_collect_sources(SOFTWARE_SCREEN_SRCS SOFTWARE_SCREEN_HEADERS)

fz2_add_common_library(software_screen SHARED ${SOFTWARE_SCREEN_SRCS} ${SOFTWARE_SCREEN_HEADERS})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE SOFTWARE_SCREEN_LIBRARY)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
)

fz2_setup_target(${_tgt} "common/core/device/displayControl/software_screen")

fz2_source_group_tree(software_screen FILES ${SOFTWARE_SCREEN_SRCS} ${SOFTWARE_SCREEN_HEADERS})

if(DEMO_SOFTWARE_SCREEN_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
