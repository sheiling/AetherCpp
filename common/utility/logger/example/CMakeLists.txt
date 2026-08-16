# ============================================================
# logger_example — Logger 模块使用示例可执行程序
# ============================================================

# ── 创建可执行文件 ─────────────────────────────────────────────────────────────
add_executable(logger_example
    main.cpp
)

# ── 链接 logger 库 ─────────────────────────────────────────────────────────────
# ${COMMON_TARGET_NAMESPACE}::logger 的 PUBLIC include 已包含：
#   - include/（Logger.h / logger_global.h）
#   - spdlog include 目录（因 Logger.h 内含 spdlog/spdlog.h，故传递给下游）
# 因此无需额外设置 target_include_directories
target_link_libraries(logger_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::logger
)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
target_compile_features(logger_example PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(logger_example PRIVATE /utf-8 /W3)
endif()

# ── 输出目录（与其他可执行文件保持一致） ─────────────────────────────────────
fz2_setup_target(logger_example "common/utility/logger/example")

message(STATUS "      [logger_example] Configured")
