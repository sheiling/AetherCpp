#include "upstream_dock_steps.h"

#include <exception>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file upstream_dock_steps.cpp
 * @brief UpstreamDockSteps 的实现。
 *
 * 执行流程：
 *   1. 读取 KEY_UPSTREAM_DOCK_STEP_CONFIG.enabled；关闭时直接返回 ok
 *   2. 校验 allowOutputName / feedInductionName → 获取 MotionController
 *   3. WriteIO(允许放板, true) → 轮询 ReadIO(进料感应) 直至 true
 *   4. WriteIO(允许放板, false) → 返回 ok
 *
 * 错误码：
 *   -20  allowOutputName 为空
 *   -21  feedInductionName 为空
 *   -22  MotionController 不可用
 *   -23  WriteIO(允许放板=true) 失败
 *   -24  ReadIO(进料感应) 失败
 *   -25  WriteIO(允许放板=false) 失败
 *   -26  等待进料感应超时（iMTimeout > 0）
 *   -99  未捕获 std::exception
 *
 * 注册名 "motion.upstream_dock"；实现位于 motion 插件。
 */

namespace step {

using json = nlohmann::json;

void UpstreamDockSteps::reflectFromJson(const json& j)
{
    fromJson(j, "allowOutputName", m_allowOutputName);
    fromJson(j, "feedInductionName", m_feedInductionName);
    fromJson(j, "iMSpace", m_iMSpace);
    fromJson(j, "iMTimeout", m_iMTimeout);
    detail::ClampNonNegative(m_iMSpace);
}

scheduler::StepResult UpstreamDockSteps::execute(scheduler::FlowContext& ctx)
{
    const std::string& label = name();
    constexpr const char* kStepType = "UpstreamDockSteps";

    try {
        // 上游对接关闭：跳过 IO 操作
        if (!detail::IsRuntimeStepEnabled(ctx, KEY_UPSTREAM_DOCK_STEP_CONFIG)) {
            return scheduler::StepResult::ok();
        }

        // 参数校验
        if (m_allowOutputName.empty()) {
            return scheduler::StepResult::fail(
                "UpstreamDockSteps[" + label + "]: allowOutputName is empty", -20);
        }
        if (m_feedInductionName.empty()) {
            return scheduler::StepResult::fail(
                "UpstreamDockSteps[" + label + "]: feedInductionName is empty", -21);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "UpstreamDockSteps[" + label + "]: motion controller not found", -22);
        }

        // 允许上料机放板
        scheduler::StepResult result = detail::WriteIoOrFail(
            motion, kStepType, label, m_allowOutputName, true, -23);
        if (!result.isOk()) {
            return result;
        }

        // 等待进料感应（iMTimeout < 0 无限等待，>= 0 限时）
        result = detail::PollDiUntil(
            motion, kStepType, label, m_feedInductionName, true,
            m_iMSpace, m_iMTimeout, -24, -26);
        if (!result.isOk()) {
            return result;
        }

        // 禁止上料机继续放板
        return detail::WriteIoOrFail(
            motion, kStepType, label, m_allowOutputName, false, -25);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::UpstreamDockSteps, "motion.upstream_dock")
