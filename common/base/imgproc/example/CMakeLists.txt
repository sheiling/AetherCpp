add_executable(imgproc_example
    main.cpp
)

target_link_libraries(imgproc_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::base
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

target_compile_features(imgproc_example PRIVATE cxx_std_17)
fz2_target_msvc_options(imgproc_example)

set_target_properties(imgproc_example PROPERTIES
    FOLDER "common/base/imgproc"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)
