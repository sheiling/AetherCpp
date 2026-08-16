#pragma once

/**
 * @file ioctrl22_steps.h
 * @brief 双线圈 SET/RST DO 输出并等待 DI 反馈工步（typeName: "motion.ioctrl22"）声明。
 *
 * 适用于双线圈电磁阀等场景：先将 rstName 写 false，再将 setName 写 true，然后轮询 DI。
 * 命名约定「22」：2 路 DO（SET + RST）+ DI 反馈确认。
 *
 * JSON params 示例：
 * @code
 * {
 *   "setName": "DO_VALVE_SET",
 *   "rstName": "DO_VALVE_RST",
 *   "diName": "DI_VALVE_OK",
 *   "bGet": true,
 *   "iMSpace": 10,
 *   "iMTimeout": 5000
 * }
 * @endcode
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 双线圈 DO 脉冲/置位 + DI 轮询等待工步。
class IoCtrl22Steps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.ioctrl22"; }

private:
    /// SET 线圈 DO 名称（JSON: setName）；execute 中在 RST 复位后写 true。
    std::string m_setName;
    /// RST 线圈 DO 名称（JSON: rstName）；execute 中首先写 false。
    std::string m_rstName;
    /// DI 反馈点位名称（JSON: diName）。
    std::string m_diName;
    /// 期望 DI 读到的逻辑电平（JSON: bGet）。
    bool        m_bGet{ false };
    /// DI 轮询间隔毫秒；<=0 时使用默认 20ms（JSON: iMSpace，负值归 0）。
    int         m_iMSpace{ 10 };
    /// 等待 DI 匹配的超时毫秒；<=0 只读一次（JSON: iMTimeout，负值归 0）。
    int         m_iMTimeout{ 0 };
};

} // namespace step
