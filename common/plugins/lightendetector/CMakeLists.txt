# ============================================================
# LightenDetectorPlugin
# ============================================================

if(NOT PLUGIN_LIGHTEN_DETECTOR_BUILD)
    return()
endif()

set(PLUGIN_NAME LightenDetector)

set(CISDK_MEASUREMENT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement")
set(CISDK_DETECTION_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/detection")

# CISDK import libs: Debug 使用 *d.lib，Release / RelWithDebInfo 与各配置子目录对应
function(lightendetector_link_cisdk_lib target is_detection lib_base_name)
    if(is_detection)
        set(_root "${CISDK_DETECTION_ROOT}")
    else()
        set(_root "${CISDK_MEASUREMENT_ROOT}")
    endif()
    target_link_libraries(${target} PRIVATE
        "$<$<CONFIG:Debug>:${_root}/lib/Debug/${lib_base_name}d.lib>"
        "$<$<CONFIG:Release>:${_root}/lib/Release/${lib_base_name}.lib>"
        "$<$<CONFIG:RelWithDebInfo>:${_root}/lib/RelWithDebInfo/${lib_base_name}.lib>"
    )
endfunction()

# 拷贝单个 CISDK 模块 DLL 至 exe 输出目录（Debug 为 *d.dll，与 lib 命名一致）
function(lightendetector_copy_cisdk_runtime_dll target cisdk_root dll_stem)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${cisdk_root}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
    )
endfunction()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Recursively collect current directory source/header files
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    lightendetectorplugin.json
)


# VS 中按 lightendetector 目录实际结构展示文件（含 algorithm / service 子目录）
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX ""
    FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS}
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/step
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement/include
    ${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/detection/include
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::scheduler
    ${COMMON_TARGET_NAMESPACE}::device_manager
    ${COMMON_TARGET_NAMESPACE}::messagecenter
)

# 与 legacy lightendetector 工程 CISDK_DEPENDENCIES 对齐（detection + measurement）
lightendetector_link_cisdk_lib(${PLUGIN_NAME} ON led_meter_inspector)
foreach(_cisdk_lib IN ITEMS
    algo_led_meter
    led_lum_extractor
    led_point_locator
    led_contour_searcher
    roi_detector
    algo_mat
    algo_shot
    algo_log
    ci_utils
    point_finder
    point_mapper
)
    lightendetector_link_cisdk_lib(${PLUGIN_NAME} OFF ${_cisdk_lib})
endforeach()

fz2_target_msvc_options(${PLUGIN_NAME})

# Output directory + VS folder
fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/lightendetector")

fz2_source_group_tree(lightendetector FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# CISDK 运行库 -> 主程序输出目录（bin/<Config>/）；按模块逐个 copy_if_different，不拷贝整 bin
# 此处不用 $<TARGET_FILE_DIR:${APP_EXECUTABLE_NAME}>：configure common/plugins 时主程序尚未定义。
if(WIN32)
    lightendetector_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" led_meter_inspector)

    foreach(_dll IN ITEMS
        algo_led_meter
        led_lum_extractor
        led_point_locator
        led_contour_searcher
        roi_detector
        algo_mat
        algo_shot
        algo_log
        ci_utils
        point_finder
        point_mapper
    )
        lightendetector_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_MEASUREMENT_ROOT}" ${_dll})
    endforeach()

    # detection CISDK（led_meter_inspector 等）依赖 OpenCV 4.3，与 measurement / Lighten 的 4.10 并存
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CISDK_DETECTION_ROOT}/Opencv430/vc15/opencv_world430$<$<CONFIG:Debug>:d>.dll"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/opencv_world430$<$<CONFIG:Debug>:d>.dll"
        COMMENT "[LightenDetector] Copy CISDK detection OpenCV 4.3 runtime DLL"
    )
endif()

message(STATUS "  [LightenDetectorPlugin] Plugin target configured.")
