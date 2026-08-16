#include "ioctrl12_steps.h"

#include <exception>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file ioctrl12_steps.cpp
 * @brief IoCtrl12Steps 的实现。
 *
 * 执行流程：校验 doName/diName → 获取 MotionController → WriteIO(doName, bSet)
 *   → 轮询 ReadIO(diName) 直至 diValue == bGet 或失败/超时。
 *
 * 轮询策略（与 iosensor / ioctrl22 一致）：
 *   - iMSpace <= 0：使用默认 20ms
 *   - iMTimeout < 0：无限等待
 *   - iMTimeout >= 0：限时等待（0 表示首次不匹配即超时）
 *
 * 错误码：
 *   -20  doName 为空
 *   -21  diName 为空
 *   -22  MotionController 不可用
 *   -23  WriteIO 失败
 *   -24  ReadIO 失败
 *   -26  轮询等待 DI 超时
 *   -99  未捕获 std::exception
 */

namespace step {

using json = nlohmann::json;

void IoCtrl12Steps::reflectFromJson(const json& j)
{
    fromJson(j, "doName", m_doName);
    fromJson(j, "bSet", m_bSet);
    fromJson(j, "diName", m_diName);
    fromJson(j, "bGet", m_bGet);
    fromJson(j, "iMSpace", m_iMSpace);
    fromJson(j, "iMTimeout", m_iMTimeout);
    detail::ClampNonNegative(m_iMSpace);
}

scheduler::StepResult IoCtrl12Steps::execute(scheduler::FlowContext& ctx)
{
    (void)ctx;
    const std::string& label = name();
    constexpr const char* kStepType = "IoCtrl12Steps";

    try {
        // 参数校验
        if (m_doName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl12Steps[" + label + "]: doName is empty", -20);
        }
        if (m_diName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl12Steps[" + label + "]: diName is empty", -21);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "IoCtrl12Steps[" + label + "]: motion controller not found", -22);
        }

        // 先写 DO，再等待 DI 反馈
        scheduler::StepResult result = detail::WriteIoOrFail(
            motion, kStepType, label, m_doName, m_bSet, -23);
        if (!result.isOk()) {
            return result;
        }

        return detail::PollDiUntil(
            motion, kStepType, label, m_diName, m_bGet,
            detail::EffectivePollIntervalMs(m_iMSpace), m_iMTimeout,
            -24, -26);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::IoCtrl12Steps, "motion.ioctrl12")
