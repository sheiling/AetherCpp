set(CMAKE_FOLDER common/base/algorithm/example)

add_executable(algorithm_demo
    algorithm_demo.cpp
)

target_include_directories(algorithm_demo
    PRIVATE
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/include
)

target_link_libraries(algorithm_demo
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::base
        ${COMMON_TARGET_NAMESPACE}::errors
        $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${PROJECT_SOURCE_DIR}/common/3rdparty/opencv_4.10.0/lib/opencv_world4100.lib>
)

fz2_target_msvc_options(algorithm_demo)
fz2_setup_target(algorithm_demo "common/base/algorithm/example")

