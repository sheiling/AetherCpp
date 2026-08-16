cmake_minimum_required(VERSION 3.16)

set(CMAKE_FOLDER common/core/device/camera)

set(GALAXY_SDK_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/camera/galaxy")
set(GALAXY_SDK_INCLUDE_DIR "${GALAXY_SDK_ROOT}/include")
set(GALAXY_SDK_LIB_DIR "${GALAXY_SDK_ROOT}/lib/x64")

set(GALAXY_CAMERA_SOURCES
    galaxy_camera.cpp
    galaxy_camera.h
    galaxy_camera_global.h
    ../camera_base.cpp
    ../camera_base.h
)

fz2_add_common_library(galaxy_camera SHARED ${GALAXY_CAMERA_SOURCES})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt}
    PRIVATE
        GALAXY_CAMERA_LIBRARY
        BUILD_CAMERA_GALAXY
)

target_include_directories(${_tgt}
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/common"
        "${PROJECT_SOURCE_DIR}/common/include"
        "${PROJECT_SOURCE_DIR}/common/3rdparty"
        "${GALAXY_SDK_INCLUDE_DIR}"
)

# Galaxy SDK headers通常带有 #pragma comment(lib, "...")，需要给使用方提供 lib 搜索路径
if (WIN32)
    target_link_directories(${_tgt}
        INTERFACE
            "${GALAXY_SDK_LIB_DIR}"
    )
endif()

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::errors
        ${COMMON_TARGET_NAMESPACE}::logger
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::messagecenter
        # Galaxy SDK (import libs) - use full paths to avoid leaking link_directories to dependents
        "${GALAXY_SDK_LIB_DIR}/GxIAPICPPEx.lib"
        "${GALAXY_SDK_LIB_DIR}/GCBase_MD_VC120_v3_0.lib"
        "${GALAXY_SDK_LIB_DIR}/GenApi_MD_VC120_v3_0.lib"
        "${GALAXY_SDK_LIB_DIR}/GenCP_MD_VC120_v3_0.lib"
        "${GALAXY_SDK_LIB_DIR}/Log_MD_VC120_v3_0.lib"
)

fz2_setup_target(${_tgt} "common/core/device/camera/galaxy_camera")
fz2_target_msvc_options(${_tgt})

fz2_source_group_tree(galaxy_camera ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." FILES ${GALAXY_CAMERA_SOURCES})

if (WIN32)
    file(GLOB GALAXY_SDK_RUNTIME_DLLS CONFIGURE_DEPENDS "${GALAXY_SDK_LIB_DIR}/*.dll")
    if (GALAXY_SDK_RUNTIME_DLLS)
        set(_galaxy_sdk_copy_cmds)
        foreach(_dll IN LISTS GALAXY_SDK_RUNTIME_DLLS)
            list(APPEND _galaxy_sdk_copy_cmds
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_dll}"
                    "$<TARGET_FILE_DIR:${_tgt}>"
            )
        endforeach()
        add_custom_command(TARGET ${_tgt} POST_BUILD
            ${_galaxy_sdk_copy_cmds}
            COMMENT "Copy Galaxy SDK DLLs (if changed) -> $<TARGET_FILE_DIR:${_tgt}>"
        )
    endif()
endif()

message(STATUS "    [galaxy_camera] Configured (SHARED library)")

