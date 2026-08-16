set(CMAKE_FOLDER common/core/examples)

set(GALAXY_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/galaxy")
set(GALAXY_SDK_INCLUDE_DIR "${GALAXY_SDK_ROOT}/include")
set(GALAXY_SDK_LIB_DIR "${GALAXY_SDK_ROOT}/lib/x64")

add_executable(galaxy_camera_demo
    galaxy_camera_demo.cpp
)

target_include_directories(galaxy_camera_demo PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/common"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/common/3rdparty"
    "${GALAXY_SDK_INCLUDE_DIR}"
)

target_compile_definitions(galaxy_camera_demo PRIVATE BUILD_CAMERA_GALAXY)

target_link_libraries(galaxy_camera_demo PRIVATE
    ${COMMON_TARGET_NAMESPACE}::galaxy_camera
)

fz2_setup_target(galaxy_camera_demo "common/core/device/camera/galaxy_camera/examples")
fz2_target_msvc_options(galaxy_camera_demo)

if (WIN32)
    add_custom_command(TARGET galaxy_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${GALAXY_SDK_LIB_DIR}"
            "$<TARGET_FILE_DIR:galaxy_camera_demo>"
        COMMENT "Copy Galaxy SDK DLLs -> $<TARGET_FILE_DIR:galaxy_camera_demo>"
    )

    add_custom_command(TARGET galaxy_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::galaxy_camera>"
            "$<TARGET_FILE_DIR:galaxy_camera_demo>"
        COMMENT "Copy galaxy_camera DLL -> $<TARGET_FILE_DIR:galaxy_camera_demo>"
    )
endif()

message(STATUS "    [galaxy_camera_demo] Configured (non-plugin executable)")

