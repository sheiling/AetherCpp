#include "ioctrl22_steps.h"

#include <exception>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file ioctrl22_steps.cpp
 * @brief IoCtrl22Steps 的实现。
 *
 * 执行流程：校验 setName/rstName/diName → 获取 MotionController
 *   → WriteIO(rstName, false) → WriteIO(setName, true)
 *   → 轮询 ReadIO(diName) 直至 diValue == bGet 或失败/超时。
 *
 * 双线圈顺序说明：先 RST 释放/复位，再 SET 置位，避免两路同时为 true 的竞态。
 *
 * 错误码：
 *   -20  setName 为空
 *   -21  rstName 为空
 *   -22  diName 为空
 *   -23  MotionController 不可用
 *   -24  WriteIO(rstName) 失败
 *   -25  WriteIO(setName) 失败
 *   -26  ReadIO 失败
 *   -28  轮询等待 DI 超时
 *   -99  未捕获 std::exception
 */

namespace step {

using json = nlohmann::json;

void IoCtrl22Steps::reflectFromJson(const json& j)
{
    fromJson(j, "setName", m_setName);
    fromJson(j, "rstName", m_rstName);
    fromJson(j, "diName", m_diName);
    fromJson(j, "bGet", m_bGet);
    fromJson(j, "iMSpace", m_iMSpace);
    fromJson(j, "iMTimeout", m_iMTimeout);
    detail::ClampNonNegative(m_iMSpace);
}

scheduler::StepResult IoCtrl22Steps::execute(scheduler::FlowContext& ctx)
{
    (void)ctx;
    const std::string& label = name();
    constexpr const char* kStepType = "IoCtrl22Steps";

    try {
        // 参数校验
        if (m_setName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl22Steps[" + label + "]: setName is empty", -20);
        }
        if (m_rstName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl22Steps[" + label + "]: rstName is empty", -21);
        }
        if (m_diName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl22Steps[" + label + "]: diName is empty", -22);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "IoCtrl22Steps[" + label + "]: motion controller not found", -23);
        }

        // 双线圈：先复位 RST，再置位 SET，避免竞态
        scheduler::StepResult result = detail::WriteIoOrFail(
            motion, kStepType, label, m_rstName, false, -24);
        if (!result.isOk()) {
            return result;
        }
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_setName, true, -25);
        if (!result.isOk()) {
            return result;
        }

        // 等待 DI 反馈到位
        return detail::PollDiUntil(
            motion, kStepType, label, m_diName, m_bGet,
            detail::EffectivePollIntervalMs(m_iMSpace), m_iMTimeout,
            -26, -28);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::IoCtrl22Steps, "motion.ioctrl22")
