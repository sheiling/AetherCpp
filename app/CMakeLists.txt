# ============================================================
# app — 主程序可执行文件
# ============================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AppOptions)

# 主程序目标名 / 输出 exe 基名（不含 .exe）；示例：cmake -DAPP_EXECUTABLE_NAME=Aether ..
set(APP_EXECUTABLE_NAME "Aether" CACHE STRING
    "Main application executable target and output base name")
string(REPLACE ".exe" "" APP_EXECUTABLE_NAME "${APP_EXECUTABLE_NAME}")
if(APP_EXECUTABLE_NAME STREQUAL "")
    set(APP_EXECUTABLE_NAME "Aether")
endif()
message(STATUS "[app] APP_EXECUTABLE_NAME: ${APP_EXECUTABLE_NAME}")

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_EXECUTABLE_NAME})

message(STATUS "[app] Configuring...")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

set(_app_subsystem_flag "")
if(WIN32 AND NOT APP_ENABLE_CONSOLE)
    set(_app_subsystem_flag WIN32)
endif()

add_executable(${APP_EXECUTABLE_NAME} ${_app_subsystem_flag}
    main.cpp
    splashscreen.cpp
    resources/app_resources.qrc
    bootstrap/application_bootstrap.cpp
    bootstrap/core_modules.cpp
    bootstrap/plugin_loader_module.cpp
    bootstrap/splash_screen_module.cpp
    $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc>
)

target_include_directories(${APP_EXECUTABLE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(_app_core_libs
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::messagecenter
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::config
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::container
    ${COMMON_TARGET_NAMESPACE}::device_manager
    ${COMMON_TARGET_NAMESPACE}::scheduler
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
if(COMMON_ORM_BUILD)
    list(APPEND _app_core_libs ${COMMON_TARGET_NAMESPACE}::orm)
endif()
target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE ${_app_core_libs})

if(COMMON_ORM_BUILD)
    target_compile_definitions(${APP_EXECUTABLE_NAME} PRIVATE COMMON_ORM_ENABLED=1)
endif()

fz2_common_target_exists(motion_control _app_has_motion_control)
if(_app_has_motion_control)
    target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
endif()

# include 路径由 extensionsystem / uibase 通过 PUBLIC 自动传播：
#   extensionsystem → common/core/  → 允许 extensionsystem/pluginmanager.h
#   extensionsystem → 项目根 / common/include → 允许 common/include/interface/igraphicsView.h
#   uibase          → common/ui/    → 允许 uibase/xxx.h, corelib/xxx.h
#   uibase          → common/ui/uibase/ → 允许 thememanager.h（直接包含）

fz2_target_msvc_options(${APP_EXECUTABLE_NAME})

# ── 输出目录（Debug / Release / RelWithDebInfo 各自独立）+ VS 文件夹 ─────────
fz2_setup_target(${APP_EXECUTABLE_NAME} "app")

fz2_collect_sources(_APP_CPP_SOURCES _APP_HEADERS)
fz2_source_group_tree(app FILES ${_APP_CPP_SOURCES} ${_APP_HEADERS})

# ── 构建后：编译主程序翻译（.ts -> .qm，输出到运行目录 config/）──────────────
fz2_target_compile_app_translations(${APP_EXECUTABLE_NAME}
    "${CMAKE_SOURCE_DIR}/config"
    "app_zh_CN.ts;app_en_US.ts"
)

# ── 构建后：拷贝内置启动图到 config/splash.ico（文件系统回退路径）──────────
add_custom_command(TARGET ${APP_EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/image/app_splash.ico"
        "$<TARGET_FILE_DIR:${APP_EXECUTABLE_NAME}>/config/splash.ico"
    COMMENT "Copying app/resources/image/app_splash.ico -> config/splash.ico"
)

# ── 构建后：运行 windeployqt 部署 Qt 运行时 ──────────────────────────────────
if(WIN32)
    get_target_property(_qmake_exe Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${_qt_bin_dir}"
        NO_DEFAULT_PATH
    )

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "  [app] windeployqt found: ${WINDEPLOYQT_EXECUTABLE}")
        add_custom_command(TARGET ${APP_EXECUTABLE_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --sql
                "$<IF:$<CONFIG:Debug>,--debug,--release>"
                "$<TARGET_FILE:${APP_EXECUTABLE_NAME}>"
            COMMENT "Deploying Qt runtime ($<CONFIG>)..."
            VERBATIM
        )
    else()
        message(WARNING "windeployqt not found; Qt DLLs will NOT be auto-deployed to output dir.")
    endif()
endif()

# ── 构建后：补充部署 Qt6OpenGLWidgets（windeployqt 可能遗漏插件依赖）──
if(WIN32)
    add_custom_command(TARGET ${APP_EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/ProgramInstall/Qt/6.11.1/msvc2022_64/bin/Qt6OpenGLWidgets.dll"
            "$<TARGET_FILE_DIR:${APP_EXECUTABLE_NAME}>/Qt6OpenGLWidgets.dll"
        COMMENT "Copying Qt6OpenGLWidgets.dll..."
    )
endif()

message(STATUS "  [app] Executable target configured: ${APP_EXECUTABLE_NAME}")
