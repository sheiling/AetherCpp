set(CMAKE_FOLDER common/core/examples)

set(MVS_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/mvs")
set(MVS_SDK_INCLUDE_DIR "${MVS_SDK_ROOT}/Includes")
set(MVS_SDK_LIB_DIR "${MVS_SDK_ROOT}/lib")

add_executable(mv_camera_demo
    mv_camera_demo.cpp
)

target_include_directories(mv_camera_demo PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/common"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/common/3rdparty"
    "${MVS_SDK_INCLUDE_DIR}"
)

target_compile_definitions(mv_camera_demo PRIVATE BUILD_CAMERA_MVS)

target_link_libraries(mv_camera_demo PRIVATE
    ${COMMON_TARGET_NAMESPACE}::mv_camera
)

fz2_setup_target(mv_camera_demo "common/core/device/camera/mv_camera/examples")
fz2_target_msvc_options(mv_camera_demo)

if (WIN32)
    add_custom_command(TARGET mv_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MVS_SDK_LIB_DIR}/MvCameraControl.dll"
            "$<TARGET_FILE_DIR:mv_camera_demo>"
        COMMENT "Copy MvCameraControl DLL -> $<TARGET_FILE_DIR:mv_camera_demo>"
    )

    add_custom_command(TARGET mv_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::mv_camera>"
            "$<TARGET_FILE_DIR:mv_camera_demo>"
        COMMENT "Copy mv_camera DLL -> $<TARGET_FILE_DIR:mv_camera_demo>"
    )
endif()

message(STATUS "    [mv_camera_demo] Configured (non-plugin executable)")

