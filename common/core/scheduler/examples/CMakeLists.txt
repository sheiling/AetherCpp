# ============================================================
# scheduler/examples — 调度引擎示例可执行文件
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── basic_example：基础功能演示 ───────────────────────────────
add_executable(scheduler_basic_example
    basic_example.cpp
)

target_compile_features(scheduler_basic_example PUBLIC cxx_std_17)

target_include_directories(scheduler_basic_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/3rdparty/nlohmann
)

target_link_libraries(scheduler_basic_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::scheduler
        ${COMMON_TARGET_NAMESPACE}::logger
)

fz2_setup_target(scheduler_basic_example "common/core/scheduler/examples")
fz2_target_msvc_options(scheduler_basic_example)

# ── station_example：多工位协同演示 ──────────────────────────
add_executable(scheduler_station_example
    station_example.cpp
)

target_compile_features(scheduler_station_example PUBLIC cxx_std_17)

target_include_directories(scheduler_station_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/3rdparty/nlohmann
)

target_link_libraries(scheduler_station_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::scheduler
        ${COMMON_TARGET_NAMESPACE}::logger
)

fz2_setup_target(scheduler_station_example "common/core/scheduler/examples")
fz2_target_msvc_options(scheduler_station_example)

# ── production_line_example：持续运行/暂停/异常恢复/JSON加载演示 ─
add_executable(scheduler_production_example
    production_line_example.cpp
)

target_compile_features(scheduler_production_example PUBLIC cxx_std_17)

target_include_directories(scheduler_production_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/3rdparty/nlohmann
)

target_compile_definitions(scheduler_production_example
    PRIVATE
        # 示例 JSON 文件目录（供 LineLoader 加载 line_config.json）
        EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(scheduler_production_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::scheduler
        ${COMMON_TARGET_NAMESPACE}::logger
)

fz2_setup_target(scheduler_production_example "common/core/scheduler/examples")
fz2_target_msvc_options(scheduler_production_example)

# ── step_flow_controller_example：步骤名 → 流程文件映射 + StepFlowController ─
add_executable(scheduler_step_flow_example
    step_flow_controller_example.cpp
)

target_compile_features(scheduler_step_flow_example PUBLIC cxx_std_17)

target_include_directories(scheduler_step_flow_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/3rdparty/nlohmann
)

target_compile_definitions(scheduler_step_flow_example
    PRIVATE
        SCHEDULER_STEP_FLOW_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(scheduler_step_flow_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::scheduler
        ${COMMON_TARGET_NAMESPACE}::logger
)

fz2_setup_target(scheduler_step_flow_example "common/core/scheduler/examples")
fz2_target_msvc_options(scheduler_step_flow_example)

message(STATUS "    [scheduler/examples] Configured.")
