#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <any>
#include <chrono>
#include <string>

#include "../scheduler_global.h"
#include "../core/uuid_gen.h"

namespace scheduler {

// ============================================================
// 调度器事件类型
// ============================================================
enum class SchedulerEventType {
    // 流程事件
    FlowSubmitted, FlowStarted, FlowCompleted, FlowFailed,
    FlowAborted,   FlowPaused,  FlowResumed,  FlowRecovered,
    // 步骤事件
    StepStarted, StepCompleted, StepFailed, StepSkipped,
    StepRetrying, StepTimeout,
    // 报警事件
    AlarmRaised, AlarmAcknowledged, AlarmCleared,
    // 系统事件
    EmergencyStop, SafeStop, SystemReady,
    // 人工任务事件
    HumanTaskCreated, HumanTaskCompleted, HumanTaskTimeout,
};

// ============================================================
// SchedulerEvent — 事件载体
// ============================================================
struct SchedulerEvent {
    SchedulerEventType                     type;
    std::string                            instanceId;
    std::string                            flowName;
    std::string                            stepName;
    std::any                               payload;
    std::chrono::system_clock::time_point  timestamp{ std::chrono::system_clock::now() };
};

// ============================================================
// EventBus — 发布/订阅事件总线
//   - subscribe()    订阅指定类型
//   - subscribeAll() 订阅所有事件
//   - publish()      同步发布
//   - publishAsync() 异步发布（投入队列，独立线程分发）
// ============================================================
class SCHEDULER_EXPORT EventBus {
public:
    using SubscriberId = int;
    using Handler      = std::function<void(const SchedulerEvent&)>;

    EventBus();
    ~EventBus();

    // ── 订阅 ──────────────────────────────────────────────────
    SubscriberId subscribe(SchedulerEventType type, Handler handler);
    SubscriberId subscribeAll(Handler handler);
    void         unsubscribe(SubscriberId id);

    // ── 发布 ──────────────────────────────────────────────────
    void publish(SchedulerEvent evt);           // 同步（调用线程直接触发）
    void publishAsync(SchedulerEvent evt);      // 异步（投队列，后台线程分发）

private:
    void dispatchLoop();

    struct Subscriber {
        SubscriberId   id;
        Handler        handler;
        bool           all{false};                     // 是否订阅所有类型
        SchedulerEventType type{};
    };

    mutable std::shared_mutex            m_subMutex_;
    std::vector<Subscriber>              m_subscribers_;

    // 异步队列
    std::queue<SchedulerEvent>           m_asyncQueue_;
    std::mutex                           m_queueMutex_;
    std::condition_variable              m_cv_;
    std::thread                          m_dispatchThread_;
    std::atomic<bool>                    m_running_{true};
};

} // namespace scheduler
