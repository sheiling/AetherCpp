set(CMAKE_FOLDER common/core/device/deviceManager/examples)

add_executable(device_manager_demo
    device_manager_demo.cpp
)

target_link_libraries(device_manager_demo
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::device_manager
)

fz2_target_msvc_options(device_manager_demo)

fz2_setup_target(device_manager_demo "common/core/device/deviceManager/examples")

# 让 demo 在运行目录下也能找到配置文件（提升可运行性）
add_custom_command(TARGET device_manager_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/device_config.json"
        "$<TARGET_FILE_DIR:device_manager_demo>"
)

