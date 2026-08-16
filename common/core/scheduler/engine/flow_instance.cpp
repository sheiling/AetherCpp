#include "flow_instance.h"
#include "../persistence/i_flow_persistence.h"
#include "../step/step_interceptor.h"
#include <Logger.h>

namespace scheduler {

FlowInstance::FlowInstance(std::shared_ptr<FlowDefinition> def,
                             std::shared_ptr<EventBus>      eventBus)
    : m_def_(std::move(def))
    , m_eventBus_(std::move(eventBus))
    , m_instanceId_(generateUUID())
{
    m_context_         = std::make_unique<FlowContext>();
    m_compensationMgr_ = std::make_unique<CompensationManager>();
    m_context_->flowName   = m_def_->name;
    m_context_->instanceId = m_instanceId_;
    m_context_->priority   = m_def_->priority;
}

FlowInstance::~FlowInstance() {
    m_abortFlag_.store(true);
    m_pauseFlag_.store(false);
    m_pauseCv_.notify_all();
    if (m_runThread_.joinable()) {
        // 若析构从 runLoop 所在线程调用（如 shared_ptr 在线程内释放），
        // 不能 join 自身，改为 detach，让线程自然退出
        if (m_runThread_.get_id() == std::this_thread::get_id()) {
            m_runThread_.detach();
        } else {
            m_runThread_.join();
        }
    }
}

void FlowInstance::start() {
    setState(FlowState::Running);
    if (m_def_->onStart) m_def_->onStart(*m_context_);

    if (m_eventBus_) {
        m_eventBus_->publishAsync({
            SchedulerEventType::FlowStarted, m_instanceId_, m_def_->name
        });
    }

    // 通过 shared_from_this 让线程持有对 FlowInstance 的强引用，
    // 防止 onInstanceCompleted 触发析构时发生同线程 join 死锁
    auto self = shared_from_this();
    m_runThread_ = std::thread([this, self]{ runLoop(); });
}

void FlowInstance::pause() {
    if (m_state_.load() == FlowState::Running) {
        m_pauseFlag_.store(true);
        setState(FlowState::Paused);
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowPaused, m_instanceId_, m_def_->name});
    }
}

void FlowInstance::resume() {
    if (m_state_.load() == FlowState::Paused ||
        m_state_.load() == FlowState::AlarmPaused)
    {
        m_pauseFlag_.store(false);
        setState(FlowState::Running);
        m_pauseCv_.notify_all();
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowResumed, m_instanceId_, m_def_->name});
    }
}

void FlowInstance::abort() {
    m_abortFlag_.store(true);
    m_pauseFlag_.store(false);
    m_pauseCv_.notify_all();
}

void FlowInstance::skipCurrentStep() {
    m_skipFlag_.store(true);
}

void FlowInstance::resumeFromStep(const std::string& stepName) {
    m_skipUntilStep_ = stepName;
}

std::string FlowInstance::currentStepName() const {
    std::lock_guard<std::mutex> lk(m_stepNameMutex_);
    return m_currentStepName_;
}

CheckpointSnapshot FlowInstance::takeCheckpoint() const {
    CheckpointSnapshot snap;
    snap.instanceId    = m_instanceId_;
    snap.flowName      = m_def_->name;
    snap.contextData   = m_context_->serialize();
    snap.completedSteps = m_compensationMgr_->getCompletedStepNames();
    snap.compensationStack = snap.completedSteps;
    snap.savedAt       = std::chrono::system_clock::now();
    {
        std::lock_guard<std::mutex> lk(m_stepNameMutex_);
        snap.checkpointId = m_currentStepName_;
    }
    return snap;
}

void FlowInstance::restoreFromCheckpoint(const CheckpointSnapshot& snap) {
    m_context_->deserialize(snap.contextData);
    m_skipUntilStep_ = snap.checkpointId;
}

// void FlowInstance::injectEvent(HsmEvent evt) {
//     // 保留扩展点（例如注入信号到边界事件）
//     (void)evt;
// }

