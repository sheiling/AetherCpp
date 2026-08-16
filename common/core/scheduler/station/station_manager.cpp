#include "station_manager.h"
#include <Logger.h>
#include <thread>
#include <chrono>

namespace scheduler {

// ============================================================
// Station
// ============================================================

Station::Station(StationConfig              config,
                 std::shared_ptr<Scheduler> scheduler,
                 std::shared_ptr<EventBus>  eventBus)
    : m_config_(std::move(config))
    , m_scheduler_(std::move(scheduler))
    , m_eventBus_(std::move(eventBus))
{
    if (!m_eventBus_) return;

    // 订阅流程完成事件：判断是否是本工位的实例，触发持续模式
    m_completedSubId_ = m_eventBus_->subscribe(
        SchedulerEventType::FlowCompleted,
        [this](const SchedulerEvent& e) { onFlowEvent(e, true); }
    );
    m_failedSubId_ = m_eventBus_->subscribe(
        SchedulerEventType::FlowFailed,
        [this](const SchedulerEvent& e) { onFlowEvent(e, false); }
    );
}

Station::~Station() {
    if (m_eventBus_) {
        if (m_completedSubId_ >= 0) m_eventBus_->unsubscribe(m_completedSubId_);
        if (m_failedSubId_   >= 0) m_eventBus_->unsubscribe(m_failedSubId_);
    }
}

FlowState Station::flowState() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    if (m_currentInstanceId_.empty()) return FlowState::Created;
    auto info = m_scheduler_->getInstanceInfo(m_currentInstanceId_);
    return info.state;
}

std::string Station::instanceId() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    return m_currentInstanceId_;
}

void Station::startFlow(json params) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    doStartFlow(std::move(params));
}

void Station::doStartFlow(json params) {
    // 注意：调用方必须已持有 m_mutex_
    SubmitRequest req;
    req.flowName = m_config_.mainFlowName;
    req.priority = 0;
    req.params   = m_config_.params;
    for (auto& [key, val] : params.items())
        req.params[key] = val;

    m_currentInstanceId_ = m_scheduler_->submit(req);
    LOG_INFO("[Station] {} 启动流程实例: {}", m_config_.stationId, m_currentInstanceId_);
}

void Station::stopFlow() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_continuousMode_.store(false);   // 同时关闭持续模式
    if (!m_currentInstanceId_.empty())
        m_scheduler_->abort(m_currentInstanceId_);
}

void Station::pauseFlow() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    if (!m_currentInstanceId_.empty())
        m_scheduler_->pause(m_currentInstanceId_);
}

void Station::resumeFlow() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    if (!m_currentInstanceId_.empty())
        m_scheduler_->resume(m_currentInstanceId_);
}

bool Station::isRunning() const {
    auto s = flowState();
    return s == FlowState::Running || s == FlowState::Paused;
}

void Station::enableContinuousMode(StationContinuousConfig cfg) {
    std::lock_guard<std::mutex> lk(m_continuousMutex_);
    m_continuousCfg_   = std::move(cfg);
    m_continuousMode_.store(true);
    LOG_INFO("[Station] {} 持续运行模式已启用（间隔 {}ms）",
             m_config_.stationId, m_continuousCfg_.intervalMs);
}

void Station::disableContinuousMode() {
    m_continuousMode_.store(false);
    LOG_INFO("[Station] {} 持续运行模式已关闭", m_config_.stationId);
}

// ── 内部：流程事件处理（持续运行核心逻辑）────────────────────
void Station::onFlowEvent(const SchedulerEvent& e, bool isSuccess) {
    // 过滤：只处理本工位当前实例的事件
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        if (e.instanceId != m_currentInstanceId_) return;
    }

    if (!m_continuousMode_.load()) return;

    // 失败时检查是否要继续
    if (!isSuccess) {
        StationContinuousConfig cfg;
        { std::lock_guard<std::mutex> lk(m_continuousMutex_); cfg = m_continuousCfg_; }
        if (!cfg.resubmitOnFail) {
            LOG_WARN("[Station] {} 流程失败，持续模式已暂停（resubmitOnFail=false）",
                     m_config_.stationId);
            m_continuousMode_.store(false);
            return;
        }
    }

    // 可选间隔等待（模拟来料节拍）
    int interval = 0;
    {
        std::lock_guard<std::mutex> lk(m_continuousMutex_);
        interval = m_continuousCfg_.intervalMs;
    }
    if (interval > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

    // 在持续模式下自动重新提交
    if (m_continuousMode_.load()) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        doStartFlow({});
    }
}

