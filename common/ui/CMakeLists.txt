# ============================================================
# common/ui — UI 框架与控件库汇总
# ============================================================

message(STATUS "[common/ui] 正在配置...")

add_subdirectory(uibase)
add_subdirectory(mvvm)
add_subdirectory(component)

message(STATUS "[common/ui] 配置完成")
