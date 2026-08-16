# ============================================================
# common/core — 核心模块汇总
# ============================================================

message(STATUS "[common/core] 正在配置...")

add_subdirectory(messagecenter)
add_subdirectory(extensionsystem)
add_subdirectory(container)
add_subdirectory(config)
add_subdirectory(comm)

if(CORE_SCHEDULER_BUILD)
    add_subdirectory(scheduler)
else()
    message(STATUS "  [common/core] 跳过 scheduler（CORE_SCHEDULER_BUILD=OFF）")
endif()

add_subdirectory(device)

message(STATUS "[common/core] 配置完成")
