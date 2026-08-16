set(CMAKE_FOLDER common/plugins)

message(STATUS "  [common/plugins] 正在配置...")

add_subdirectory(core)
if(PLUGIN_PERMISSION_BUILD)
    add_subdirectory(permission)
else()
    message(STATUS "    [common/plugins] 跳过 Permission（PLUGIN_PERMISSION_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(Permission)
endif()
add_subdirectory(graphicsview)
add_subdirectory(help)
add_subdirectory(motion)

if(PLUGIN_LIGHTEN_DETECTOR_BUILD)
    add_subdirectory(lightendetector)
else()
    message(STATUS "    [common/plugins] 跳过 LightenDetector（PLUGIN_LIGHTEN_DETECTOR_BUILD=OFF）")
    fz2_remove_disabled_plugin_artifacts(LightenDetector)
endif()

message(STATUS "  [common/plugins] 配置完成")
