set(CMAKE_FOLDER common/core/device)

# -----------------------------------------------------------------------------
# libcserialport 预编译
# 目录约定：
#   include/CSerialPort/SerialPort.h
#   lib/Debug/libcserialport.lib(.dll)
#   lib/Release/libcserialport.lib(.dll)
set(CSERIALPORT_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/cserialport")
set(CSERIALPORT_INCLUDE_DIR "${CSERIALPORT_ROOT}/include")
set(CSERIALPORT_LIB_DIR_DEBUG "${CSERIALPORT_ROOT}/lib/Debug")
set(CSERIALPORT_LIB_DIR_RELEASE "${CSERIALPORT_ROOT}/lib/Release")

if (NOT EXISTS "${CSERIALPORT_INCLUDE_DIR}/CSerialPort/SerialPort.h")
    message(WARNING "CSerialPort headers not found: ${CSERIALPORT_INCLUDE_DIR}/CSerialPort/SerialPort.h")
endif()

fz2_collect_sources(RANGEFINDER_SRCS RANGEFINDER_HEADERS)

fz2_add_common_library(rangefinder SHARED ${RANGEFINDER_SRCS} ${RANGEFINDER_HEADERS})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE RANGEFINDER_LIBRARY)
target_link_libraries(${_tgt}
    PUBLIC
        ${COMMON_TARGET_NAMESPACE}::regaccess_modbus
    PRIVATE
        libcserialport
)

target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${CSERIALPORT_INCLUDE_DIR}
)

# 按配置链接 libcserialport
# RelWithDebInfo 复用 Release 库
target_link_directories(${_tgt} PRIVATE
    $<$<CONFIG:Debug>:${CSERIALPORT_LIB_DIR_DEBUG}>
    $<$<CONFIG:Release>:${CSERIALPORT_LIB_DIR_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>:${CSERIALPORT_LIB_DIR_RELEASE}>
)

# 拷贝 libcserialport 运行时 DLL 到 common_rangefinder 输出目录
# RelWithDebInfo 复用 Release DLL
set(CSERIALPORT_DLL
    $<$<CONFIG:Debug>:${CSERIALPORT_LIB_DIR_DEBUG}/libcserialport.dll>
    $<$<CONFIG:Release>:${CSERIALPORT_LIB_DIR_RELEASE}/libcserialport.dll>
    $<$<CONFIG:RelWithDebInfo>:${CSERIALPORT_LIB_DIR_RELEASE}/libcserialport.dll>
)
add_custom_command(TARGET ${_tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CSERIALPORT_DLL}
        $<TARGET_FILE_DIR:${_tgt}>
    COMMENT "Copying libcserialport runtime DLL to rangefinder output directory"
)

fz2_setup_target(${_tgt} "common/core/device/rangefinder")

fz2_source_group_tree(rangefinder FILES ${RANGEFINDER_SRCS} ${RANGEFINDER_HEADERS})

if(DEMO_RANGEFINDER_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()

