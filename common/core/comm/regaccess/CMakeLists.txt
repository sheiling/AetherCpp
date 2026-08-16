cmake_minimum_required(VERSION 3.16)

# ============================================================
# regaccess — 寄存器访问统一封装（Keyence / Melsec / Modbus）
# ============================================================

message(STATUS "    [regaccess] Configuring keyence/melsec/modbus...")

add_subdirectory(keyence)
add_subdirectory(melsec)
add_subdirectory(modbus)

# ── 构建 example（可选，通过 DEMO_COMM_REGACCESS_EXAMPLE_BUILD 选项控制）───────────────
if(DEMO_COMM_REGACCESS_EXAMPLE_BUILD)
    message(STATUS "    [regaccess/examples] Configuring...")
    add_subdirectory(examples)
endif()

message(STATUS "    [regaccess] Configured")