set(CMAKE_FOLDER common/core/examples)

add_executable(scanner_demo scanner_demo.cpp)
target_link_libraries(scanner_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::scanner)
target_include_directories(scanner_demo PRIVATE ${PROJECT_SOURCE_DIR})

fz2_setup_target(scanner_demo "common/core/device/scanner/examples")

if(WIN32)
    add_custom_command(TARGET scanner_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::scanner>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::serial_comm>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::tcp_comm>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::logger>"
            "$<TARGET_FILE_DIR:scanner_demo>"
        COMMENT "Copy scanner runtime DLLs -> scanner_demo output directory"
    )
endif()
