#pragma once
#include "../i_step.h"
#include <functional>

namespace scheduler {

// ── 循环步骤 ────────────────────────────────────────────────
class LoopStep : public IStep {
public:
    LoopStep() = default;

    // 固定次数循环
    explicit LoopStep(int times) : m_max_(times) {}

    // 条件循环
    explicit LoopStep(std::function<bool(const FlowContext&)> cond)
        : m_cond_(std::move(cond)) {}

    void setCondition(std::function<bool(const FlowContext&)> cond) { m_cond_ = std::move(cond); }
    void setMaxIterations(int n) { m_max_ = n; }
    void setBody(std::shared_ptr<IStep> body) { m_body_ = std::move(body); }

    StepResult execute(FlowContext& ctx) override {
        int iter = 0;
        while ((!m_cond_ || m_cond_(ctx)) && (m_max_ <= 0 || iter < m_max_)) {
            ctx.set("loop_iteration", iter, VariableScope::STEP);
            ++iter;
            // 若通过 addChild 添加了子步骤，则执行所有子步骤
            if (!m_children_.empty()) {
                for (auto& child : m_children_) {
                    auto r = child->executeWithRetry(ctx);
                    if (r.isFailed() && !child->config().skipOnFail && !child->config().optional)
                        return r;
                }
            } else if (m_body_) {
                auto r = m_body_->executeWithRetry(ctx);
                if (r.isFailed()) return r;
            }
        }
        return StepResult::ok();
    }

    std::string typeName() const override { return "control.loop"; }

private:
    std::function<bool(const FlowContext&)> m_cond_;
    int                                     m_max_{0};
    std::shared_ptr<IStep>                  m_body_;
};

} // namespace scheduler
