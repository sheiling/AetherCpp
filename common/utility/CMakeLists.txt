# ============================================================
# common/utility — 工具类库汇总
# ============================================================

message(STATUS "[common/utility] 正在配置...")

add_subdirectory(logger)
if(COMMON_ORM_BUILD)
    add_subdirectory(orm)
else()
    message(STATUS "[common/utility] 跳过 orm（COMMON_ORM_BUILD=OFF）")
endif()
add_subdirectory(errors)
add_subdirectory(exception_tool)

message(STATUS "[common/utility] 配置完成")
