set(CMAKE_FOLDER common/base)

set(COMMON_BASE_SRCS)
set(COMMON_BASE_HEADERS)
set(COMMON_BASE_DEFINES)
set(COMMON_BASE_PUBLIC_INCLUDES
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/include
)
set(COMMON_BASE_PUBLIC_LIBS
    ${COMMON_TARGET_NAMESPACE}::errors
)
set(COMMON_BASE_PRIVATE_LIBS)

if(CORE_ALGORITHM_BUILD)
    list(APPEND COMMON_BASE_SRCS
        algorithm/algorithm_runtime.cpp
    )
    list(APPEND COMMON_BASE_HEADERS
        algorithm/algorithm.h
        algorithm/algorithm_base.h
        algorithm/algorithm_factory.h
        algorithm/algorithm_global.h
        algorithm/algorithm_packet.h
    )
    list(APPEND COMMON_BASE_DEFINES
        ALGORITHM_LIBRARY
    )
    message(STATUS "    [base/algorithm] Enabled")
endif()

if(CORE_IMGPROC_BUILD)
    list(APPEND COMMON_BASE_SRCS
        imgproc/imgproc.cpp
    )
    list(APPEND COMMON_BASE_HEADERS
        imgproc/imgproc.h
        imgproc/imgproc_global.h
        imgproc/imgproc_error_codes.h
        imgproc/image_type_map.h
    )
    list(APPEND COMMON_BASE_DEFINES
        IMGPROC_LIBRARY
    )
    list(APPEND COMMON_BASE_PUBLIC_INCLUDES
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/include
    )
    list(APPEND COMMON_BASE_PRIVATE_LIBS
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
    )
    message(STATUS "    [base/imgproc] Enabled")
endif()

if(NOT COMMON_BASE_SRCS)
    message(STATUS "    [base] Skipped (no modules enabled)")
    return()
endif()

list(REMOVE_DUPLICATES COMMON_BASE_PUBLIC_INCLUDES)
list(REMOVE_DUPLICATES COMMON_BASE_PRIVATE_LIBS)
list(REMOVE_DUPLICATES COMMON_BASE_DEFINES)

fz2_add_common_library(base SHARED
    ${COMMON_BASE_SRCS}
    ${COMMON_BASE_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# VS 里按 base 目录下实际结构展示源文件（algorithm/imgproc/...）
fz2_source_group_tree(base FILES ${COMMON_BASE_SRCS} ${COMMON_BASE_HEADERS})

target_compile_definitions(${_tgt}
    PRIVATE
        ${COMMON_BASE_DEFINES}
)

target_include_directories(${_tgt}
    PUBLIC
        ${COMMON_BASE_PUBLIC_INCLUDES}
)

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_BASE_PUBLIC_LIBS}
    PRIVATE
        ${COMMON_BASE_PRIVATE_LIBS}
)

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/base")

if(DEMO_ALGORITHM_EXAMPLE_BUILD AND CORE_ALGORITHM_BUILD)
    add_subdirectory(algorithm/example)
endif()

if(DEMO_IMGPROC_EXAMPLE_BUILD AND CORE_IMGPROC_BUILD)
    add_subdirectory(imgproc/example)
endif()

message(STATUS "    [base] Configured (SHARED library: ${COMMON_LIB_PREFIX}base)")
