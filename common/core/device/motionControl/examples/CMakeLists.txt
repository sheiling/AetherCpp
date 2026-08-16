set(CMAKE_FOLDER common/core/examples)

add_executable(motion_control_demo motion_control_demo.cpp)
target_link_libraries(motion_control_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::motion_control)
target_include_directories(motion_control_demo PRIVATE ${PROJECT_SOURCE_DIR})

fz2_setup_target(motion_control_demo "common/core/device/motionControl/examples")
fz2_target_msvc_options(motion_control_demo)

