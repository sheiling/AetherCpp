#pragma once

/**
 * @file ioctrl12_steps.h
 * @brief 单路 DO 输出并等待 DI 反馈工步（typeName: "motion.ioctrl12"）声明。
 *
 * 先 WriteIO 写入一路 DO，再轮询 ReadIO 读取 DI，直到读值等于 bGet 或超时/失败。
 * 命名约定「12」：1 路输出 + DI 反馈确认。
 *
 * JSON params 示例：
 * @code
 * {
 *   "doName": "DO_VACUUM",
 *   "bSet": true,
 *   "diName": "DI_VACUUM_OK",
 *   "bGet": true,
 *   "iMSpace": 10,
 *   "iMTimeout": 3000
 * }
 * @endcode
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 单路 DO 写 + DI 轮询等待工步。
class IoCtrl12Steps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.ioctrl12"; }

private:
    /// DO 点位名称（JSON: doName）。
    std::string m_doName;
    /// 写入 DO 的目标逻辑电平（JSON: bSet）。
    bool        m_bSet{ false };
    /// DI 点位名称（JSON: diName）。
    std::string m_diName;
    /// 期望 DI 读到的逻辑电平（JSON: bGet；已含 highActive 映射后的有效值）。
    bool        m_bGet{ false };
    /// DI 轮询间隔毫秒；<=0 时 execute 内使用默认 20ms（JSON: iMSpace，负值归 0）。
    int         m_iMSpace{ 10 };
    /// 等待 DI 匹配的超时毫秒；<=0 表示只读一次，不匹配立即失败（JSON: iMTimeout，负值归 0）。
    int         m_iMTimeout{ 0 };
};

} // namespace step
