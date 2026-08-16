#pragma once
#include "../i_step.h"
#include <vector>
#include <future>

namespace scheduler {

// ── 并行步骤（And-Split/Join：全部并发，全部完成后汇聚）────
class ParallelStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        std::vector<std::future<StepResult>> futures;
        futures.reserve(m_children_.size());
        for (auto& child : m_children_)
            futures.push_back(child->executeAsync(ctx));

        bool allOk = true;
        std::string errMsg;
        for (auto& f : futures) {
            try {
                auto r = f.get();
                if (r.isFailed()) { allOk = false; errMsg = r.message; }
            } catch (const std::exception& e) {
                allOk = false;
                errMsg = e.what();
            }
        }
        return allOk ? StepResult::ok()
                     : StepResult::fail("ParallelStep: " + errMsg);
    }
    std::string typeName() const override { return "control.parallel"; }
};

} // namespace scheduler
