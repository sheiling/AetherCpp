set(CMAKE_FOLDER common/core/device)

fz2_collect_sources(FLASH_DETECT_SRCS FLASH_DETECT_HEADERS)

fz2_add_common_library(flash_detect SHARED
    ${FLASH_DETECT_SRCS}
    ${FLASH_DETECT_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        FLASH_DETECT_LIBRARY
)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/include
        ${PROJECT_SOURCE_DIR}/common/3rdparty
)

target_link_libraries(${_tgt}
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::logger
        ${COMMON_TARGET_NAMESPACE}::messagecenter
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/device/flashDetect")

fz2_source_group_tree(flash_detect FILES ${FLASH_DETECT_SRCS} ${FLASH_DETECT_HEADERS})

if(DEMO_FLASH_DETECT_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
