# ============================================================
# VisionAlgoPlugin — 统一视觉算法插件（CISDK + ZXing）
# ============================================================

set(PLUGIN_NAME VisionAlgo)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/VisionAlgoUnits.cmake)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CISDK_MEASUREMENT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement")
set(CISDK_DETECTION_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/detection")
set(ZXING_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/zxing-cpp")

set(_VISION_ALGO_NEEDS_CISDK_DETECTION OFF)
if(VISION_ALGO_ENABLE_MODEL OR VISION_ALGO_ENABLE_MARKER OR VISION_ALGO_ENABLE_CHIP OR VISION_ALGO_ENABLE_DIE_BONDING)
    set(_VISION_ALGO_NEEDS_CISDK_DETECTION ON)
endif()

# --- 源文件（按单元开关增减）---
set(VISION_ALGO_SOURCES
    visionalgoplugin.cpp
    service/vision_algo_service_impl.cpp
    include/ivision_algo_service.cpp
    src/core/vision_algo_i18n.cpp
    src/core/image_utils.cpp
    src/core/algo_ci_data_convert.cpp
    src/core/algo_ci_error_parser.cpp
    src/core/algo_ci_error_check.cpp
)

if(VISION_ALGO_ENABLE_MODEL)
    list(APPEND VISION_ALGO_SOURCES
        src/units/model/model_processor_unit.cpp
    )
endif()

if(VISION_ALGO_ENABLE_MARKER)
    list(APPEND VISION_ALGO_SOURCES
        src/units/marker/mark_point_locate.cpp
    )
endif()

if(VISION_ALGO_ENABLE_CHIP)
    list(APPEND VISION_ALGO_SOURCES
        src/units/chip_detector/chip_detector.cpp
    )
endif()

if(VISION_ALGO_ENABLE_DIE_BONDING)
    list(APPEND VISION_ALGO_SOURCES
        src/units/die_bonding/die_bonding_chip_detect.cpp
        src/units/die_bonding/die_bonding_detect_engine.cpp
        src/units/die_bonding/die_bonding_engine_manager.cpp
        src/units/die_bonding/die_handle_pool.cpp
        src/units/die_bonding/dl_model_scanner.cpp
    )
endif()

if(VISION_ALGO_ENABLE_BARCODE)
    list(APPEND VISION_ALGO_SOURCES
        src/units/scan/barcode_scan_unit.cpp
    )
endif()

add_library(${PLUGIN_NAME} SHARED
    ${VISION_ALGO_SOURCES}
    visionalgoplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PLUGIN_NAME}
    PUBLIC
        ${VISION_ALGO_COMPILE_DEFS}
    PRIVATE
        VISION_ALGO_PLUGIN_LIBRARY
)

if(VISION_ALGO_ENABLE_DL_DIE_BONDING)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE
        VISION_ALGO_ENABLE_DL_BONDING_DETECT
        ENABLE_DL_BONDING_DETECT)
endif()

set(VISION_ALGO_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/common/utility/defer/include
    ${PROJECT_SOURCE_DIR}/common/core
    ${CISDK_MEASUREMENT_ROOT}/include
)

if(_VISION_ALGO_NEEDS_CISDK_DETECTION)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CISDK_DETECTION_ROOT}/include)
endif()
if(VISION_ALGO_ENABLE_BARCODE)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${ZXING_ROOT}/include)
endif()
if(VISION_ALGO_ENABLE_MODEL)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/units/model)
endif()
if(VISION_ALGO_ENABLE_MARKER)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/units/marker)
endif()
if(VISION_ALGO_ENABLE_CHIP)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/units/chip_detector)
endif()
if(VISION_ALGO_ENABLE_DIE_BONDING)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/units/die_bonding)
endif()
if(VISION_ALGO_ENABLE_BARCODE)
    list(APPEND VISION_ALGO_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/units/scan)
endif()

