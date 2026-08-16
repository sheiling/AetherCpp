#pragma once
#include "../i_step.h"
#include <functional>

namespace scheduler {

// ── 脚本步骤（Lambda / 自定义逻辑）─────────────────────────
class ScriptStep : public IStep {
public:
    using ScriptFn = std::function<StepResult(FlowContext&)>;

    explicit ScriptStep(ScriptFn fn = nullptr) : m_fn_(std::move(fn)) {}

    void setScript(ScriptFn fn) { m_fn_ = std::move(fn); }

    StepResult execute(FlowContext& ctx) override {
        if (!m_fn_) return StepResult::skip();
        return m_fn_(ctx);
    }
    std::string typeName() const override { return "control.script"; }

private:
    ScriptFn m_fn_;
};

} // namespace scheduler
