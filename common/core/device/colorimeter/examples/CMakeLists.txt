set(CMAKE_FOLDER common/core/device/examples)

add_executable(colorimeter_demo colorimeter_demo.cpp)
target_link_libraries(colorimeter_demo PRIVATE ${COMMON_TARGET_NAMESPACE}::colorimeter)
target_include_directories(colorimeter_demo PRIVATE ${PROJECT_SOURCE_DIR})

if(WIN32)
    set(CASDK2_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/colorimeter/konicaminolta/CA-SDK2")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CASDK2_LIB_DIR "${CASDK2_ROOT}/lib/x64")
    else()
        set(CASDK2_LIB_DIR "${CASDK2_ROOT}/lib/x86")
    endif()

    add_custom_command(TARGET colorimeter_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::colorimeter>"
            "$<TARGET_FILE:${COMMON_TARGET_NAMESPACE}::logger>"
            "$<TARGET_FILE_DIR:colorimeter_demo>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2.dll"
            "${CASDK2_LIB_DIR}/CA200Srvr.dll"
            "${CASDK2_LIB_DIR}/CASDK2Calc.dll"
            "${CASDK2_LIB_DIR}/CASDK2Comm.dll"
            "${CASDK2_LIB_DIR}/CASDK2Net.dll"
            "${CASDK2_LIB_DIR}/CASDK2Socket.dll"
            "$<TARGET_FILE_DIR:colorimeter_demo>"
        COMMENT "Copy colorimeter runtime DLLs -> colorimeter_demo output directory"
    )
endif()

