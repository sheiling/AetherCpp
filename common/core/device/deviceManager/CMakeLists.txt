set(CMAKE_FOLDER common/core/device/deviceManager)

if(NOT HW_DEVICE_MANAGER_BUILD)
    return()
endif()

set(DEVICE_MANAGER_FILES
    device_manager.cpp
    device_manager.h
    device_manager_global.h
)

fz2_add_common_library(device_manager SHARED ${DEVICE_MANAGER_FILES})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/include
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/libmodbus/include
        ${PROJECT_SOURCE_DIR}/common/utility/logger/include
        ${PROJECT_SOURCE_DIR}/common/3rdparty/spdlog/include
)

target_link_libraries(${_tgt}
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::errors
        ${COMMON_TARGET_NAMESPACE}::logger
        ${COMMON_TARGET_NAMESPACE}::container
        ${COMMON_TARGET_NAMESPACE}::tcp_comm
        ${COMMON_TARGET_NAMESPACE}::config
)

# Explicitly link libhv import library (hv logging symbols are referenced via hlog.h macros).
set(LIBHV_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/libhv")
target_link_directories(${_tgt} PRIVATE
    $<$<CONFIG:Debug>:${LIBHV_ROOT}/lib/Debug>
    $<$<CONFIG:Release>:${LIBHV_ROOT}/lib/Release>
    $<$<CONFIG:RelWithDebInfo>:${LIBHV_ROOT}/lib/RelWithDebInfo>
)
target_link_libraries(${_tgt} PRIVATE hv)

target_compile_definitions(${_tgt} PRIVATE
    DEVICE_MANAGER_LIBRARY
    # 默认情况下禁用“厂商扩展实现”，由下方选项决定是否开启
)

# BUILD_DEVICE_* / BUILD_CAMERA_* 注入 device_manager.cpp，用于 create*Instance 子类屏蔽（PRIVATE 即可）
if(HW_MOTION_CONTROL_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::motion_control)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_MOTION)
endif()

if(HW_LAMPHOUSE_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::lamphouse)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_LAMPHOUSE)
endif()

if(HW_SCANNER_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::scanner)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_SCANNER)
endif()

if(HW_RANGEFINDER_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::rangefinder)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_RANGEFINDER)
endif()

if(HW_PRESSUREPICKUP_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::pressurepickup)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_PRESSUREPICKUP)
endif()

if(HW_SOFTWARE_SCREEN_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::software_screen)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_SOFTWARE_SCREEN)
endif()

if(HW_COLORIMETER_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::colorimeter)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_COLORIMETER BUILD_COLORIMETER_KONICA)
endif()

if(HW_FLASH_DETECT_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::flash_detect)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_FLASH_DETECT)
endif()

if(HW_CAMERA_MV_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::mv_camera)
    target_compile_definitions(${_tgt} PRIVATE BUILD_CAMERA_MVS)
endif()

if(HW_CAMERA_TOUP_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::toup_camera)
    target_compile_definitions(${_tgt} PRIVATE BUILD_CAMERA_TOUP)
endif()

if(HW_CAMERA_GALAXY_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::galaxy_camera)
    target_compile_definitions(${_tgt} PRIVATE BUILD_CAMERA_GALAXY)
endif()

if(HW_CAMERA_VIRTUAL_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::virtual_camera)
    target_compile_definitions(${_tgt} PRIVATE BUILD_CAMERA_VC)
endif()

if(HW_IMAGE_CAPTURE_BUILD)
    target_link_libraries(${_tgt} PUBLIC ${COMMON_TARGET_NAMESPACE}::image_capture)
    target_compile_definitions(${_tgt} PRIVATE BUILD_DEVICE_IMAGE_CAPTURE)
endif()

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/device/deviceManager")

fz2_source_group_tree(deviceManager FILES ${DEVICE_MANAGER_FILES})

if(DEMO_DEVICE_MANAGER_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()

message(STATUS "    [deviceManager] Configured (SHARED library)")

