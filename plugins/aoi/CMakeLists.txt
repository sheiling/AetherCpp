# ============================================================
# AoiPlugin — AOI 固晶检测 UI
# ============================================================

set(PLUGIN_NAME Aoi)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

if(COMMON_ORM_BUILD)
    fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)
else()
    fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS
        ".*/data/orm/.*"
        ".*/repository/orm/.*"
        ".*/repository/param_change/.*"
        ".*/repository/inspection_history/.*"
    )
endif()

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    aoiplugin.json
)

target_compile_definitions(${PLUGIN_NAME} PRIVATE ENABLE_DL_BONDING_DETECT)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm/home
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm/recipe_config
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm/review
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/view/review
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/binding
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/binding/fsm
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/dataDefinition
    ${CMAKE_CURRENT_SOURCE_DIR}/data/dataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/data/dataModel/base
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection/session
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection/detect
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection/result
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection/virtual_inspect
    ${CMAKE_CURRENT_SOURCE_DIR}/service/inspection/history
    ${CMAKE_CURRENT_SOURCE_DIR}/service/marker
    ${CMAKE_CURRENT_SOURCE_DIR}/service/vision_bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/service/fov_planner
    ${CMAKE_CURRENT_SOURCE_DIR}/service/flow
    ${CMAKE_CURRENT_SOURCE_DIR}/service/capture
    ${CMAKE_CURRENT_SOURCE_DIR}/service/path_planner
    ${CMAKE_CURRENT_SOURCE_DIR}/service/affineTransform
    ${CMAKE_CURRENT_SOURCE_DIR}/step
    ${CMAKE_CURRENT_SOURCE_DIR}/repository/inspection_history
    ${CMAKE_CURRENT_SOURCE_DIR}/repository/orm
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm/inspection_history
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/view/inspection_history
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/core
    ${PROJECT_SOURCE_DIR}/common/core/scheduler
    ${PROJECT_SOURCE_DIR}/plugins/camera/include
    ${PROJECT_SOURCE_DIR}/plugins/motion_control/include
    ${PROJECT_SOURCE_DIR}/plugins/vision_algo/include
    ${PROJECT_SOURCE_DIR}/common/core/config/include
    ${PROJECT_SOURCE_DIR}/common/utility/diagnostics
)

set(CISDK_MEASUREMENT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement")

function(aoi_link_cisdk_measurement_lib target lib_base_name)
    target_link_libraries(${target} PRIVATE
        "$<$<CONFIG:Debug>:${CISDK_MEASUREMENT_ROOT}/lib/Debug/${lib_base_name}d.lib>"
        "$<$<CONFIG:Release>:${CISDK_MEASUREMENT_ROOT}/lib/Release/${lib_base_name}.lib>"
        "$<$<CONFIG:RelWithDebInfo>:${CISDK_MEASUREMENT_ROOT}/lib/RelWithDebInfo/${lib_base_name}.lib>"
    )
endfunction()

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::ui_component
    ${COMMON_TARGET_NAMESPACE}::mvvm
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::config
    ${COMMON_TARGET_NAMESPACE}::scheduler
    ${COMMON_TARGET_NAMESPACE}::messagecenter
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
    $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)
if(COMMON_ORM_BUILD)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::orm)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE COMMON_ORM_ENABLED=1)
endif()

# UI 图像显示（AlgoMat）仍依赖 measurement 库；检测 CISDK 由 VisionAlgo 插件承载
aoi_link_cisdk_measurement_lib(${PLUGIN_NAME} algo_mat)

# ICameraService / IMotionControlService 含 Q_OBJECT 信号；VisionAlgo 导出 startupIssue 等
if(NOT TARGET VisionAlgo)
    message(FATAL_ERROR "Aoi 依赖 VisionAlgo，请设置 -DPLUGIN_VISION_ALGO_BUILD=ON 并重新 CMake configure。")
endif()
if(NOT TARGET Camera)
    message(FATAL_ERROR "Aoi 依赖 Camera，请设置 -DPLUGIN_CAMERA_BUILD=ON 并重新 CMake configure。")
endif()
if(NOT TARGET MotionControl)
    message(FATAL_ERROR "Aoi 依赖 MotionControl，请设置 -DPLUGIN_MOTION_CONTROL_BUILD=ON 并重新 CMake configure。")
endif()
foreach(_aoi_plugin_dep IN ITEMS VisionAlgo Camera MotionControl)
    add_dependencies(${PLUGIN_NAME} ${_aoi_plugin_dep})
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${_aoi_plugin_dep})
endforeach()

add_dependencies(${PLUGIN_NAME} ${COMMON_TARGET_NAMESPACE}::ui_component)

# 工步层直访 DeviceManager / ImageCapture（service/capture/capture_device_access 等）
fz2_common_target_exists(device_manager _aoi_has_device_manager)
if(_aoi_has_device_manager)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::device_manager)
endif()
fz2_common_target_exists(motion_control _aoi_has_motion_control)
if(_aoi_has_motion_control)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
endif()
fz2_common_target_exists(image_capture _aoi_has_image_capture)
if(_aoi_has_image_capture)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::image_capture)
endif()

fz2_target_msvc_options(${PLUGIN_NAME})
if(MSVC)
    # scheduler 导出类含 STL 成员，插件侧 include 时抑制 C4251
    target_compile_options(${PLUGIN_NAME} PRIVATE /wd4251)
endif()

fz2_setup_plugin(${PLUGIN_NAME} "plugins/aoi")

fz2_source_group_tree(aoi FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# 构建 Aoi 前先完成主程序（含 app 翻译与 splash 等资源 POST_BUILD），再执行下方 config 覆盖。
if(TARGET ${APP_EXECUTABLE_NAME})
    add_dependencies(${PLUGIN_NAME} ${APP_EXECUTABLE_NAME})
endif()

# ── 构建后：将 AOI config/ 覆盖到运行目录 config/ ────────────────────────────
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config"
    COMMENT "Overlaying AOI config -> config"
)

add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config/sql"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMENT "Refreshing config/sql"
)

fz2_target_compile_plugin_translations(${PLUGIN_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n"
    "aoi_zh_CN.ts;aoi_en_US.ts"
)

message(STATUS "  [AoiPlugin] Plugin target configured.")

# Build tests if enabled
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
