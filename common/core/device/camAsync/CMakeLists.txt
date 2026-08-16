set(CMAKE_FOLDER common/core/device/camAsync)

if(NOT HW_CAM_ASYNC_BUILD)
    return()
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

fz2_collect_sources(CAM_ASYNC_SRCS CAM_ASYNC_HEADERS)

fz2_add_common_library(cam_async SHARED
    ${CAM_ASYNC_SRCS}
    ${CAM_ASYNC_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_features(${_tgt} PUBLIC cxx_std_17)

target_compile_definitions(${_tgt} PRIVATE CAM_ASYNC_LIBRARY)

target_include_directories(${_tgt}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
)

target_link_libraries(${_tgt}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::messagecenter
        ${COMMON_TARGET_NAMESPACE}::container
        ${COMMON_TARGET_NAMESPACE}::device_manager
        ${COMMON_TARGET_NAMESPACE}::base
        ${COMMON_TARGET_NAMESPACE}::logger
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

if(WIN32)
    add_custom_command(TARGET ${_tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/bin/$<$<CONFIG:Debug>:opencv_world4100d>$<$<NOT:$<CONFIG:Debug>>:opencv_world4100>.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMENT "[CamAsync] Copy OpenCV runtime DLL -> output directory"
    )
endif()

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/device/camAsync")

fz2_source_group_tree(cam_async FILES ${CAM_ASYNC_SRCS} ${CAM_ASYNC_HEADERS})

message(STATUS "    [CamAsync] Configured (SHARED library)")
