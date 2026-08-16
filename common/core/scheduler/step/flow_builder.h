#pragma once

// ============================================================
// FlowBuilder — 流程 DSL（链式调用构建器）
//
// 用法示例（C++ DSL）：
//
//   auto def = FlowBuilder("采样流程")
//       .timeout(30000)
//       .enableCompensation(true)
//       .step<ScriptStep>([](FlowContext& ctx){
//           ctx.set("sample_id", 42);
//           return StepResult::ok();
//       }).name("初始化")
//       .sequential()
//           .step<DelayStep>(500).name("延时预热")
//           .step<ScriptStep>([](FlowContext& ctx){
//               LOG_INFO("采样中...");
//               return StepResult::ok();
//           }).name("执行采样")
//       .end()
//       .build();
//
// ============================================================

#include <string>
#include <memory>
#include <functional>
#include <stack>
#include <vector>
#include <stdexcept>

#include "i_step.h"
#include "control/sequential_step.h"
#include "control/parallel_step.h"
#include "control/loop_step.h"
#include "control/delay_step.h"
#include "control/script_step.h"
#include "control/exclusive_gateway.h"
#include "../engine/flow_definition.h"
#include "../scheduler_global.h"

namespace scheduler {

// ============================================================
// StepBuilder — 单步骤配置链
// ============================================================
class StepBuilder {
public:
    explicit StepBuilder(std::shared_ptr<IStep> step) : m_step_(std::move(step)) {}

