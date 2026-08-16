# ============================================================
# comm — 通信模块汇总（TCP / RegAccess 等通信能力）
# ============================================================

message(STATUS "  [comm/tcp] 正在配置...")
add_subdirectory(tcp)

message(STATUS "  [comm/regaccess] 正在配置...")
add_subdirectory(regaccess)

message(STATUS "  [comm/http] 正在配置...")
add_subdirectory(http)

message(STATUS "  [comm/serial] 正在配置...")
add_subdirectory(serial)