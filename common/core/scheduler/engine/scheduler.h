#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <optional>
#include <chrono>
#include <functional>

#include "../scheduler_global.h"
#include "../core/flow_types.h"
#include "../core/uuid_gen.h"
#include "flow_definition.h"
#include "flow_instance.h"
#include "../event/event_bus.h"
#include "../persistence/i_flow_persistence.h"

namespace scheduler {

// ============================================================
// SubmitRequest — 流程提交请求
// ============================================================
struct SubmitRequest {
    std::string flowName;
    int         priority{0};    // 0-9，越大越高
    json        params;         // 传入 FlowContext.initParams
    std::string requestId;      // 外部追踪 ID（空则自动生成）
};

// ============================================================
// RecoveryMode / RecoveryRequest — 人工介入恢复
// ============================================================
enum class RecoveryMode {
    RetryCurrentStep,
    SkipCurrentStep,
    ResumeFromStep,
    ResumeFromCheckpoint,
    AbortFlow,
};

struct RecoveryRequest {
    std::string  instanceId;
    RecoveryMode mode;
    std::string  targetStepName;    // ResumeFromStep 时有效
};

// ============================================================
// ResourceLocker — 设备资源锁（防死锁：固定排序加锁）
// ============================================================
class SCHEDULER_EXPORT ResourceLocker {
public:
    // 批量申请资源（固定排序，防止死锁）
    bool tryAcquireAll(const std::string& instanceId,
                       std::vector<std::string> resources,
                       int timeoutMs = 5000);

    // 释放该实例持有的所有资源
    void releaseAll(const std::string& instanceId);

    // 查询资源被哪个实例持有
    std::optional<std::string> holder(const std::string& resource) const;

    bool isAvailable(const std::string& resource) const;

private:
    mutable std::mutex      m_mutex_;
    std::condition_variable m_cv_;
    // resource -> instanceId
    std::unordered_map<std::string, std::string> m_locks_;
};

// ============================================================
// IDispatchStrategy — 调度策略接口
// ============================================================
struct PendingRequest {
    SubmitRequest req;
    std::chrono::steady_clock::time_point submitTime;
};

class SCHEDULER_EXPORT IDispatchStrategy {
public:
    virtual ~IDispatchStrategy() = default;
    virtual std::optional<PendingRequest> selectNext(
        const std::vector<PendingRequest>& waitQueue,
        const std::unordered_map<std::string, std::shared_ptr<FlowInstance>>& running
    ) const = 0;
};

// 内置：优先级 + FIFO
class SCHEDULER_EXPORT PriorityFIFOStrategy : public IDispatchStrategy {
public:
    std::optional<PendingRequest> selectNext(
        const std::vector<PendingRequest>& waitQueue,
        const std::unordered_map<std::string, std::shared_ptr<FlowInstance>>&
    ) const override;
};

// ============================================================
// Scheduler — 流程调度引擎（核心）
// ============================================================
class SCHEDULER_EXPORT Scheduler {
public:
    explicit Scheduler(std::shared_ptr<EventBus> eventBus = nullptr);
    ~Scheduler();

    // ── 流程注册 ─────────────────────────────────────────────
    void registerFlow(std::shared_ptr<FlowDefinition> def);
    bool hasFlow(const std::string& name) const;

    // ── 流程提交与控制 ───────────────────────────────────────
    std::string submit(SubmitRequest req);   // 返回 instanceId
    void pause(const std::string& instanceId);
    void resume(const std::string& instanceId);
    void abort(const std::string& instanceId);

    // ── 人工介入恢复 ─────────────────────────────────────────
    void handleRecovery(const RecoveryRequest& req);

    // ── 全局控制 ─────────────────────────────────────────────
    void stopAll();         // 优雅停机：标记停机中，等当前步骤完成后停止
    void pauseAll();        // 暂停所有运行中的流程实例（可恢复）
    void resumeAll();       // 恢复所有已暂停的流程实例
    void emergencyStop();   // 紧急停机（立即中止所有流程）

    // 当所有运行实例均完成且等待队列为空时，触发回调（用于优雅停机通知）
    void setOnIdle(std::function<void()> cb)  { m_onIdleCb_ = std::move(cb); }

    // 查询当前是否有正在运行的流程
    bool isIdle() const;

    // 禁止调度新流程（配合优雅停机使用）
    void setAcceptingRequests(bool accept)    { m_acceptingRequests_.store(accept); }

    // ── 查询 ─────────────────────────────────────────────────
    std::vector<FlowInstanceInfo> runningInstances() const;
    FlowInstanceInfo              getInstanceInfo(const std::string& id) const;
    SchedulerStats                stats() const;

    // ── 配置 ─────────────────────────────────────────────────
    void setMaxConcurrent(int n)                          { m_maxConcurrent_ = n; }
    void setDispatchStrategy(std::shared_ptr<IDispatchStrategy> s) { m_strategy_ = std::move(s); }
    void setPersistence(std::shared_ptr<IFlowPersistence> p)       { m_persistence_ = std::move(p); }
    void setEventBus(std::shared_ptr<EventBus> bus)                { m_eventBus_ = std::move(bus); }

    // ── 断电恢复 ─────────────────────────────────────────────
    void recoverFromPersistence();

    std::shared_ptr<EventBus> eventBus() const { return m_eventBus_; }

private:
    void scheduleLoop();
    void tryScheduleNext();
  // resourceOwnerId：资源锁持有者，与 tryAcquireAll 时传入的 requestId 一致
    void onInstanceCompleted(const std::string& instanceId,
                             const std::string& resourceOwnerId,
                             bool success);
    std::shared_ptr<FlowInstance> getRunningInstance(const std::string& instanceId) const;

    mutable std::mutex m_mutex_;
    std::condition_variable m_cv_;

    std::unordered_map<std::string, std::shared_ptr<FlowDefinition>> m_registry_;
    std::unordered_map<std::string, std::shared_ptr<FlowInstance>>   m_running_;
    std::vector<PendingRequest>                                       m_waitQueue_;

    std::unique_ptr<ResourceLocker>       m_resourceLocker_;
    std::shared_ptr<IDispatchStrategy>    m_strategy_;
    std::shared_ptr<IFlowPersistence>     m_persistence_;
    std::shared_ptr<EventBus>             m_eventBus_;

    int m_maxConcurrent_{4};
    std::atomic<bool> m_schedulerRunning_{true};
    std::thread       m_scheduleThread_;

    // 统计
    std::atomic<int>  m_completedTotal_{0};
    std::atomic<int>  m_failedTotal_{0};
    std::atomic<int>  m_abortedTotal_{0};
    std::atomic<bool> m_acceptingRequests_{true};

    // 空闲回调（优雅停机使用）
    std::function<void()> m_onIdleCb_;
};

} // namespace scheduler
