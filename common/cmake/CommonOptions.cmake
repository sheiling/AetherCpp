# ============================================================
# CommonOptions.cmake
#
# common/ 目录编译开关（核心模块、硬件驱动、公共插件、示例程序）。
# 由 common/CMakeLists.txt include。
# app / plugins 选项见 app/cmake/AppOptions.cmake、plugins/cmake/PluginOptions.cmake。
# ============================================================

# ── 公共插件 ─────────────────────────────────────────────────

option(PLUGIN_LIGHTEN_DETECTOR_BUILD
    "编译 common/plugins/lightendetector（LightenDetector 算法插件）" ON)


# ── 核心模块 ─────────────────────────────────────────────────

option(CORE_ALGORITHM_BUILD
    "将 algorithm 模块编入 common_base" ON)

option(CORE_IMGPROC_BUILD
    "将 imgproc 模块编入 common_base" ON)

option(CORE_SCHEDULER_BUILD
    "编译 scheduler 流程调度共享库" ON)


# ── 硬件驱动 ─────────────────────────────────────────────────

option(HW_LAMPHOUSE_BUILD
    "编译光源控制库（lamphouse）" ON)

option(HW_SCANNER_BUILD
    "编译扫码器库（scanner）" ON)

option(HW_SOFTWARE_SCREEN_BUILD
    "编译软件打屏库（software_screen）" ON)

option(HW_RANGEFINDER_BUILD
    "编译测距仪库（rangefinder）" ON)

option(HW_COLORIMETER_BUILD
    "编译色度计库（colorimeter）" ON)

option(HW_PRESSUREPICKUP_BUILD
    "编译压力传感器库（pressurePickUp）" ON)

option(HW_FLASH_DETECT_BUILD
    "编译 Flash 检测库（flashDetect）" ON)

option(HW_CAMERA_GALAXY_BUILD
    "编译 Galaxy 相机库（需 common/3rdparty 厂商 SDK）" ON)

option(HW_CAMERA_TOUP_BUILD
    "编译 Toupcam 相机库（需 common/3rdparty 厂商 SDK）" ON)

option(HW_CAMERA_VIRTUAL_BUILD
    "编译虚拟相机库（virtual_camera）" ON)

option(HW_CAMERA_MV_BUILD
    "编译 MVS 相机库（需 MvCameraControl SDK）" ON)

option(HW_DEVICE_MANAGER_BUILD
    "编译设备管理器库（deviceManager）" ON)

option(HW_IMAGE_CAPTURE_BUILD
    "编译图像采集库（imageCapture）" ON)

option(HW_CAM_ASYNC_BUILD
    "编译 CamAsync 异步采集库" ON)

option(HW_DEVICE_MOTION_LEISHINE_DMC
    "编译雷赛 DMC 运动控制后端" OFF)

option(HW_DEVICE_MOTION_LEISHINE_NMC
    "编译雷赛 NMC 运动控制后端" OFF)

option(HW_DEVICE_MOTION_SOLID
    "编译 Solid 运动控制后端" OFF)

option(HW_DEVICE_MOTION_GAOCHUAN_IO
    "编译高川 IO 运动控制后端" OFF)

option(HW_DEVICE_MOTION_VIRTUAL
    "编译虚拟运控后端（无实体控制卡/SDK，开发机联调）" ON)


# ── 示例与 Demo ──────────────────────────────────────────────

option(DEMO_ALGORITHM_EXAMPLE_BUILD
    "编译 algorithm 示例程序" ON)

option(DEMO_IMGPROC_EXAMPLE_BUILD
    "编译 imgproc 示例程序" ON)

option(DEMO_TCP_COMM_EXAMPLE_BUILD
    "编译 TCP 通信示例" ON)

option(DEMO_HTTP_COMM_EXAMPLE_BUILD
    "编译 HTTP 通信示例" ON)

option(DEMO_SERIAL_COMM_EXAMPLE_BUILD
    "编译串口通信示例" ON)

