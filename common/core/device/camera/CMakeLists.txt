# ============================================================
# common/core/device/camera — 相机驱动汇总
# ============================================================

message(STATUS "  [device/camera] 正在配置...")

if(NOT HW_CAMERA_ANY_BUILD)
    message(STATUS "  [device/camera] 跳过（所有 HW_CAMERA_* 均为 OFF）")
    return()
endif()

if(HW_CAMERA_GALAXY_BUILD)
    add_subdirectory(galaxy_camera)

    if(DEMO_CAMERA_GALAXY_EXAMPLE_BUILD)
        add_subdirectory(galaxy_camera/examples)
    endif()
else()
    message(STATUS "    [camera] 跳过 galaxy_camera（HW_CAMERA_GALAXY_BUILD=OFF）")
endif()

if(HW_CAMERA_TOUP_BUILD)
    add_subdirectory(toup_camera)

    if(DEMO_CAMERA_TOUP_EXAMPLE_BUILD)
        add_subdirectory(toup_camera/examples)
    endif()
else()
    message(STATUS "    [camera] 跳过 toup_camera（HW_CAMERA_TOUP_BUILD=OFF）")
endif()

if(HW_CAMERA_VIRTUAL_BUILD)
    add_subdirectory(virtual_camera)
else()
    message(STATUS "    [camera] 跳过 virtual_camera（HW_CAMERA_VIRTUAL_BUILD=OFF）")
endif()

if(HW_CAMERA_MV_BUILD)
    add_subdirectory(mv_camera)

    if(DEMO_CAMERA_MV_EXAMPLE_BUILD)
        add_subdirectory(mv_camera/examples)
    endif()
else()
    message(STATUS "    [camera] 跳过 mv_camera（HW_CAMERA_MV_BUILD=OFF）")
endif()

message(STATUS "  [device/camera] 配置完成")
