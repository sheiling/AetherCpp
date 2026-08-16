set(CMAKE_FOLDER common/core/device)

fz2_collect_sources(COLORIMETER_SRCS COLORIMETER_HEADERS)

fz2_add_common_library(colorimeter SHARED ${COLORIMETER_SRCS} ${COLORIMETER_HEADERS})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE COLORIMETER_LIBRARY)

# CA-SDK2 (Konica Minolta Colorimeter SDK) paths
set(CASDK2_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/device/colorimeter/konicaminolta/CA-SDK2")
set(CASDK2_INCLUDE_DIR "${CASDK2_ROOT}/include")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CASDK2_LIB_DIR "${CASDK2_ROOT}/lib/x64")
else()
    set(CASDK2_LIB_DIR "${CASDK2_ROOT}/lib/x86")
endif()

set(CASDK2_LIB "${CASDK2_LIB_DIR}/CASDK2.lib")

target_link_libraries(${_tgt}
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::logger
        ${CASDK2_LIB}
)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${CASDK2_INCLUDE_DIR}
)

fz2_setup_target(${_tgt} "common/core/device/colorimeter")

fz2_source_group_tree(colorimeter FILES ${COLORIMETER_SRCS} ${COLORIMETER_HEADERS})

if(WIN32)
    add_custom_command(TARGET ${_tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CA200Srvr.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2Calc.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2Comm.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2Net.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CASDK2_LIB_DIR}/CASDK2Socket.dll"
            "$<TARGET_FILE_DIR:${_tgt}>"
        COMMENT "Copy CASDK2 runtime DLLs -> colorimeter output directory"
    )
endif()

if(DEMO_COLORIMETER_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
