# ============================================================
# PermissionPlugin
# ============================================================

set(PLUGIN_NAME Permission)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

# 递归收集当前目录下所有 .cpp / .h / .hpp 源文件
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    permissionplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/repository/local
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/service
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::ui_component
    ${COMMON_TARGET_NAMESPACE}::mvvm
)

if(WIN32)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

fz2_target_msvc_options(${PLUGIN_NAME})

fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/permission")

fz2_source_group_tree(permission FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

add_dependencies(${PLUGIN_NAME} ${COMMON_TARGET_NAMESPACE}::ui_component)

# 单独编插件时同步 SQL 到 exe 目录（与 ${APP_EXECUTABLE_NAME} 输出目录一致）
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config/sql"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
    COMMENT "Refreshing config/sql -> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/sql"
)

fz2_target_compile_plugin_translations(${PLUGIN_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n"
    "permission_zh_CN.ts;permission_en_US.ts"
)

message(STATUS "  [PermissionPlugin] Plugin target configured.")
