# ============================================================
# common/core/device — 硬件设备驱动汇总
# ============================================================

message(STATUS "[common/core/device] 正在配置...")

if(HW_MOTION_CONTROL_BUILD)
    add_subdirectory(motionControl)
else()
    message(STATUS "  [device] 跳过 motionControl（HW_DEVICE_MOTION_VIRTUAL 与实体控制卡后端均未开启）")
endif()

if(HW_LAMPHOUSE_BUILD)
    add_subdirectory(lamphouse)
else()
    message(STATUS "  [device] 跳过 lamphouse（HW_LAMPHOUSE_BUILD=OFF）")
endif()

if(HW_SCANNER_BUILD)
    add_subdirectory(scanner)
else()
    message(STATUS "  [device] 跳过 scanner（HW_SCANNER_BUILD=OFF）")
endif()

if(HW_SOFTWARE_SCREEN_BUILD)
    add_subdirectory(displayControl/software_screen)
else()
    message(STATUS "  [device] 跳过 software_screen（HW_SOFTWARE_SCREEN_BUILD=OFF）")
endif()

if(HW_RANGEFINDER_BUILD)
    add_subdirectory(rangefinder)
else()
    message(STATUS "  [device] 跳过 rangefinder（HW_RANGEFINDER_BUILD=OFF）")
endif()

if(HW_COLORIMETER_BUILD)
    add_subdirectory(colorimeter)
else()
    message(STATUS "  [device] 跳过 colorimeter（HW_COLORIMETER_BUILD=OFF）")
endif()

if(HW_PRESSUREPICKUP_BUILD)
    add_subdirectory(pressurePickUp)
else()
    message(STATUS "  [device] 跳过 pressurePickUp（HW_PRESSUREPICKUP_BUILD=OFF）")
endif()

if(HW_FLASH_DETECT_BUILD)
    add_subdirectory(flashDetect)
else()
    message(STATUS "  [device] 跳过 flashDetect（HW_FLASH_DETECT_BUILD=OFF）")
endif()

add_subdirectory(camera)

if(HW_DEVICE_MANAGER_BUILD)
    add_subdirectory(deviceManager)
else()
    message(STATUS "  [device] 跳过 deviceManager（HW_DEVICE_MANAGER_BUILD=OFF）")
endif()

if(HW_IMAGE_CAPTURE_BUILD)
    add_subdirectory(imageCapture)
else()
    message(STATUS "  [device] 跳过 imageCapture（HW_IMAGE_CAPTURE_BUILD=OFF）")
endif()

if(HW_CAM_ASYNC_BUILD)
    add_subdirectory(camAsync)
else()
    message(STATUS "  [device] 跳过 camAsync（HW_CAM_ASYNC_BUILD=OFF）")
endif()

message(STATUS "[common/core/device] 配置完成")
