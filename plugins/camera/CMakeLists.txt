# ============================================================
# CameraPlugin — 相机采图服务（ICameraService + ImageCapture）
# ============================================================

set(PLUGIN_NAME Camera)

set(CISDK_MEASUREMENT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement")

function(camera_link_cisdk_lib target lib_base_name)
    target_link_libraries(${target} PRIVATE
        "$<$<CONFIG:Debug>:${CISDK_MEASUREMENT_ROOT}/lib/Debug/${lib_base_name}d.lib>"
        "$<$<CONFIG:Release>:${CISDK_MEASUREMENT_ROOT}/lib/Release/${lib_base_name}.lib>"
        "$<$<CONFIG:RelWithDebInfo>:${CISDK_MEASUREMENT_ROOT}/lib/RelWithDebInfo/${lib_base_name}.lib>"
    )
endfunction()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Concurrent)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    cameraplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PLUGIN_NAME} PRIVATE CAMERA_PLUGIN_LIBRARY)

target_include_directories(${PLUGIN_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/service
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/include
        ${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement/include
        ${PROJECT_SOURCE_DIR}/common/core/config/include
        ${PROJECT_SOURCE_DIR}/common/utility/diagnostics
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::device_manager
    ${COMMON_TARGET_NAMESPACE}::image_capture
    ${COMMON_TARGET_NAMESPACE}::config
    ${COMMON_TARGET_NAMESPACE}::errors
    ${COMMON_TARGET_NAMESPACE}::logger
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
    $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

foreach(_lib IN ITEMS algo_mat ci_utils)
    camera_link_cisdk_lib(${PLUGIN_NAME} ${_lib})
endforeach()

fz2_target_msvc_options(${PLUGIN_NAME})
fz2_setup_plugin(${PLUGIN_NAME} "plugins/camera")

fz2_target_source_group_tree(${PLUGIN_NAME} camera FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

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

message(STATUS "  [CameraPlugin] Plugin target configured.")
