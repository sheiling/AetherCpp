#pragma once

/**
 * @file iosensor_steps.h
 * @brief 纯 DI 传感器等待工步（typeName: "motion.iosensor"）声明。
 *
 * 不写入 DO，仅轮询 ReadIO 读取 DI，直到读值等于 bGet 或超时/失败。
 * 常用于到位检测、安全门、真空/夹具到位等输入信号等待。
 *
 * JSON params 示例：
 * @code
 * {
 *   "diName": "DI_PRODUCT_IN_PLACE",
 *   "bGet": true,
 *   "iMSpace": 10,
 *   "iMTimeout": 10000
 * }
 * @endcode
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 纯 DI 读入轮询等待工步；无 DO 输出。
class IoSensorSteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.iosensor"; }

private:
    /// DI 点位名称（JSON: diName）。
    std::string m_diName;
    /// 期望 DI 读到的逻辑电平（JSON: bGet）。
    bool        m_bGet{ false };
    /// DI 轮询间隔毫秒；<=0 时使用默认 20ms（JSON: iMSpace，负值归 0）。
    int         m_iMSpace{ 10 };
    /// 等待 DI 匹配的超时毫秒；< 0 无限等待，>= 0 限时（JSON: iMTimeout，负值保留）。
    int         m_iMTimeout{ 0 };
};

} // namespace step
