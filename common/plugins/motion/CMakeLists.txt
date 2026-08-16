# ============================================================
# MotionPlugin — 运控调试 UI + 产线 IO 工步
# ============================================================

set(PLUGIN_NAME Motion)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    motionplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/step
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::scheduler
    ${COMMON_TARGET_NAMESPACE}::device_manager
)

fz2_common_target_exists(motion_control _has_motion_control)
if(_has_motion_control)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
endif()

fz2_target_msvc_options(${PLUGIN_NAME})

fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/motion")

fz2_source_group_tree(motion FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [MotionPlugin] Plugin target configured.")
