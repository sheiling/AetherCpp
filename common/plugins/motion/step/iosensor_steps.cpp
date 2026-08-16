#include "iosensor_steps.h"

#include <exception>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file iosensor_steps.cpp
 * @brief IoSensorSteps 的实现。
 *
 * 执行流程：校验 diName → 获取 MotionController
 *   → 轮询 ReadIO(diName) 直至 diValue == bGet 或失败/超时。
 * 不写入任何 DO，仅做 DI 等待。
 *
 * 轮询策略：
 *   - iMSpace <= 0：使用默认 20ms
 *   - iMTimeout < 0：无限等待
 *   - iMTimeout >= 0：限时等待（0 表示首次不匹配即超时）
 *
 * 错误码：
 *   -20  diName 为空
 *   -21  MotionController 不可用
 *   -22  ReadIO 失败
 *   -24  轮询等待 DI 超时
 *   -99  未捕获 std::exception
 */

namespace step {

using json = nlohmann::json;

void IoSensorSteps::reflectFromJson(const json& j)
{
    fromJson(j, "diName", m_diName);
    fromJson(j, "bGet", m_bGet);
    fromJson(j, "iMSpace", m_iMSpace);
    fromJson(j, "iMTimeout", m_iMTimeout);
    detail::ClampNonNegative(m_iMSpace);
}

scheduler::StepResult IoSensorSteps::execute(scheduler::FlowContext& ctx)
{
    (void)ctx;
    const std::string& label = name();
    constexpr const char* kStepType = "IoSensorSteps";

    try {
        // 参数校验
        if (m_diName.empty()) {
            return scheduler::StepResult::fail(
                "IoSensorSteps[" + label + "]: diName is empty", -20);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "IoSensorSteps[" + label + "]: motion controller not found", -21);
        }

        // 仅轮询 DI，不写 DO
        return detail::PollDiUntil(
            motion, kStepType, label, m_diName, m_bGet,
            detail::EffectivePollIntervalMs(m_iMSpace), m_iMTimeout,
            -22, -24);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::IoSensorSteps, "motion.iosensor")
