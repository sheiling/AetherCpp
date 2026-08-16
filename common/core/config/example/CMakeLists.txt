# ============================================================
# config_example — config 模块使用示例可执行程序
# ============================================================

# ── 创建可执行文件 ─────────────────────────────────────────────────────────────
add_executable(config_example
    main.cpp
)

# ── 链接 config 库 ─────────────────────────────────────────────────────────────
# ${COMMON_TARGET_NAMESPACE}::config 的 PUBLIC include 已包含：
#   - include/（Config.h / config_global.h）
#   - toml11 include 目录（因 Config.h 内含 toml.hpp，故传递给下游）
# 因此无需额外设置 target_include_directories
target_link_libraries(config_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::config
)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
target_compile_features(config_example PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(config_example PRIVATE /utf-8 /W3)
else()
    target_compile_options(config_example PRIVATE -Wall -Wextra)
endif()

# ── 输出目录（与其他可执行文件保持一致） ─────────────────────────────────────
fz2_setup_target(config_example "common/core/config/example")

# ── 拷贝 res/ 目录到可执行文件输出目录 ────────────────────────────────────────
# 确保程序运行时能找到 res/app.toml
add_custom_command(TARGET config_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:config_example>/res"
    COMMENT "  Copying config_example resources to output directory..."
)

message(STATUS "      [config_example] Configured")