target_include_directories(${PLUGIN_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${VISION_ALGO_PRIVATE_INCLUDES}
)

if(_VISION_ALGO_NEEDS_CISDK_DETECTION)
    target_link_directories(${PLUGIN_NAME} PRIVATE
        "${CISDK_DETECTION_ROOT}/lib/$<IF:$<CONFIG:Debug>,Debug,Release>"
    )
endif()
target_link_directories(${PLUGIN_NAME} PRIVATE
    "${CISDK_MEASUREMENT_ROOT}/lib/$<IF:$<CONFIG:Debug>,Debug,Release>"
)

set(VISION_ALGO_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::errors
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
    $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
    $<$<CONFIG:Debug>:algo_matd>
    $<$<NOT:$<CONFIG:Debug>>:algo_mat>
    $<$<CONFIG:Debug>:ci_utilsd>
    $<$<NOT:$<CONFIG:Debug>>:ci_utils>
    $<$<CONFIG:Debug>:algo_logd>
    $<$<NOT:$<CONFIG:Debug>>:algo_log>
)

if(VISION_ALGO_ENABLE_MODEL)
    list(APPEND VISION_ALGO_LIBS
        $<$<CONFIG:Debug>:model_processord>
        $<$<NOT:$<CONFIG:Debug>>:model_processor>
    )
endif()
if(VISION_ALGO_ENABLE_MARKER)
    list(APPEND VISION_ALGO_LIBS
        $<$<CONFIG:Debug>:marker_detectord>
        $<$<NOT:$<CONFIG:Debug>>:marker_detector>
    )
endif()
if(VISION_ALGO_ENABLE_CHIP)
    list(APPEND VISION_ALGO_LIBS
        $<$<CONFIG:Debug>:chip_detectord>
        $<$<NOT:$<CONFIG:Debug>>:chip_detector>
    )
endif()
if(VISION_ALGO_ENABLE_DIE_BONDING)
    list(APPEND VISION_ALGO_LIBS
        $<$<CONFIG:Debug>:die_bonding_defect_detectord>
        $<$<NOT:$<CONFIG:Debug>>:die_bonding_defect_detector>
    )
    if(VISION_ALGO_ENABLE_DL_DIE_BONDING)
        list(APPEND VISION_ALGO_LIBS
            $<$<NOT:$<CONFIG:Debug>>:dl_die_bonding_detector>
        )
    endif()
endif()
if(VISION_ALGO_ENABLE_BARCODE)
    list(APPEND VISION_ALGO_LIBS
        $<$<CONFIG:Debug>:${ZXING_ROOT}/lib/ZXingd.lib>
        $<$<NOT:$<CONFIG:Debug>>:${ZXING_ROOT}/lib/ZXing.lib>
    )
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${VISION_ALGO_LIBS})

fz2_common_target_exists(image_capture _vision_algo_has_image_capture)
if(_vision_algo_has_image_capture)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::image_capture)
endif()

fz2_target_msvc_options(${PLUGIN_NAME})
fz2_setup_plugin(${PLUGIN_NAME} "plugins/vision_algo")

# IDE 树与 VISION_ALGO_SOURCES 一致；公开头文件始终纳入（与编译开关无关）
fz2_expand_tree_files(VISION_ALGO_TREE_FILES SOURCES ${VISION_ALGO_SOURCES}
    EXTRA
        visionalgoplugin.h
        include/vision_algo_plugin_global.h
        include/ivision_algo_service.h
)
fz2_glob_relative_append(VISION_ALGO_TREE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.h"
)
fz2_glob_relative_append(VISION_ALGO_TREE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/units/model/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/units/marker/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/units/chip_detector/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/units/die_bonding/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vision_algo/units/scan/*.h"
)
fz2_target_source_group_tree(${PLUGIN_NAME} vision_algo FILES ${VISION_ALGO_TREE_FILES})

fz2_target_compile_plugin_translations(${PLUGIN_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n"
    "vision_algo_zh_CN.ts;vision_algo_en_US.ts"
)

if(WIN32)
    function(vision_algo_copy_cisdk_runtime_dll target cisdk_root dll_stem)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${cisdk_root}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
                "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
        )
    endfunction()

    function(vision_algo_copy_cisdk_release_runtime_dll target cisdk_root dll_stem)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${cisdk_root}/bin/Release/${dll_stem}.dll"
                "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${dll_stem}.dll"
        )
    endfunction()

    if(VISION_ALGO_ENABLE_MODEL)
        vision_algo_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" model_processor)
    endif()
    if(VISION_ALGO_ENABLE_MARKER)
        vision_algo_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" marker_detector)
    endif()
    if(VISION_ALGO_ENABLE_CHIP)
        vision_algo_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" chip_detector)
    endif()
    if(VISION_ALGO_ENABLE_DIE_BONDING)
        vision_algo_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" die_bonding_defect_detector)
        if(VISION_ALGO_ENABLE_DL_DIE_BONDING)
            vision_algo_copy_cisdk_release_runtime_dll(${PLUGIN_NAME} "${CISDK_DETECTION_ROOT}" dl_die_bonding_detector)
        endif()
    endif()
    foreach(_dll IN ITEMS algo_mat ci_utils algo_log)
        vision_algo_copy_cisdk_runtime_dll(${PLUGIN_NAME} "${CISDK_MEASUREMENT_ROOT}" ${_dll})
    endforeach()
endif()

# ── 运行时 OpenCV DLL ─────────────────────────────────────────
if(WIN32)
    set(OPENCV_DLL_DIR "${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/bin")
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENCV_DLL_DIR}/opencv_world4100$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/opencv_world4100$<$<CONFIG:Debug>:d>.dll"
        COMMENT "Copying OpenCV DLL..."
    )
endif()

message(STATUS "  [VisionAlgo] Plugin configured.")
