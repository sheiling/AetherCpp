# ============================================================
# common — 公共库汇总
#
# 本层 CMake 模块（FZ2Helpers / CommonOptions）仅服务于 common 及其子目录；
# app、plugins 在各自 CMakeLists.txt 中 include 对应选项。
# ============================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FZ2Helpers)
include(CommonOptions)

message(STATUS "[common] 正在配置公共库...")

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(utility)
add_subdirectory(base)
add_subdirectory(plugins)

message(STATUS "[common] 公共库配置完成")