    StepBuilder& name(const std::string& n) {
        auto cfg = m_step_->config();
        cfg.name = n;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& timeout(int ms) {
        auto cfg = m_step_->config();
        cfg.timeoutMs = ms;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& retry(int count, int intervalMs = 500) {
        auto cfg = m_step_->config();
        cfg.retryCount      = count;
        cfg.retryIntervalMs = intervalMs;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& skipOnFail(bool v = true) {
        auto cfg = m_step_->config();
        cfg.skipOnFail = v;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& optional(bool v = true) {
        auto cfg = m_step_->config();
        cfg.optional = v;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& checkpoint(bool v = true) {
        auto cfg = m_step_->config();
        cfg.isCheckpoint = v;
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    StepBuilder& compensate(std::shared_ptr<IStep> comp) {
        auto cfg = m_step_->config();
        cfg.compensationStep = std::move(comp);
        m_step_->setConfig(std::move(cfg));
        return *this;
    }

    std::shared_ptr<IStep> get() const { return m_step_; }

private:
    std::shared_ptr<IStep> m_step_;
};

// ============================================================
// FlowBuilder — 流程定义构建器
// ============================================================
class FlowBuilder {
public:
    explicit FlowBuilder(std::string flowName) {
        m_def_ = std::make_shared<FlowDefinition>();
        m_def_->name = std::move(flowName);

        // 根节点始终是 SequentialStep
        auto root = std::make_shared<SequentialStep>();
        root->setConfig(StepConfig{m_def_->name + "_root", "control.sequential"});
        m_def_->rootStep = root;

        // 初始化构建栈，当前容器指向根
        m_containerStack_.push(root);
    }

    // ── 流程元信息 ──────────────────────────────────────────
    FlowBuilder& description(const std::string& desc) {
        m_def_->description = desc; return *this;
    }
    FlowBuilder& version(int v)                   { m_def_->version = v;               return *this; }
    FlowBuilder& priority(int p)                  { m_def_->priority = p;              return *this; }
    FlowBuilder& timeout(int ms)                  { m_def_->timeoutMs = ms;            return *this; }
    FlowBuilder& maxInstances(int n)              { m_def_->maxInstances = n;          return *this; }
    FlowBuilder& enableCompensation(bool v)        { m_def_->enableCompensation = v;    return *this; }
    FlowBuilder& requiredDevices(std::vector<std::string> devs) {
        m_def_->requiredDevices = std::move(devs); return *this;
    }

    // ── 生命周期回调 ──────────────────────────────────────────
    FlowBuilder& onStart(std::function<void(FlowContext&)> cb)    { m_def_->onStart    = std::move(cb); return *this; }
    FlowBuilder& onComplete(std::function<void(FlowContext&)> cb) { m_def_->onComplete = std::move(cb); return *this; }
    FlowBuilder& onFailed(std::function<void(FlowContext&)> cb)   { m_def_->onFailed   = std::move(cb); return *this; }
    FlowBuilder& onAbort(std::function<void(FlowContext&)> cb)    { m_def_->onAbort    = std::move(cb); return *this; }

    // ── 添加脚本步骤（Lambda）──────────────────────────────────
    FlowBuilder& script(const std::string& name,
                        std::function<StepResult(FlowContext&)> fn)
    {
        auto step = std::make_shared<ScriptStep>(std::move(fn));
        step->setConfig(StepConfig{name, "control.script"});
        currentContainer()->addChild(step);
        return *this;
    }

    // ── 添加延时步骤 ───────────────────────────────────────────
    FlowBuilder& delay(int ms, const std::string& name = "") {
        auto step = std::make_shared<DelayStep>(ms);
        step->setConfig(StepConfig{name.empty() ? ("delay_" + std::to_string(ms) + "ms") : name,
                                   "control.delay"});
        currentContainer()->addChild(step);
        return *this;
    }

    // ── 添加任意步骤 ───────────────────────────────────────────
    FlowBuilder& addStep(std::shared_ptr<IStep> step) {
        currentContainer()->addChild(std::move(step));
        return *this;
    }

    // ── 进入串行子块 ───────────────────────────────────────────
    FlowBuilder& sequential(const std::string& name = "") {
        auto step = std::make_shared<SequentialStep>();
        step->setConfig(StepConfig{name.empty() ? ("seq_" + std::to_string(m_counter_++)) : name,
                                   "control.sequential"});
        currentContainer()->addChild(step);
        m_containerStack_.push(step);
        return *this;
    }

    // ── 进入并行子块 ───────────────────────────────────────────
    FlowBuilder& parallel(const std::string& name = "") {
        auto step = std::make_shared<ParallelStep>();
        step->setConfig(StepConfig{name.empty() ? ("par_" + std::to_string(m_counter_++)) : name,
                                   "control.parallel"});
        currentContainer()->addChild(step);
        m_containerStack_.push(step);
        return *this;
    }

    // ── 进入循环子块 ───────────────────────────────────────────
    FlowBuilder& loop(int times, const std::string& name = "") {
        auto step = std::make_shared<LoopStep>(times);
        step->setConfig(StepConfig{name.empty() ? ("loop_" + std::to_string(m_counter_++)) : name,
                                   "control.loop"});
        currentContainer()->addChild(step);
        m_containerStack_.push(step);
        return *this;
    }

    FlowBuilder& loopWhile(std::function<bool(const FlowContext&)> cond,
                            const std::string& name = "")
    {
        auto step = std::make_shared<LoopStep>(std::move(cond));
        step->setConfig(StepConfig{name.empty() ? ("loopWhile_" + std::to_string(m_counter_++)) : name,
                                   "control.loop"});
        currentContainer()->addChild(step);
        m_containerStack_.push(step);
        return *this;
    }

    // ── 进入条件网关子块 ──────────────────────────────────────
    FlowBuilder& gateway(const std::string& name = "") {
        auto step = std::make_shared<ExclusiveGateway>();
        step->setConfig(StepConfig{name.empty() ? ("gw_" + std::to_string(m_counter_++)) : name,
                                   "gateway.exclusive"});
        currentContainer()->addChild(step);
        m_containerStack_.push(step);
        return *this;
    }

    // ── 退出当前块（回到父容器）──────────────────────────────
    FlowBuilder& end() {
        if (m_containerStack_.size() <= 1) {
            throw std::logic_error("FlowBuilder::end() called too many times");
        }
        m_containerStack_.pop();
        return *this;
    }

    // ── 构建最终流程定义 ──────────────────────────────────────
    std::shared_ptr<FlowDefinition> build() {
        while (m_containerStack_.size() > 1) m_containerStack_.pop();
        return m_def_;
    }

private:
    std::shared_ptr<IStep> currentContainer() {
        if (m_containerStack_.empty())
            throw std::logic_error("FlowBuilder: container stack is empty");
        return m_containerStack_.top();
    }

    std::shared_ptr<FlowDefinition>          m_def_;
    std::stack<std::shared_ptr<IStep>>       m_containerStack_;
    int                                      m_counter_{0};
};

} // namespace scheduler
