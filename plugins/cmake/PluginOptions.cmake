# ============================================================
# PluginOptions.cmake
#
# plugins/ 目录编译开关（业务插件与示例）。
# 由 plugins/CMakeLists.txt include。
# ============================================================

# ── 业务插件 ─────────────────────────────────────────────────

option(PLUGIN_LIGHTEN_BUILD
    "编译 plugins/lighten（Lighten 插件）" ON)

option(PLUGIN_AOI_BUILD
    "编译 plugins/aoi（Aoi 插件）" ON)

option(PLUGIN_CAMERA_BUILD
    "编译 plugins/camera（Camera 插件）" ON)

option(PLUGIN_MOTION_CONTROL_BUILD
    "编译 plugins/motion_control（MotionControl 插件）" ON)

option(PLUGIN_VISION_ALGO_BUILD
    "编译 plugins/vision_algo（VisionAlgo 插件）" OFF)

option(PLUGIN_MES_BUILD
    "编译 plugins/mes（Mes 插件）" ON)


# ── 示例与 Demo ──────────────────────────────────────────────

option(DEMO_PLUGINS_EXAMPLE_BUILD
    "编译 plugins/example 下示例插件（Home、Settings、Mvvm 等）" OFF)

option(DEMO_AOI_ALGO_EXAMPLE_BUILD
    "编译 AOI 算法检测控制台示例" ON)