option(DEMO_COMM_REGACCESS_EXAMPLE_BUILD
    "编译寄存器访问（regaccess）示例" ON)

option(DEMO_CONFIG_EXAMPLE_BUILD
    "编译 config 模块示例" ON)

option(COMMON_ORM_BUILD
    "编译 ORM 模块（common/utility/orm）—— QxOrm 预编译库仅支持 Qt5 ABI，Qt6 下自动关闭" ON)

option(DEMO_ORM_EXAMPLE_BUILD
    "编译 ORM 示例（sqlite 与 mysql）" ON)

option(DEMO_ORM_SQLITE_EXAMPLE_BUILD
    "编译 ORM SQLite 示例" ON)

option(DEMO_ORM_MYSQL_EXAMPLE_BUILD
    "编译 ORM MySQL 示例" ON)

option(DEMO_LOGGER_EXAMPLE_BUILD
    "编译 logger 示例" ON)

option(DEMO_ERRORS_EXAMPLE_BUILD
    "编译 errors 模块示例" ON)

option(DEMO_MEMORY_HELPER_EXAMPLE_BUILD
    "编译 memory_helper 示例" ON)

option(DEMO_SCHEDULER_EXAMPLE_BUILD
    "编译 scheduler 示例程序" ON)

option(DEMO_LAMPHOUSE_EXAMPLE_BUILD
    "编译 lamphouse 示例" ON)

option(DEMO_SCANNER_EXAMPLE_BUILD
    "编译 scanner 示例" ON)

option(DEMO_SOFTWARE_SCREEN_EXAMPLE_BUILD
    "编译 software_screen 示例" ON)

option(DEMO_RANGEFINDER_EXAMPLE_BUILD
    "编译 rangefinder 示例" ON)

option(DEMO_COLORIMETER_EXAMPLE_BUILD
    "编译 colorimeter 示例" ON)

option(DEMO_PRESSUREPICKUP_EXAMPLE_BUILD
    "编译 pressurePickUp 示例" ON)

option(DEMO_FLASH_DETECT_EXAMPLE_BUILD
    "编译 flashDetect 示例" ON)

option(DEMO_CAMERA_GALAXY_EXAMPLE_BUILD
    "编译 Galaxy 相机示例" ON)

option(DEMO_CAMERA_TOUP_EXAMPLE_BUILD
    "编译 Toupcam 相机示例" ON)

option(DEMO_CAMERA_MV_EXAMPLE_BUILD
    "编译 MVS 相机示例" ON)

option(DEMO_DEVICE_MANAGER_EXAMPLE_BUILD
    "编译 deviceManager 示例" ON)

option(DEMO_DEVICE_MOTION_EXAMPLE_BUILD
    "编译运动控制示例" ON)

# ── 派生开关（勿在 CMake GUI 中手改；由上方硬件/核心选项计算）──────────────

if(HW_DEVICE_MOTION_VIRTUAL OR HW_DEVICE_MOTION_LEISHINE_DMC OR HW_DEVICE_MOTION_LEISHINE_NMC
   OR HW_DEVICE_MOTION_SOLID OR HW_DEVICE_MOTION_GAOCHUAN_IO)
    set(HW_MOTION_CONTROL_BUILD TRUE)
else()
    set(HW_MOTION_CONTROL_BUILD FALSE)
endif()
set(HW_MOTION_CONTROL_BUILD ${HW_MOTION_CONTROL_BUILD} CACHE INTERNAL
    "运控库 motionControl：HW_DEVICE_MOTION_VIRTUAL 或任一实体控制卡后端 HW_DEVICE_MOTION_* 为 ON 时为 TRUE")


if(HW_CAMERA_GALAXY_BUILD OR HW_CAMERA_TOUP_BUILD
   OR HW_CAMERA_VIRTUAL_BUILD OR HW_CAMERA_MV_BUILD)
    set(HW_CAMERA_ANY_BUILD TRUE)
else()
    set(HW_CAMERA_ANY_BUILD FALSE)
