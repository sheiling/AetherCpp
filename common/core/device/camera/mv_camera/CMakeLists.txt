cmake_minimum_required(VERSION 3.16)

set(CMAKE_FOLDER common/core/device/camera)

set(MVS_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/mvs")
set(MVS_SDK_INCLUDE_DIR "${MVS_SDK_ROOT}/Includes")
set(MVS_SDK_LIB_DIR "${MVS_SDK_ROOT}/lib")

set(MV_CAMERA_SOURCES
    mv_camera.cpp
    mv_camera.h
    mv_camera_global.h
    ../camera_base.cpp
    ../camera_base.h
)

fz2_add_common_library(mv_camera SHARED ${MV_CAMERA_SOURCES})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        MV_CAMERA_LIBRARY
        BUILD_CAMERA_MVS
)

target_include_directories(${_tgt}
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/common"
        "${PROJECT_SOURCE_DIR}/common/include"
        "${PROJECT_SOURCE_DIR}/common/3rdparty"
        "${MVS_SDK_INCLUDE_DIR}"
)

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::errors
        ${COMMON_TARGET_NAMESPACE}::logger
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::messagecenter
        "${MVS_SDK_LIB_DIR}/MvCameraControl.lib"
)

fz2_setup_target(${_tgt} "common/core/device/camera/mv_camera")
fz2_target_msvc_options(${_tgt})

fz2_source_group_tree(mv_camera ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." FILES ${MV_CAMERA_SOURCES})

# 直接使用工控机上安装的版本
# if (WIN32)
#     add_custom_command(TARGET ${_tgt} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${MVS_SDK_LIB_DIR}/MvCameraControl.dll"
#             "$<TARGET_FILE_DIR:${_tgt}>"
#         COMMENT "Copy MvCameraControl DLL -> $<TARGET_FILE_DIR:${_tgt}>"
#     )
# endif()

message(STATUS "    [mv_camera] Configured (SHARED library)")

