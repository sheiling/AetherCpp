#pragma once
#include "../i_step.h"

namespace scheduler {

// ── 顺序步骤（串行执行所有子步骤）──────────────────────────
class SequentialStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        for (auto& child : m_children_) {
            ctx.pushStepScope();
            auto result = child->executeWithRetry(ctx);
            ctx.popStepScope();

            if (result.isFailed()) {
                if (child->config().skipOnFail || child->config().optional)
                    continue;
                return result;
            }
        }
        return StepResult::ok();
    }
    std::string typeName() const override { return "control.sequential"; }
};

} // namespace scheduler