endif()
set(HW_CAMERA_ANY_BUILD ${HW_CAMERA_ANY_BUILD} CACHE INTERNAL
    "相机子树：任一 HW_CAMERA_* 为 ON 时为 TRUE")


# Demo 与硬件/核心开关联动：依赖模块未编译时 Demo 强制 OFF
function(_fz2_gate_demo_if_off DEMO_VAR HW_VAR REASON)
    if(NOT ${HW_VAR})
        if(${DEMO_VAR})
            message(STATUS "[CommonOptions] ${DEMO_VAR} → OFF（${REASON}）")
        endif()
        set(${DEMO_VAR} OFF CACHE BOOL "" FORCE)
    endif()
endfunction()

_fz2_gate_demo_if_off(DEMO_ALGORITHM_EXAMPLE_BUILD CORE_ALGORITHM_BUILD "CORE_ALGORITHM_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_IMGPROC_EXAMPLE_BUILD CORE_IMGPROC_BUILD "CORE_IMGPROC_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_SCHEDULER_EXAMPLE_BUILD CORE_SCHEDULER_BUILD "CORE_SCHEDULER_BUILD=OFF")


_fz2_gate_demo_if_off(DEMO_LAMPHOUSE_EXAMPLE_BUILD HW_LAMPHOUSE_BUILD "HW_LAMPHOUSE_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_SCANNER_EXAMPLE_BUILD HW_SCANNER_BUILD "HW_SCANNER_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_SOFTWARE_SCREEN_EXAMPLE_BUILD HW_SOFTWARE_SCREEN_BUILD "HW_SOFTWARE_SCREEN_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_RANGEFINDER_EXAMPLE_BUILD HW_RANGEFINDER_BUILD "HW_RANGEFINDER_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_COLORIMETER_EXAMPLE_BUILD HW_COLORIMETER_BUILD "HW_COLORIMETER_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_PRESSUREPICKUP_EXAMPLE_BUILD HW_PRESSUREPICKUP_BUILD "HW_PRESSUREPICKUP_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_FLASH_DETECT_EXAMPLE_BUILD HW_FLASH_DETECT_BUILD "HW_FLASH_DETECT_BUILD=OFF")


_fz2_gate_demo_if_off(DEMO_CAMERA_GALAXY_EXAMPLE_BUILD HW_CAMERA_GALAXY_BUILD "HW_CAMERA_GALAXY_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_CAMERA_TOUP_EXAMPLE_BUILD HW_CAMERA_TOUP_BUILD "HW_CAMERA_TOUP_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_CAMERA_MV_EXAMPLE_BUILD HW_CAMERA_MV_BUILD "HW_CAMERA_MV_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_DEVICE_MANAGER_EXAMPLE_BUILD HW_DEVICE_MANAGER_BUILD "HW_DEVICE_MANAGER_BUILD=OFF")
_fz2_gate_demo_if_off(DEMO_DEVICE_MOTION_EXAMPLE_BUILD HW_MOTION_CONTROL_BUILD "无运控卡后端开启")

if(NOT DEMO_ORM_EXAMPLE_BUILD)
    set(DEMO_ORM_SQLITE_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
    set(DEMO_ORM_MYSQL_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
endif()

# ── Qt6 下自动关闭 ORM（QxOrm 预编译库仅支持 Qt5 ABI）──────────────────────
if(QT_VERSION_MAJOR EQUAL 6 AND COMMON_ORM_BUILD)
    message(STATUS "[CommonOptions] COMMON_ORM_BUILD → OFF（Qt6 下 QxOrm ABI 不兼容）")
    set(COMMON_ORM_BUILD OFF CACHE BOOL "" FORCE)
endif()

if(NOT COMMON_ORM_BUILD)
    set(DEMO_ORM_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
    set(DEMO_ORM_SQLITE_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
    set(DEMO_ORM_MYSQL_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
endif()

# ── 公共插件 ─────────────────────────────────────────────────

option(PLUGIN_PERMISSION_BUILD
    "编译 common/plugins/permission（权限管理插件）" ON)
