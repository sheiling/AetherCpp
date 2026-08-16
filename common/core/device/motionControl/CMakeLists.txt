set(CMAKE_FOLDER common/core/device)

if(NOT HW_MOTION_CONTROL_BUILD)
    message(STATUS "  [device] 跳过 motionControl（HW_DEVICE_MOTION_VIRTUAL 与实体控制卡后端均未开启）")
    return()
endif()

if(HW_DEVICE_MOTION_VIRTUAL)
    message(STATUS "  [device] motionControl 含虚拟运控后端（HW_DEVICE_MOTION_VIRTUAL=ON）")
endif()

set(MOTION_CONTROL_SRCS
    motion_controller.cpp
    motion_control_impl/motion_card_base.cpp
    motion_control_impl/motion_card_dispatch.cpp
    motion_control_impl/motion_types.cpp
    motion_control_impl/motion_card_registry.cpp
    motion_control_impl/motion_sdk_err_map.cpp
    motion_control_impl/drivers/virtual_motion/virtual_motion_card.cpp
)

set(MOTION_CONTROL_DEPS
    ${COMMON_TARGET_NAMESPACE}::errors
    ${COMMON_TARGET_NAMESPACE}::logger
    ${COMMON_TARGET_NAMESPACE}::config
    ${COMMON_TARGET_NAMESPACE}::messagecenter
)

if(HW_DEVICE_MOTION_LEISHINE_DMC)
    add_compile_definitions(BUILD_MOTION_LEISHINE_DMC)
    list(APPEND MOTION_CONTROL_SRCS motion_control_impl/drivers/leadshine_dmc/leadshine_dmc_motion_card.cpp)
    set(MOTION_DMC_LIB_DIR "${PROJECT_SOURCE_DIR}/common/3rdparty/device/motion/Leadshine_DMC/lib/x64")
    link_directories(${MOTION_DMC_LIB_DIR})
    list(APPEND MOTION_CONTROL_DEPS LTDMC)
endif()

if(HW_DEVICE_MOTION_LEISHINE_NMC)
    add_compile_definitions(BUILD_MOTION_LEISHINE_NMC)
    list(APPEND MOTION_CONTROL_SRCS motion_control_impl/drivers/leadshine_nmc/leadshine_nmc_motion_card.cpp)
    set(MOTION_NMC_LIB_DIR "${PROJECT_SOURCE_DIR}/common/3rdparty/device/motion/Leadshine_NMC/lib/x64")
    link_directories(${MOTION_NMC_LIB_DIR})
    list(APPEND MOTION_CONTROL_DEPS LTSMC)
endif()

if(HW_DEVICE_MOTION_SOLID)
    add_compile_definitions(BUILD_MOTION_SOLID)
    list(APPEND MOTION_CONTROL_SRCS motion_control_impl/drivers/solid/solid_motion_card.cpp)
    set(MOTION_SOLID_LIB_DIR "${PROJECT_SOURCE_DIR}/common/3rdparty/device/motion/Solid/lib/x64")
    link_directories(${MOTION_SOLID_LIB_DIR})
    list(APPEND MOTION_CONTROL_DEPS sldm)
endif()

if(HW_DEVICE_MOTION_GAOCHUAN_IO)
    add_compile_definitions(BUILD_MOTION_GAOCHUAN_IO)
    list(APPEND MOTION_CONTROL_SRCS motion_control_impl/drivers/gaochuan_io/gaochuan_io_motion_card.cpp)
    set(MOTION_GAOCHUAN_IO_LIB_DIR "${PROJECT_SOURCE_DIR}/common/3rdparty/device/motion/GaoChuan_IO/lib/x64")
    link_directories(${MOTION_GAOCHUAN_IO_LIB_DIR})
    list(APPEND MOTION_CONTROL_DEPS ecat_io_dll)
endif()

# 编译列表（随 HW_DEVICE_MOTION_* 增减）；IDE 树与编译列表一致，勿用 fz2_collect_sources 全量收集驱动
fz2_expand_tree_files(MOTION_CONTROL_TREE_FILES SOURCES ${MOTION_CONTROL_SRCS})

fz2_add_common_library(motion_control SHARED ${MOTION_CONTROL_TREE_FILES})
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_compile_definitions(${_tgt} PRIVATE MOTION_CONTROL_LIBRARY)
target_link_libraries(${_tgt} PRIVATE ${MOTION_CONTROL_DEPS})
target_include_directories(${_tgt}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common/3rdparty
        ${PROJECT_SOURCE_DIR}/common/3rdparty/spdlog/include
        ${PROJECT_SOURCE_DIR}/common/core/config/include
        ${PROJECT_SOURCE_DIR}/common/utility/logger/include
)
fz2_setup_target(${_tgt} "common/core/device/motionControl")

fz2_source_group_tree(motion_control FILES ${MOTION_CONTROL_TREE_FILES})

if(WIN32)
    if(HW_DEVICE_MOTION_LEISHINE_DMC)
        add_custom_command(TARGET ${_tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MOTION_DMC_LIB_DIR}/LTDMC.dll"
                "$<TARGET_FILE_DIR:${_tgt}>"
            COMMENT "Copy Leadshine DMC DLL -> $<TARGET_FILE_DIR:${_tgt}>"
        )
    endif()

    if(HW_DEVICE_MOTION_LEISHINE_NMC)
        add_custom_command(TARGET ${_tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MOTION_NMC_LIB_DIR}/LTSMC.dll"
                "$<TARGET_FILE_DIR:${_tgt}>"
            COMMENT "Copy Leadshine NMC DLL -> $<TARGET_FILE_DIR:${_tgt}>"
        )
    endif()

    if(HW_DEVICE_MOTION_SOLID)
        add_custom_command(TARGET ${_tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MOTION_SOLID_LIB_DIR}/sldm.dll"
                "$<TARGET_FILE_DIR:${_tgt}>"
            COMMENT "Copy Solid DLL -> $<TARGET_FILE_DIR:${_tgt}>"
        )
    endif()

    if(HW_DEVICE_MOTION_GAOCHUAN_IO)
        add_custom_command(TARGET ${_tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MOTION_GAOCHUAN_IO_LIB_DIR}/ecat_io_dll.dll"
                "$<TARGET_FILE_DIR:${_tgt}>"
            COMMENT "Copy GaoChuan IO DLL -> $<TARGET_FILE_DIR:${_tgt}>"
        )
    endif()
endif()

if(DEMO_DEVICE_MOTION_EXAMPLE_BUILD)
    add_subdirectory(examples)
endif()
