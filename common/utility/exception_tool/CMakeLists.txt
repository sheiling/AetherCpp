# ============================================================
# exception_tool — 异常诊断工具集
#
# 子模块：
#   - memory_helper  — 基于 CRT 的内存泄漏检测与快照对比
# ============================================================

cmake_minimum_required(VERSION 3.16)

message(STATUS "    [exception_tool] Configuring sub-modules...")

# memory_helper：CRT 内存泄漏检测工具
message(STATUS "      [memory_helper] Configuring...")
add_subdirectory(memory_helper)

message(STATUS "    [exception_tool] Sub-modules configured.")