void FlowInstance::onStateChanged(std::function<void(FlowState, FlowState)> cb) {
    m_stateChangedCb_ = std::move(cb);
}
void FlowInstance::onStepStarted(std::function<void(const std::string&)> cb) {
    m_stepStartedCb_ = std::move(cb);
}
void FlowInstance::onStepCompleted(std::function<void(const std::string&, StepResult)> cb) {
    m_stepCompletedCb_ = std::move(cb);
}
void FlowInstance::onCompleted(std::function<void(bool)> cb) {
    m_completedCb_ = std::move(cb);
}

void FlowInstance::setPersistence(std::shared_ptr<IFlowPersistence> p) {
    m_persistence_ = std::move(p);
}

bool FlowInstance::waitForCompletion(int timeoutMs) {
    std::unique_lock<std::mutex> lk(m_completeMutex_);
    auto pred = [this]{
        auto s = m_state_.load();
        return s == FlowState::Completed ||
               s == FlowState::Failed    ||
               s == FlowState::Aborted;
    };
    if (timeoutMs > 0)
        return m_completeCv_.wait_for(lk, std::chrono::milliseconds(timeoutMs), pred);
    else {
        m_completeCv_.wait(lk, pred);
        return true;
    }
}

// ── 私有方法 ──────────────────────────────────────────────────

void FlowInstance::setState(FlowState newState) {
    FlowState old = m_state_.exchange(newState);
    if (old != newState && m_stateChangedCb_)
        m_stateChangedCb_(old, newState);
}

void FlowInstance::runLoop() {
  try {
    if (!m_def_->rootStep) {
        setState(FlowState::Completed);
        m_completeCv_.notify_all();
        return;
    }

    // 执行根步骤（通常是 SequentialStep）
    auto& root = m_def_->rootStep;
    const auto& steps = root->children().empty()
        ? std::vector<std::shared_ptr<IStep>>{root}
        : root->children();

    bool success = true;
    for (auto& step : steps) {
        if (m_abortFlag_.load()) { success = false; break; }

        // 暂停检查
        {
            std::unique_lock<std::mutex> lk(m_pauseMutex_);
            m_pauseCv_.wait(lk, [this]{ return !m_pauseFlag_.load(); });
        }

        if (m_abortFlag_.load()) { success = false; break; }

        // 跳过检查（断点恢复）
        if (m_skipUntilStep_.has_value()) {
            if (step->name() != *m_skipUntilStep_) continue;
            else m_skipUntilStep_.reset();
        }

        {
            std::lock_guard<std::mutex> lk(m_stepNameMutex_);
            m_currentStepName_ = step->name();
        }

        if (m_stepStartedCb_) m_stepStartedCb_(step->name());
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::StepStarted,
                                       m_instanceId_, m_def_->name, step->name()});

        auto t0 = std::chrono::steady_clock::now();
        auto result = executeStep(step.get(), *m_context_);
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();

        // 记录历史
        m_context_->history.push_back({step->name(), result.status, elapsed, result.message});

        if (m_stepCompletedCb_) m_stepCompletedCb_(step->name(), result);

        if (result.isOk()) {
            // 记录补偿栈
            if (step->config().compensationStep && m_def_->enableCompensation) {
                m_compensationMgr_->recordCompleted(step->name(),
                                                    step->config().compensationStep);
            }
            // 检查点快照
            if (step->config().isCheckpoint && m_persistence_) {
                auto snap = takeCheckpoint();
                m_persistence_->saveCheckpoint(snap);
            }
            if (m_eventBus_)
                m_eventBus_->publishAsync({SchedulerEventType::StepCompleted,
                                           m_instanceId_, m_def_->name, step->name()});
        } else if (result.isFailed()) {
            success = false;
            if (m_eventBus_)
                m_eventBus_->publishAsync({SchedulerEventType::StepFailed,
                                           m_instanceId_, m_def_->name, step->name()});
            if (!step->config().skipOnFail && !step->config().optional)
                break;
            success = true;  // 可选步骤失败不算整体失败
        }
    }

    if (m_abortFlag_.load()) {
        performCompensation();
        setState(FlowState::Aborted);
        if (m_def_->onAbort) m_def_->onAbort(*m_context_);
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowAborted, m_instanceId_, m_def_->name});
    } else if (success) {
        m_compensationMgr_->clear();
        setState(FlowState::Completed);
        if (m_def_->onComplete) m_def_->onComplete(*m_context_);
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowCompleted, m_instanceId_, m_def_->name});
    } else {
        setState(FlowState::Failed);
        if (m_def_->onFailed) m_def_->onFailed(*m_context_);
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowFailed, m_instanceId_, m_def_->name});
    }

    if (m_completedCb_) m_completedCb_(m_state_.load() == FlowState::Completed);
    m_completeCv_.notify_all();

  } catch (const std::exception& ex) {
      LOG_ERROR("[FlowInstance] runLoop uncaught exception: {}", ex.what());
      setState(FlowState::Failed);
      if (m_def_ && m_def_->onFailed) m_def_->onFailed(*m_context_);
      if (m_completedCb_) m_completedCb_(false);
      m_completeCv_.notify_all();
  } catch (...) {
      LOG_ERROR("[FlowInstance] runLoop uncaught unknown exception");
      setState(FlowState::Failed);
      if (m_completedCb_) m_completedCb_(false);
      m_completeCv_.notify_all();
  }
}

