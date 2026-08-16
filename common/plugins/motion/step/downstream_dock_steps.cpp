#include "downstream_dock_steps.h"

#include <chrono>
#include <exception>
#include <thread>

#include "motion_control_helper.h"
#include "motion_step_common.h"
#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

/**
 * @file downstream_dock_steps.cpp
 * @brief DownstreamDockSteps 的实现。
 *
 * 执行流程：
 *   1. 读取 KEY_DOWNSTREAM_DOCK_STEP_CONFIG.enabled；关闭时直接返回 ok
 *   2. 从 KEY_OVERALL_DETECT_RESULT_OUT 读取综合判定，写入 OK/NG 输出
 *   3. 本机有板 true → 等待下游响应 → 设方向并启动皮带
 *   4. 等待出料感应 false → 延时 → 停皮带 → 本机无板 → 清除 OK/NG
 *
 * 错误码：
 *   -20  okOutputName 为空
 *   -21  ngOutputName 为空
 *   -22  boardPresentOutputName 为空
 *   -23  downstreamResponseInputName 为空
 *   -24  beltDirectionOutputName 为空
 *   -25  beltStartOutputName 为空
 *   -26  dischargeInductionInputName 为空
 *   -27  MotionController 不可用
 *   -28  WriteIO 失败
 *   -29  ReadIO 失败
 *   -30  等待下游响应超时
 *   -31  等待出料完成超时
 *   -99  未捕获 std::exception
 *
 * 注册名 "motion.downstream_dock"；实现位于 motion 插件。
 */

namespace step {

using json = nlohmann::json;

namespace {

common::algorithm::DetectResultStatus ReadOverallDetectResult(
    const scheduler::FlowContext& ctx)
{
    int result = static_cast<int>(common::algorithm::DetectResultStatus::Ok);
    const auto result_opt = ctx.tryGet<int>(KEY_OVERALL_DETECT_RESULT_OUT);
    if (result_opt) {
        result = *result_opt;
    }
    return static_cast<common::algorithm::DetectResultStatus>(result);
}

} // namespace

void DownstreamDockSteps::reflectFromJson(const json& j)
{
    fromJson(j, "okOutputName", m_okOutputName);
    fromJson(j, "ngOutputName", m_ngOutputName);
    fromJson(j, "boardPresentOutputName", m_boardPresentOutputName);
    fromJson(j, "downstreamResponseInputName", m_downstreamResponseInputName);
    fromJson(j, "beltDirectionOutputName", m_beltDirectionOutputName);
    fromJson(j, "beltDirection", m_beltDirection);
    fromJson(j, "beltStartOutputName", m_beltStartOutputName);
    fromJson(j, "dischargeInductionInputName", m_dischargeInductionInputName);
    fromJson(j, "stopBeltDelayMs", m_stopBeltDelayMs);
    fromJson(j, "iMSpace", m_iMSpace);
    fromJson(j, "iMTimeout", m_iMTimeout);

    detail::ClampNonNegative(m_stopBeltDelayMs);
    detail::ClampNonNegative(m_iMSpace);
}

scheduler::StepResult DownstreamDockSteps::execute(scheduler::FlowContext& ctx)
{
    const std::string& label = name();
    constexpr const char* kStepType = "DownstreamDockSteps";

    try {
        // 下游对接关闭：跳过 IO 操作
        if (!detail::IsRuntimeStepEnabled(ctx, KEY_DOWNSTREAM_DOCK_STEP_CONFIG)) {
            return scheduler::StepResult::ok();
        }

        // 必填 IO 参数校验
        if (m_okOutputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: okOutputName is empty", -20);
        }
        if (m_ngOutputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: ngOutputName is empty", -21);
        }
        if (m_boardPresentOutputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: boardPresentOutputName is empty", -22);
        }
        if (m_downstreamResponseInputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: downstreamResponseInputName is empty", -23);
        }
        if (m_beltDirectionOutputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: beltDirectionOutputName is empty", -24);
        }
        if (m_beltStartOutputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: beltStartOutputName is empty", -25);
        }
        if (m_dischargeInductionInputName.empty()) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: dischargeInductionInputName is empty", -26);
        }

        auto motion = motion::motionControllerOrNull();
        if (!motion) {
            return scheduler::StepResult::fail(
                "DownstreamDockSteps[" + label + "]: motion controller not found", -27);
        }

        // 根据综合判定写 OK/NG 信号
        const auto overall = ReadOverallDetectResult(ctx);
        const bool is_ok = (overall == common::algorithm::DetectResultStatus::Ok);

        scheduler::StepResult result = detail::WriteIoOrFail(
            motion, kStepType, label, m_okOutputName, is_ok, -28);
        if (!result.isOk()) {
            return result;
        }
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_ngOutputName, !is_ok, -28);
        if (!result.isOk()) {
            return result;
        }

        // 通知下游本机有板
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_boardPresentOutputName, true, -28);
        if (!result.isOk()) {
            return result;
        }

        // 等待下游收料就绪
        result = detail::PollDiUntil(
            motion, kStepType, label, m_downstreamResponseInputName, true,
            m_iMSpace, m_iMTimeout, -29, -30);
        if (!result.isOk()) {
            return result;
        }

        // 先设方向，再启动传送带
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_beltDirectionOutputName, m_beltDirection, -28);
        if (!result.isOk()) {
            return result;
        }
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_beltStartOutputName, true, -28);
        if (!result.isOk()) {
            return result;
        }

        // 等待出料感应消失（出料完成）
        result = detail::PollDiUntil(
            motion, kStepType, label, m_dischargeInductionInputName, false,
            m_iMSpace, m_iMTimeout, -29, -31);
        if (!result.isOk()) {
            return result;
        }

        // 停止皮带前延时
        std::this_thread::sleep_for(std::chrono::milliseconds(m_stopBeltDelayMs));

        // 停止传送带
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_beltStartOutputName, false, -28);
        if (!result.isOk()) {
            return result;
        }

        // 通知下游本机无板
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_boardPresentOutputName, false, -28);
        if (!result.isOk()) {
            return result;
        }

        // 清除 OK/NG 信号
        result = detail::WriteIoOrFail(
            motion, kStepType, label, m_okOutputName, false, -28);
        if (!result.isOk()) {
            return result;
        }
        return detail::WriteIoOrFail(
            motion, kStepType, label, m_ngOutputName, false, -28);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::DownstreamDockSteps, "motion.downstream_dock")
