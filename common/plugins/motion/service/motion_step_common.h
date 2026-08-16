#pragma once

/**
 * @file motion_step_common.h
 * @brief motion 工步公共辅助（DI 轮询、DO 写入、运行时开关读取）。
 *
 * 与 motion_control_helper 同属 service 层，供 step/ 下各 IO 工步复用。
 */

#include <memory>
#include <string>

#include <common/core/device/motionControl/motion_controller.h>
#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

namespace step {
namespace detail {

constexpr int kDefaultPollIntervalMs = 20;

/** DI 轮询无限等待：timeout_ms 传此值（或任意负数）。 */
constexpr int kInfiniteDiWaitMs = -1;

/** iMSpace <= 0 时返回 default_ms，否则返回 iMSpace。 */
int EffectivePollIntervalMs(int poll_space_ms,
                            int default_ms = kDefaultPollIntervalMs);

/** 负值钳制为 0（勿用于 iMTimeout，否则会破坏无限等待语义）。 */
void ClampNonNegative(int& value);

/**
 * 从 FlowContext GLOBAL 配置 JSON 读取 enabled 字段。
 * @param config_key 如 KEY_SCAN_STEP_CONFIG
 * @param default_enabled 配置缺失时的默认值
 */
bool IsRuntimeStepEnabled(const scheduler::FlowContext& ctx,
                          const char* config_key,
                          bool default_enabled = true);

/** WriteIO 失败时封装为 StepResult::fail。 */
scheduler::StepResult WriteIoOrFail(
    const std::shared_ptr<common::device::MotionController>& motion,
    const std::string& step_type,
    const std::string& step_label,
    const std::string& io_name,
    bool value,
    int err_code);

/**
 * 轮询 ReadIO 直至 di 读值等于 expect_value，或失败/超时。
 * @param timeout_ms < 0 无限等待；>= 0 限时等待（毫秒，0 表示首次不匹配即超时）
 */
scheduler::StepResult PollDiUntil(
    const std::shared_ptr<common::device::MotionController>& motion,
    const std::string& step_type,
    const std::string& step_label,
    const std::string& di_name,
    bool expect_value,
    int poll_ms,
    int timeout_ms,
    int read_err_code,
    int timeout_err_code);

} // namespace detail
} // namespace step
