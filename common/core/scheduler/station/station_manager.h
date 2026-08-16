#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>
#include <common/3rdparty/nlohmann/json.hpp>

#include "../scheduler_global.h"
#include "../core/flow_types.h"
#include "../engine/scheduler.h"
#include "../engine/flow_instance.h"
#include "../step/sync/station_barrier.h"
#include "../step/sync/pipeline_channel.h"
#include "../event/event_bus.h"

namespace scheduler {

// ============================================================
// StationContinuousConfig — 单工位持续运行配置
// ============================================================
struct StationContinuousConfig {
    int  intervalMs{0};         // 两次执行之间的最小间隔（ms），0=立即
    bool resubmitOnFail{true};  // 失败后是否继续执行下一片
};

// ============================================================
// StationConfig — 工位配置
// ============================================================
struct StationConfig {
    std::string              stationId;
    std::string              name;
    std::vector<std::string> ownedDevices;  // 专属设备
    std::string              mainFlowName;  // 主流程名称
    json                     params;
};

// ============================================================
// Station — 单工位（含独立流程实例 + 持续运行模式）
// ============================================================
class SCHEDULER_EXPORT Station {
public:
    Station(StationConfig              config,
            std::shared_ptr<Scheduler> scheduler,
            std::shared_ptr<EventBus>  eventBus = nullptr);
    ~Station();

    const std::string& stationId() const { return m_config_.stationId; }
    const std::string& name()      const { return m_config_.name; }
    FlowState          flowState() const;
    std::string        instanceId() const;

    // ── 流程控制 ─────────────────────────────────────────────
    void startFlow(json params = {});
    void stopFlow();
    void pauseFlow();
    void resumeFlow();
    bool isRunning() const;

    // ── 持续运行模式 ──────────────────────────────────────────
    // 开启后，每次流程完成（或失败）自动重新提交，实现连续接料
    void enableContinuousMode(StationContinuousConfig cfg = {});
    void disableContinuousMode();
    bool isContinuousMode() const { return m_continuousMode_.load(); }

    // ── 状态变化回调（可选）──────────────────────────────────
    void onFlowStateChanged(
        std::function<void(const std::string& stationId,
                           FlowState oldState, FlowState newState)> cb) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_stateCb_ = std::move(cb);
    }

private:
    void onFlowEvent(const SchedulerEvent& e, bool isSuccess);
    void doStartFlow(json params);  // 不加锁版本，供内部调用

    StationConfig                       m_config_;
    std::shared_ptr<Scheduler>          m_scheduler_;
    std::shared_ptr<EventBus>           m_eventBus_;
    std::string                         m_currentInstanceId_;
    mutable std::mutex                  m_mutex_;

    // 持续运行
    std::atomic<bool>          m_continuousMode_{false};
    StationContinuousConfig    m_continuousCfg_;
    mutable std::mutex         m_continuousMutex_;

    // EventBus 订阅 ID（析构时注销）
    EventBus::SubscriberId     m_completedSubId_{-1};
    EventBus::SubscriberId     m_failedSubId_{-1};

    // 状态变化回调
    std::function<void(const std::string&, FlowState, FlowState)> m_stateCb_;
};

// ============================================================
// StationManager — 多工位管理器
//
// 兼容单工位（addStation + startAll）和多工位（addStation×N）模式
// 工位间通过 PipelineChannel 传递数据，通过 StationBarrier 同步
// ============================================================
class SCHEDULER_EXPORT StationManager {
public:
    explicit StationManager(std::shared_ptr<Scheduler> scheduler,
                             std::shared_ptr<EventBus>  eventBus = nullptr);

    // ── 工位管理 ─────────────────────────────────────────────
    void   addStation(StationConfig config);
    void   removeStation(const std::string& stationId);
    Station* getStation(const std::string& stationId);
    std::vector<Station*> allStations();

    // ── 批量控制 ─────────────────────────────────────────────
    void startAll(json params = {});
    void stopAll();
    void pauseAll();
    void resumeAll();

    // ── 批量持续运行模式 ──────────────────────────────────────
    void enableContinuousMode(StationContinuousConfig cfg = {});
    void disableContinuousMode();

    // ── 同步屏障工厂 ─────────────────────────────────────────
    // 创建屏障并同时注册到 BarrierRegistry（供 BarrierStep 懒查找）
    std::shared_ptr<StationBarrier> createBarrier(
        const std::string& name,
        std::vector<std::string> participantIds);

    std::shared_ptr<StationBarrier> getBarrier(const std::string& name) const;

    // ── 流水线通道工厂 ────────────────────────────────────────
    // 创建通道并注册到 PipelineChannelHub（供 ChannelSendStep/ReceiveStep 使用）
    std::shared_ptr<PipelineChannel> createChannel(
        const std::string& name, int capacity = 0);

    std::shared_ptr<PipelineChannel> getChannel(const std::string& name) const;

    // ── 状态监控 ─────────────────────────────────────────────
    std::map<std::string, FlowState> allStationStates() const;
    bool allIdle() const;  // 所有工位均处于非运行状态

private:
    std::shared_ptr<Scheduler>  m_scheduler_;
    std::shared_ptr<EventBus>   m_eventBus_;

    mutable std::mutex          m_mutex_;
    std::unordered_map<std::string, std::unique_ptr<Station>>       m_stations_;
    std::unordered_map<std::string, std::shared_ptr<StationBarrier>> m_barriers_;
    std::unordered_map<std::string, std::shared_ptr<PipelineChannel>> m_channels_;
};

} // namespace scheduler
