#pragma once

/**
 * @file ioctrl10_steps.h
 * @brief 单路 DO 输出工步（typeName: "motion.ioctrl10"）声明。
 *
 * 通过 MotionController 写入一路数字量输出（DO），可选执行固定延时后成功返回。
 * 命名约定「10」：1 路输出 + 0 路 DI 反馈确认。
 *
 * JSON params 示例：
 * @code
 * {
 *   "doName": "DO_CLAMP",
 *   "bSet": true,
 *   "iMSleep": 100
 * }
 * @endcode
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 单路 DO 写输出工步；不读取 DI，仅 WriteIO + 可选 sleep。
class IoCtrl10Steps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.ioctrl10"; }

private:
    /// DO 点位名称（JSON: doName；对应 device_config.json DeviceIo 中的键名）。
    std::string m_doName;
    /// 写入 DO 的目标逻辑电平（JSON: bSet）。
    bool        m_bSet{ false };
    /// WriteIO 成功后额外休眠毫秒数；0 表示不休眠（JSON: iMSleep，负值归 0）。
    int         m_iMSleep{ 0 };
};

} // namespace step
