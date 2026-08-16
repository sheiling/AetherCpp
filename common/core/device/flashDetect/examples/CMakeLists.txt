set(CMAKE_FOLDER common/core/device/flashDetect/examples)

add_executable(flash_detect_demo
    flash_detect_demo.cpp
)

target_link_libraries(flash_detect_demo
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::flash_detect
)

fz2_target_msvc_options(flash_detect_demo)
fz2_setup_target(flash_detect_demo "common/core/device/flashDetect/examples")
