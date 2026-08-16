set(CMAKE_FOLDER common/core/examples)

set(TOUP_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/toupcam")
set(TOUP_SDK_LIB_DIR "${TOUP_SDK_ROOT}/lib")

add_executable(toup_camera_demo
    toup_camera_demo.cpp
)

target_include_directories(toup_camera_demo PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/common"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/common/3rdparty"
    "${TOUP_SDK_ROOT}/include"
)

target_compile_definitions(toup_camera_demo PRIVATE BUILD_CAMERA_TOUP)

target_link_libraries(toup_camera_demo PRIVATE
    ${COMMON_TARGET_NAMESPACE}::toup_camera
)

fz2_setup_target(toup_camera_demo "common/core/device/camera/toup_camera/examples")

if (WIN32)
    add_custom_command(TARGET toup_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TOUP_SDK_LIB_DIR}/toupcam.dll"
            "$<TARGET_FILE_DIR:toup_camera_demo>"
        COMMENT "Copy Toupcam SDK DLL -> $<TARGET_FILE_DIR:toup_camera_demo>"
    )

    add_custom_command(TARGET toup_camera_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::toup_camera>"
            "$<TARGET_FILE_DIR:toup_camera_demo>"
        COMMENT "Copy toup_camera DLL -> $<TARGET_FILE_DIR:toup_camera_demo>"
    )
endif()

message(STATUS "    [toup_camera_demo] Configured (non-plugin executable)")