// ============================================================
// StationManager
// ============================================================

StationManager::StationManager(std::shared_ptr<Scheduler> scheduler,
                                 std::shared_ptr<EventBus>  eventBus)
    : m_scheduler_(std::move(scheduler))
    , m_eventBus_(std::move(eventBus))
{}

void StationManager::addStation(StationConfig config) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto id = config.stationId;
    m_stations_[id] = std::make_unique<Station>(
        std::move(config), m_scheduler_, m_eventBus_);
    LOG_INFO("[StationManager] 添加工位: {}", id);
}

void StationManager::removeStation(const std::string& stationId) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_stations_.erase(stationId);
}

Station* StationManager::getStation(const std::string& stationId) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_stations_.find(stationId);
    return it != m_stations_.end() ? it->second.get() : nullptr;
}

std::vector<Station*> StationManager::allStations() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<Station*> result;
    for (auto& [id, st] : m_stations_)
        result.push_back(st.get());
    return result;
}

void StationManager::startAll(json params) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_)
        st->startFlow(params);
}

void StationManager::stopAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_) {
        st->disableContinuousMode();
        st->stopFlow();
    }
}

void StationManager::pauseAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_)
        st->pauseFlow();
}

void StationManager::resumeAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_)
        st->resumeFlow();
}

void StationManager::enableContinuousMode(StationContinuousConfig cfg) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_)
        st->enableContinuousMode(cfg);
    LOG_INFO("[StationManager] 所有工位已开启持续运行模式");
}

void StationManager::disableContinuousMode() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_)
        st->disableContinuousMode();
    LOG_INFO("[StationManager] 所有工位已关闭持续运行模式");
}

// ── 屏障工厂 ────────────────────────────────────────────────

std::shared_ptr<StationBarrier> StationManager::createBarrier(
    const std::string& name, std::vector<std::string> participantIds)
{
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto barrier = std::make_shared<StationBarrier>(name, participantIds);
    m_barriers_[name] = barrier;
    // 同时注册到全局注册表，供 BarrierStep 懒查找
    BarrierRegistry::instance().registerBarrier(name, barrier);
    LOG_INFO("[StationManager] 创建屏障 '{}' ({} 个参与者)", name, participantIds.size());
    return barrier;
}

std::shared_ptr<StationBarrier> StationManager::getBarrier(const std::string& name) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_barriers_.find(name);
    return it != m_barriers_.end() ? it->second : nullptr;
}

// ── 通道工厂 ────────────────────────────────────────────────

std::shared_ptr<PipelineChannel> StationManager::createChannel(
    const std::string& name, int capacity)
{
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto ch = PipelineChannelHub::instance().createChannel(name, capacity);
    m_channels_[name] = ch;
    LOG_INFO("[StationManager] 创建流水线通道 '{}' (capacity={})", name, capacity);
    return ch;
}

std::shared_ptr<PipelineChannel> StationManager::getChannel(const std::string& name) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_channels_.find(name);
    return it != m_channels_.end() ? it->second : nullptr;
}

// ── 状态监控 ────────────────────────────────────────────────

std::map<std::string, FlowState> StationManager::allStationStates() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::map<std::string, FlowState> result;
    for (auto& [id, st] : m_stations_)
        result[id] = st->flowState();
    return result;
}

bool StationManager::allIdle() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, st] : m_stations_) {
        if (st->isRunning()) return false;
    }
    return true;
}

} // namespace scheduler
