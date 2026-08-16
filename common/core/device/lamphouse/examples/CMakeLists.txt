set(CMAKE_FOLDER common/core/examples)

add_executable(lamphouse_demo lamphouse_demo.cpp)
target_link_libraries(lamphouse_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::lamphouse)
target_include_directories(lamphouse_demo PRIVATE ${PROJECT_SOURCE_DIR})

fz2_setup_target(lamphouse_demo "common/core/device/lamphouse/examples")

if(WIN32)
    add_custom_command(TARGET lamphouse_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::lamphouse>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::serial_comm>"
            "$<TARGET_FILE_DIR:lamphouse_demo>"
        COMMENT "Copy lamphouse/serial runtime DLLs -> lamphouse_demo output directory"
    )
endif()
