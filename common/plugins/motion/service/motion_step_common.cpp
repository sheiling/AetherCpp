#include "motion_step_common.h"

#include <chrono>
#include <thread>

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/utility/json_tool/reflector.h>

/**
 * @file motion_step_common.cpp
 * @brief motion 工步公共辅助实现（位于 service/）。
 *
 * 供 iosensor / ioctrl12 / ioctrl22 / upstream_dock / downstream_dock / scan_code 等工步复用：
 *   - 轮询间隔归一化、负值钳制
 *   - FlowContext GLOBAL 运行时开关读取
 *   - DO 写入失败封装
 *   - DI 轮询等待（timeout_ms < 0 无限，>= 0 限时）
 */

namespace step {
namespace detail {

namespace {

using json = nlohmann::json;

} // namespace

int EffectivePollIntervalMs(int poll_space_ms, int default_ms)
{
    // 工步 JSON 未配或配非正数时，回退到各工步约定的默认间隔
    return (poll_space_ms > 0) ? poll_space_ms : default_ms;
}

void ClampNonNegative(int& value)
{
    // reflectFromJson 后统一钳制，避免 sleep 等参数传入负值
    if (value < 0) {
        value = 0;
    }
}

bool IsRuntimeStepEnabled(const scheduler::FlowContext& ctx,
                          const char* config_key,
                          bool default_enabled)
{
    // 从 GLOBAL 变量读取 { "enabled": bool }；配置缺失或非 object 时用 default_enabled
    bool enabled = default_enabled;
    const auto cfg_opt = ctx.tryGet<json>(config_key);
    if (cfg_opt && cfg_opt->is_object()) {
        fromJson(*cfg_opt, "enabled", enabled);
    }
    return enabled;
}

scheduler::StepResult WriteIoOrFail(
    const std::shared_ptr<common::device::MotionController>& motion,
    const std::string& step_type,
    const std::string& step_label,
    const std::string& io_name,
    bool value,
    int err_code)
{
    const auto write_err = motion->WriteIO(io_name, value);
    if (write_err) {
        return scheduler::StepResult::fail(
            step_type + "[" + step_label + "]: WriteIO failed, io=" + io_name
                + ", value=" + (value ? "true" : "false")
                + ", err=" + write_err->Message(),
            err_code);
    }
    return scheduler::StepResult::ok();
}

scheduler::StepResult PollDiUntil(
    const std::shared_ptr<common::device::MotionController>& motion,
    const std::string& step_type,
    const std::string& step_label,
    const std::string& di_name,
    bool expect_value,
    int poll_ms,
    int timeout_ms,
    int read_err_code,
    int timeout_err_code)
{
    const bool infinite_wait = (timeout_ms < 0);
    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(timeout_ms);

    while (true) {
        bool di_value = false;
        const auto read_err = motion->ReadIO(di_name, di_value);
        if (read_err) {
            return scheduler::StepResult::fail(
                step_type + "[" + step_label + "]: ReadIO failed, di=" + di_name
                    + ", err=" + read_err->Message(),
                read_err_code);
        }
        if (di_value == expect_value) {
            return scheduler::StepResult::ok();
        }

        // timeout_ms >= 0：超过 deadline 则超时失败
        if (!infinite_wait && std::chrono::steady_clock::now() >= deadline) {
            return scheduler::StepResult::fail(
                step_type + "[" + step_label + "]: wait DI timeout, di=" + di_name
                    + ", expect=" + (expect_value ? "true" : "false"),
                timeout_err_code);
        }
        // timeout_ms < 0：sleep 后继续轮询
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_ms));
    }
}

} // namespace detail
} // namespace step
