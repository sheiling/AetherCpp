#pragma once
#include "../i_step.h"
#include <thread>
#include <chrono>

namespace scheduler {

// ── 延时步骤 ─────────────────────────────────────────────────
class DelayStep : public IStep {
public:
    DelayStep() = default;
    explicit DelayStep(int ms) : m_delayMs_(ms) {}

    void setDelayMs(int ms) { m_delayMs_ = ms; }

    StepResult execute(FlowContext& /*ctx*/) override {
        // 优先使用构造/setter注入的值，其次用 params.ms
        int ms = (m_delayMs_ > 0) ? m_delayMs_
                                   : m_config_.params.value("ms", 0);
        if (ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return StepResult::ok();
    }
    std::string typeName() const override { return "control.delay"; }

private:
    int m_delayMs_{0};
};

} // namespace scheduler
