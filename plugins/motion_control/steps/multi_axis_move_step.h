#pragma once

/**
 * @file multi_axis_move_step.h
 * @brief 多轴点位运动工步（typeName: motion.multi_axis_move）
 *
 * 本目录约定：一种工步 = 一对文件 xxx_step.h / xxx_step.cpp。
 *
 * 参数来源（execute 内合并）：
 * 1. JSON params — 流程配置静态默认值
 * 2. FlowContext — submit 前注入；ctxKey 或默认键 kCtxMultiAxisMoveParams
 *
 * JSON params 示例：
 * @code
 * {
 *   "ctxKey": "motion.multi_axis_move.params",
 *   "axisNames": ["X轴", "Y轴"],
 *   "positions": [0.0, 0.0],
 *   "absolute": true,
 *   "speed": 0.0,
 *   "parallel": true
 * }
 * @endcode
 */

#include <optional>
#include <string>
#include <vector>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/context/flow_context.h>
#include <scheduler/step/i_step.h>

namespace motion_control_plugin {
class IMotionControlService;
} // namespace motion_control_plugin

namespace motion_control_plugin::step {

/** FlowContext 中多轴运动参数的默认键名 */
inline constexpr const char *kCtxMultiAxisMoveParams = "motion.multi_axis_move.params";

/** 多轴运动完整参数（JSON 或代码整包指定） */
struct MultiAxisMoveParams {
    std::vector<std::string> axisNames;
    std::vector<double>      positions;
    bool                     absolute{ true };
    double                   speed{ 0.0 };
    bool                     parallel{ true };

    bool isValid() const
    {
        return !axisNames.empty() && axisNames.size() == positions.size();
    }
};

/** 部分覆盖：仅非空/有值字段覆盖 JSON 默认值 */
struct MultiAxisMoveOverride {
    std::vector<std::string> axisNames;
    std::vector<double>      positions;
    std::optional<bool>      absolute;
    std::optional<double>    speed;
    std::optional<bool>      parallel;
};

inline void setMultiAxisMoveParams(scheduler::FlowContext &ctx,
                                   const MultiAxisMoveParams &params,
                                   const std::string &key = kCtxMultiAxisMoveParams,
                                   scheduler::VariableScope scope = scheduler::VariableScope::FLOW)
{
    ctx.set(key, params, scope);
}

inline void setMultiAxisMoveOverride(scheduler::FlowContext &ctx,
                                     const MultiAxisMoveOverride &overrideParams,
                                     const std::string &key = kCtxMultiAxisMoveParams,
                                     scheduler::VariableScope scope = scheduler::VariableScope::FLOW)
{
    ctx.set(key, overrideParams, scope);
}

inline void setMultiAxisMoveOverrideJson(scheduler::FlowContext &ctx,
                                         const nlohmann::json &partial,
                                         const std::string &key = kCtxMultiAxisMoveParams,
                                         scheduler::VariableScope scope = scheduler::VariableScope::FLOW)
{
    ctx.set(key, partial, scope);
}

/** 不经过 Scheduler，直接执行多轴运动（params 须已合并完成） */
scheduler::StepResult runMultiAxisMove(IMotionControlService *motion,
                                       const MultiAxisMoveParams &params,
                                       const std::string &stepLabel = {});

scheduler::StepResult runMultiAxisMove(const MultiAxisMoveParams &params,
                                       const std::string &stepLabel = {});

class MultiAxisMoveStep final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json &j) override;
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string typeName() const override { return "motion.multi_axis_move"; }

private:
    MultiAxisMoveParams m_configParams;
    std::string         m_ctxKey;
};

} // namespace motion_control_plugin::step
