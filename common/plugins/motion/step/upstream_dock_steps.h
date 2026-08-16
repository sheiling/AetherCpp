#pragma once

/**
 * @file upstream_dock_steps.h
 * @brief 上游对接进料工步（typeName: "motion.upstream_dock"）声明。
 *
 * 执行流程（上游对接开启时）：
 *   1. WriteIO(允许放板输出信号, true) — 允许上料机放板
 *   2. 轮询 ReadIO(进料感应输入信号) 直至为 true — 灯板进料感应
 *   3. WriteIO(允许放板输出信号, false) — 不允许上料机放板
 *
 * 运行时开关（GLOBAL）：
 *   - KEY_UPSTREAM_DOCK_STEP_CONFIG.enabled ← 系统配置「上游对接」
 *   - 由 HomeView::slot_initialize → setUpstreamDockStepConfig 写入
 *   - enabled=false 时不访问硬件，直接返回 ok
 *
 * JSON params 示例：
 * @code
 * {
 *   "allowOutputName": "DO_UPSTREAM_ALLOW",
 *   "feedInductionName": "DI_FEED_INDUCTION",
 *   "iMSpace": 20,
 *   "iMTimeout": -1
 * }
 * @endcode
 *
 * 轮询策略：
 *   - iMSpace：轮询间隔毫秒（默认 20）
 *   - iMTimeout < 0：无限等待进料感应
 *   - iMTimeout >= 0：限时等待，超时失败
 */

#include <string>

#include "motion_step_common.h"

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 上游对接进料工步：允许放板 → 等待进料感应 → 禁止放板。
class UpstreamDockSteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.upstream_dock"; }

private:
    /// 允许上料机放板的 DO 点位名称（JSON: allowOutputName）。
    std::string m_allowOutputName;
    /// 进料感应 DI 点位名称（JSON: feedInductionName）。
    std::string m_feedInductionName;
    /// DI 轮询间隔毫秒（JSON: iMSpace，默认 20；负值归 0）。
    int         m_iMSpace{ 20 };
    /// 等待进料感应超时毫秒；< 0 无限等待（JSON: iMTimeout，默认 -1）。
    int         m_iMTimeout{ detail::kInfiniteDiWaitMs };
};

} // namespace step
