# ============================================================
# LightenPlugin — 点亮检测配置界面插件
# ============================================================

if(NOT PLUGIN_LIGHTEN_BUILD)
    return()
endif()

set(PLUGIN_NAME Lighten)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

if(COMMON_ORM_BUILD)
    fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)
else()
    fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS
        ".*/repository/orm/.*"
        ".*/repository/sqlite/.*"
        ".*/repository/mysql/.*"
    )
endif()

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    lightenplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/repository
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/common/3rdparty
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::messagecenter
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::ui_component
    ${COMMON_TARGET_NAMESPACE}::mvvm
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::device_manager
    ${COMMON_TARGET_NAMESPACE}::base
    ${COMMON_TARGET_NAMESPACE}::cam_async
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::scheduler
)
if(COMMON_ORM_BUILD)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::orm)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE COMMON_ORM_ENABLED=1)
endif()

fz2_common_target_exists(motion_control _has_motion_control)
if(_has_motion_control)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
endif()

add_dependencies(${PLUGIN_NAME} ${COMMON_TARGET_NAMESPACE}::ui_component)

fz2_target_msvc_options(${PLUGIN_NAME})

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX ""
    FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS}
)

fz2_setup_plugin(${PLUGIN_NAME} "plugins/lighten")

if(TARGET LightenDetector)
    add_dependencies(${PLUGIN_NAME} LightenDetector)
endif()

# ── 构建后：将 Lighten config/ 覆盖到运行目录 config/ ────────────────────────
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config"
    COMMENT "Overlaying Lighten config -> config"
)

add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config/sql"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMENT "Refreshing config/sql"
)

message(STATUS "  [LightenPlugin] UI plugin target configured.")
