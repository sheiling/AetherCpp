cmake_minimum_required(VERSION 3.16)

set(CMAKE_FOLDER common/core/device/camera)

fz2_add_common_library(virtual_camera SHARED
    virtual_camera.cpp
    virtual_camera.h
    virtual_camera_global.h
    ../camera_base.cpp
    ../camera_base.h
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        VIRTUAL_CAMERA_LIBRARY
        BUILD_CAMERA_VC
)

target_include_directories(${_tgt}
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/common"
        "${PROJECT_SOURCE_DIR}/common/include"
        "${PROJECT_SOURCE_DIR}/common/3rdparty"
        "${PROJECT_SOURCE_DIR}/common/core/config/include"
        "${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/include"
)

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::errors
        ${COMMON_TARGET_NAMESPACE}::logger
        ${COMMON_TARGET_NAMESPACE}::config
        ${COMMON_TARGET_NAMESPACE}::messagecenter
    PRIVATE
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

fz2_setup_target(${_tgt} "common/core/device/camera/virtual_camera")

message(STATUS "    [virtual_camera] Configured (SHARED library)")
