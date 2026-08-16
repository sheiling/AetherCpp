#pragma once
#include "../i_step.h"
#include <functional>

namespace scheduler {

// ── 排他网关（XOR-Split：仅执行第一个满足条件的分支）────────
class ExclusiveGateway : public IStep {
public:
    struct Branch {
        std::function<bool(const FlowContext&)> guard;
        std::shared_ptr<IStep>                  step;
        std::string                             label;
    };

    void addBranch(Branch b)                    { m_branches_.push_back(std::move(b)); }
    void setDefault(std::shared_ptr<IStep> def) { m_default_ = std::move(def); }

    StepResult execute(FlowContext& ctx) override {
        for (auto& b : m_branches_) {
            if (!b.guard || b.guard(ctx))
                return b.step->executeWithRetry(ctx);
        }
        return m_default_ ? m_default_->executeWithRetry(ctx) : StepResult::skip();
    }
    std::string typeName() const override { return "gateway.exclusive"; }

private:
    std::vector<Branch>    m_branches_;
    std::shared_ptr<IStep> m_default_;
};

} // namespace scheduler
