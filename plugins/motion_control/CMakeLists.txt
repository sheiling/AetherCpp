# ============================================================
# MotionControlPlugin — 运控服务（IMotionControlService）+ 轴参数设置 UI
# ============================================================

set(PLUGIN_NAME MotionControl)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)
list(FILTER PLUGIN_SOURCES EXCLUDE REGEX ".*/include/.*\\.cpp$")

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    motioncontrolplugin.json
    include/imotion_control_service.cpp
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PLUGIN_NAME} PRIVATE MOTION_CONTROL_PLUGIN_LIBRARY)

target_include_directories(${PLUGIN_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/service
        ${CMAKE_CURRENT_SOURCE_DIR}/steps
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/axis
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/io
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialogs
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/core/scheduler
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::ui_component
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::device_manager
    ${COMMON_TARGET_NAMESPACE}::errors
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::scheduler
)

fz2_common_target_exists(motion_control _has_motion_control)
if(_has_motion_control)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
endif()

add_dependencies(${PLUGIN_NAME} ${COMMON_TARGET_NAMESPACE}::ui_component)

fz2_target_compile_plugin_translations(${PLUGIN_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n"
    "motion_control_zh_CN.ts;motion_control_en_US.ts"
)

fz2_target_msvc_options(${PLUGIN_NAME})
fz2_setup_plugin(${PLUGIN_NAME} "plugins/motion_control")

set(_MOTION_PLUGIN_TREE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} include/imotion_control_service.cpp)
fz2_target_source_group_tree(${PLUGIN_NAME} motion_control FILES ${_MOTION_PLUGIN_TREE})

message(STATUS "  [MotionControlPlugin] Plugin target configured.")
