set(CMAKE_FOLDER common/core/device/imageCapture)

if(NOT HW_IMAGE_CAPTURE_BUILD)
    return()
endif()

set(CISDK_MEASUREMENT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement")

function(image_capture_link_cisdk_lib target lib_base_name)
    target_link_libraries(${target} PRIVATE
        "$<$<CONFIG:Debug>:${CISDK_MEASUREMENT_ROOT}/lib/Debug/${lib_base_name}d.lib>"
        "$<$<CONFIG:Release>:${CISDK_MEASUREMENT_ROOT}/lib/Release/${lib_base_name}.lib>"
        "$<$<CONFIG:RelWithDebInfo>:${CISDK_MEASUREMENT_ROOT}/lib/RelWithDebInfo/${lib_base_name}.lib>"
    )
endfunction()

fz2_add_common_library(image_capture SHARED
    image_capture.cpp
    image_capture.h
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_features(${_tgt} PUBLIC cxx_std_17)
target_compile_definitions(${_tgt} PRIVATE IMAGE_CAPTURE_LIBRARY)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/cisdk/measurement/include
        ${PROJECT_SOURCE_DIR}/common/core/config/include
        ${PROJECT_SOURCE_DIR}/common/utility/diagnostics
    PRIVATE
        ${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/include
)

target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::base
        ${COMMON_TARGET_NAMESPACE}::logger
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::config
        ${COMMON_TARGET_NAMESPACE}::lamphouse
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

foreach(_lib IN ITEMS algo_mat ci_utils)
    image_capture_link_cisdk_lib(${_tgt} ${_lib})
endforeach()

set_target_properties(${_tgt} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(WIN32)
    function(image_capture_copy_cisdk_runtime_dll target dll_stem)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CISDK_MEASUREMENT_ROOT}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
                "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${dll_stem}$<$<CONFIG:Debug>:d>.dll"
        )
    endfunction()
    foreach(_dll IN ITEMS algo_mat ci_utils)
        image_capture_copy_cisdk_runtime_dll(${_tgt} ${_dll})
    endforeach()
endif()

fz2_target_msvc_options(${_tgt})
fz2_setup_target(${_tgt} "common/core/device/imageCapture")

message(STATUS "    [image_capture] Configured (SHARED library)")
