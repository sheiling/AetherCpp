cmake_minimum_required(VERSION 3.16)

set(CMAKE_FOLDER common/core/device/camera)

set(TOUP_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/toupcam")
set(TOUP_SDK_INCLUDE_DIR "${TOUP_SDK_ROOT}/include")
set(TOUP_SDK_LIB_DIR "${TOUP_SDK_ROOT}/lib")

set(TOUP_CAMERA_SOURCES
    toup_camera.cpp
    toup_camera.h
    toup_camera_global.h
    ../camera_base.cpp
    ../camera_base.h
)

fz2_add_common_library(toup_camera SHARED ${TOUP_CAMERA_SOURCES})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        TOUP_CAMERA_LIBRARY
        BUILD_CAMERA_TOUP
)

target_include_directories(${_tgt}
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/common"
        "${PROJECT_SOURCE_DIR}/common/include"
        "${PROJECT_SOURCE_DIR}/common/3rdparty"
        "${TOUP_SDK_INCLUDE_DIR}"
)

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::errors
        ${COMMON_TARGET_NAMESPACE}::logger
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::messagecenter
        "${TOUP_SDK_LIB_DIR}/toupcam.lib"
)

fz2_setup_target(${_tgt} "common/core/device/camera/toup_camera")

fz2_source_group_tree(toup_camera ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." FILES ${TOUP_CAMERA_SOURCES})

if (WIN32)
    add_custom_command(TARGET ${_tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TOUP_SDK_LIB_DIR}/toupcam.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMENT "Copy Toupcam SDK DLL -> $<TARGET_FILE_DIR:${_tgt}>"
    )
endif()

message(STATUS "    [toup_camera] Configured (SHARED library)")
