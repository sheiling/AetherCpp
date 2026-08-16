set(CMAKE_FOLDER common/core/device/examples)

add_executable(software_screen_demo winscreen_demo.cpp)
target_link_libraries(software_screen_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::software_screen)
target_include_directories(software_screen_demo PRIVATE ${PROJECT_SOURCE_DIR})

if(WIN32)
    add_custom_command(TARGET software_screen_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::software_screen>"
            "$<TARGET_FILE_DIR:software_screen_demo>"
        COMMENT "Copy software_screen runtime DLLs -> software_screen_demo output directory"
    )
endif()
