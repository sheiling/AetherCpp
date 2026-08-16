set(CMAKE_FOLDER common/core/examples)

add_executable(rangefinder_demo rangefinder_demo.cpp)
target_link_libraries(rangefinder_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::rangefinder)
target_include_directories(rangefinder_demo PRIVATE ${PROJECT_SOURCE_DIR})

if(WIN32)
    add_custom_command(TARGET rangefinder_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::rangefinder>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::regaccess_modbus>"
            "$<TARGET_FILE_DIR:rangefinder_demo>"
        COMMENT "Copy rangefinder runtime DLLs -> rangefinder_demo output directory"
    )
endif()

