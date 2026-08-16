# ============================================================
# MesPlugin — MES 对接插件
# ============================================================

if(NOT PLUGIN_MES_BUILD)
    return()
endif()

set(PLUGIN_NAME Mes)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    mesplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/vm
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/3rdparty
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::mvvm
    ${COMMON_TARGET_NAMESPACE}::http_comm
)

fz2_target_msvc_options(${PLUGIN_NAME})

fz2_setup_plugin(${PLUGIN_NAME} "plugins/mes")

fz2_source_group_tree(mes FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [MesPlugin] Plugin target configured.")