// ── 辅助：递归展开并执行拦截器链 ────────────────────────────
// 拦截器链布局：flow_interceptors → step_interceptors → coreExec
// 每个拦截器通过调用 proceed() 决定是否继续向下执行。
static StepResult runInterceptorChain(
    IStep*                                      step,
    FlowContext&                                ctx,
    const std::vector<StepInterceptorFn>&       chain,
    std::size_t                                 idx,
    const std::function<StepResult()>&          coreExec)
{
    if (idx >= chain.size()) return coreExec();
    return chain[idx](step, ctx, [&]() -> StepResult {
        return runInterceptorChain(step, ctx, chain, idx + 1, coreExec);
    });
}

StepResult FlowInstance::executeStep(IStep* step, FlowContext& ctx) {
    // ── 1. 构建拦截器链：流程级（先）→ 步骤级（后）────────────
    // 执行顺序（前置）：flow[0] → flow[1] → ... → step[0] → step[1] → 核心执行
    // 返回顺序（后置）：核心执行 → step[N-1] → ... → step[0] → flow[N-1] → ...
    std::vector<StepInterceptorFn> chain;
    auto& reg = StepInterceptorRegistry::instance();

    // 流程级拦截器
    for (const auto& name : m_def_->interceptorNames) {
        auto fn = reg.getInterceptor(name);
        if (fn) {
            chain.push_back(fn);
        } else {
            LOG_WARN("[FlowInstance] 流程级拦截器 '{}' 未注册，已跳过", name);
        }
    }

    // 步骤级拦截器
    for (const auto& name : step->config().interceptorNames) {
        auto fn = reg.getInterceptor(name);
        if (fn) {
            chain.push_back(fn);
        } else {
            LOG_WARN("[FlowInstance] 步骤 [{}] 的拦截器 '{}' 未注册，已跳过",
                     step->name(), name);
        }
    }

    // ── 2. 核心执行逻辑（超时 + 重试）────────────────────────
    // 这是拦截器链的最终目标；拦截器通过 proceed() 最终调用到此处
    auto coreExec = [this, step, &ctx]() -> StepResult {
        int timeout = step->config().timeoutMs;
        if (timeout <= 0) {
            // 无超时限制：在当前线程直接执行（含重试）
            return step->executeWithRetry(ctx);
        }

        // 有超时限制：另起线程执行，主线程等待超时
        std::promise<StepResult> prom;
        auto fut = prom.get_future();
        std::thread([step, &ctx, &prom]() {
            try { prom.set_value(step->executeWithRetry(ctx)); }
            catch (...) {
                prom.set_value(StepResult::fail("Step threw exception"));
            }
        }).detach();

        if (fut.wait_for(std::chrono::milliseconds(timeout))
            == std::future_status::timeout)
        {
            step->cancel();
            return StepResult::timeout("Step timeout: " + step->name());
        }
        return fut.get();
    };

    // ── 3. 无拦截器：直接执行（零开销）──────────────────────
    if (chain.empty()) return coreExec();

    // ── 4. 有拦截器：展开职责链后执行 ────────────────────────
    return runInterceptorChain(step, ctx, chain, 0, coreExec);
}

void FlowInstance::performCompensation() {
    if (m_def_->enableCompensation) {
        m_compensationMgr_->compensateAll(*m_context_);
    }
}

} // namespace scheduler
