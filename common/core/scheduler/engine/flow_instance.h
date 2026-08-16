#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <optional>

#include "flow_definition.h"
#include "checkpoint_snapshot.h"
#include "../context/flow_context.h"
#include "../compensation/compensation_manager.h"
// #include "../fsm/hsm.h"
#include "../core/flow_types.h"
#include "../event/event_bus.h"
#include "../scheduler_global.h"

namespace scheduler {

class IFlowPersistence;

// ============================================================
// FlowInstance — 流程运行实例
//   每个实例拥有独立的：FlowContext / HSM / CompensationManager
// ============================================================
class SCHEDULER_EXPORT FlowInstance : public std::enable_shared_from_this<FlowInstance> {
public:
    explicit FlowInstance(std::shared_ptr<FlowDefinition> def,
                          std::shared_ptr<EventBus>      eventBus = nullptr);
    ~FlowInstance();

    // ── 生命周期控制 ─────────────────────────────────────────
    void start();
    void pause();
    void resume();
    void abort();       // 触发补偿事务后中止

    // ── 步骤控制（人工介入）──────────────────────────────────
    void skipCurrentStep();
    void resumeFromStep(const std::string& stepName);

    // ── 状态查询 ─────────────────────────────────────────────
    FlowState          state()      const { return m_state_.load(); }
    const std::string& instanceId() const { return m_instanceId_;  }
    FlowContext&       context()          { return *m_context_;     }
    const FlowContext& context()    const { return *m_context_;     }
    std::string        currentStepName() const;

    // ── 断点恢复 ─────────────────────────────────────────────
    CheckpointSnapshot takeCheckpoint() const;
    void restoreFromCheckpoint(const CheckpointSnapshot& snap);

    // ── 事件注入（注入 HSM 事件）─────────────────────────────
    // void injectEvent(HsmEvent evt);

    // ── 回调注册 ─────────────────────────────────────────────
    void onStateChanged(std::function<void(FlowState, FlowState)> cb);
    void onStepStarted(std::function<void(const std::string&)>    cb);
    void onStepCompleted(std::function<void(const std::string&, StepResult)> cb);
    void onCompleted(std::function<void(bool success)>            cb);

    // ── 持久化 ───────────────────────────────────────────────
    void setPersistence(std::shared_ptr<IFlowPersistence> p);

    // ── 等待完成 ─────────────────────────────────────────────
    bool waitForCompletion(int timeoutMs = 0);

private:
    void runLoop();
    StepResult executeStep(IStep* step, FlowContext& ctx);
    void setState(FlowState newState);
    void performCompensation();

    std::shared_ptr<FlowDefinition>      m_def_;
    std::unique_ptr<FlowContext>         m_context_;
    std::unique_ptr<CompensationManager> m_compensationMgr_;
    std::shared_ptr<EventBus>            m_eventBus_;
    std::shared_ptr<IFlowPersistence>    m_persistence_;

    std::atomic<FlowState>   m_state_{ FlowState::Created };
    std::atomic<bool>        m_abortFlag_{ false };
    std::atomic<bool>        m_pauseFlag_{ false };
    std::atomic<bool>        m_skipFlag_{ false };

    std::string              m_instanceId_;
    mutable std::mutex       m_stepNameMutex_;
    std::string              m_currentStepName_;
    std::optional<std::string> m_skipUntilStep_;

    std::thread              m_runThread_;
    std::mutex               m_pauseMutex_;
    std::condition_variable  m_pauseCv_;
    std::mutex               m_completeMutex_;
    std::condition_variable  m_completeCv_;

    // 回调
    std::function<void(FlowState, FlowState)>         m_stateChangedCb_;
    std::function<void(const std::string&)>           m_stepStartedCb_;
    std::function<void(const std::string&, StepResult)> m_stepCompletedCb_;
    std::function<void(bool)>                         m_completedCb_;
};

} // namespace scheduler
