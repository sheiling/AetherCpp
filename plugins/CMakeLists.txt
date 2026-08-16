# ============================================================
# Plugins — 业务插件汇总
# ============================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PluginOptions)

message(STATUS "[plugins] 正在配置业务插件...")

if(PLUGIN_CAMERA_BUILD)
    add_subdirectory(camera)
else()
    message(STATUS "  [plugins] 跳过 Camera 插件（PLUGIN_CAMERA_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Camera)
endif()

if(PLUGIN_MOTION_CONTROL_BUILD)
    add_subdirectory(motion_control)
else()
    message(STATUS "  [plugins] 跳过 MotionControl 插件（PLUGIN_MOTION_CONTROL_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(MotionControl)
endif()

if(PLUGIN_VISION_ALGO_BUILD)
    add_subdirectory(vision_algo)
else()
    message(STATUS "  [plugins] 跳过 VisionAlgo 插件（PLUGIN_VISION_ALGO_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(VisionAlgo)
endif()

if(PLUGIN_MES_BUILD)
    add_subdirectory(mes)
else()
    message(STATUS "  [plugins] 跳过 Mes 插件（PLUGIN_MES_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Mes)
endif()

if(PLUGIN_LIGHTEN_BUILD)
    add_subdirectory(lighten)
else()
    message(STATUS "  [plugins] 跳过 Lighten 插件（PLUGIN_LIGHTEN_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Lighten)
endif()

if(PLUGIN_AOI_BUILD)
    add_subdirectory(aoi)
else()
    message(STATUS "  [plugins] 跳过 Aoi 插件（PLUGIN_AOI_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Aoi)
endif()

if(DEMO_PLUGINS_EXAMPLE_BUILD)
    add_subdirectory(example)
else()
    message(STATUS "  [plugins] 跳过示例插件（DEMO_PLUGINS_EXAMPLE_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Home Settings MvvmPlugin Middleware Invoker ContainerPlugin)
endif()


message(STATUS "[plugins] 业务插件配置完成")
