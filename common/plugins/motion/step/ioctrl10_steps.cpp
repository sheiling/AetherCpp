#include "ioctrl10_steps.h"

#include <chrono>
#include <exception>
#include <thread>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file ioctrl10_steps.cpp
 * @brief IoCtrl10Steps 的实现。
 *
 * 执行流程：校验 doName → 获取 MotionController → WriteIO(doName, bSet)
 *   → 可选 sleep(iMSleep) → 返回 ok。
 *
 * 错误码：
 *   -20  doName 为空
 *   -21  MotionController 不可用
 *   -22  WriteIO 失败
 *   -99  未捕获 std::exception
 *
 * 注册名 "motion.ioctrl10"；实现位于 motion 插件。
 */

namespace step {

using json = nlohmann::json;

void IoCtrl10Steps::reflectFromJson(const json& j)
{
    fromJson(j, "doName", m_doName);
    fromJson(j, "bSet", m_bSet);
    fromJson(j, "iMSleep", m_iMSleep);
    detail::ClampNonNegative(m_iMSleep);
}

scheduler::StepResult IoCtrl10Steps::execute(scheduler::FlowContext& ctx)
{
    (void)ctx;
    const std::string& label = name();

    try {
        // 参数校验
        if (m_doName.empty()) {
            return scheduler::StepResult::fail(
                "IoCtrl10Steps[" + label + "]: doName is empty", -20);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "IoCtrl10Steps[" + label + "]: motion controller not found", -21);
        }

        // 写入单路 DO
        const auto err = motion->WriteIO(m_doName, m_bSet);
        if (err) {
            return scheduler::StepResult::fail(
                "IoCtrl10Steps[" + label + "]: WriteIO failed, do=" + m_doName
                    + ", err=" + err->Message(),
                -22);
        }

        // 可选稳定延时（继电器吸合、气缸动作等）
        if (m_iMSleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_iMSleep));
        }

        return scheduler::StepResult::ok();
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::IoCtrl10Steps, "motion.ioctrl10")
