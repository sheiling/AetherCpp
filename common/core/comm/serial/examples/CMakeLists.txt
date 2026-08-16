add_executable(serial_example
    main.cpp
)

target_link_libraries(serial_example
    PRIVATE ${COMMON_TARGET_NAMESPACE}::serial_comm
)

fz2_target_msvc_options(serial_example)
fz2_setup_target(serial_example "common/core/comm/serial/examples")
