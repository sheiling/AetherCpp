#include "scheduler.h"
#include <algorithm>
#include <Logger.h>

namespace scheduler {

// ============================================================
// ResourceLocker
// ============================================================
bool ResourceLocker::tryAcquireAll(const std::string& instanceId,
                                    std::vector<std::string> resources,
                                    int timeoutMs) {
    // 固定排序防死锁
    std::sort(resources.begin(), resources.end());
    resources.erase(std::unique(resources.begin(), resources.end()), resources.end());

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    std::unique_lock<std::mutex> lk(m_mutex_);

    for (const auto& res : resources) {
        auto ok = m_cv_.wait_until(lk, deadline, [this, &res]{
            return m_locks_.count(res) == 0;
        });
        if (!ok) {
            // 超时，释放已申请的资源
            for (const auto& r2 : resources) {
                auto it = m_locks_.find(r2);
                if (it != m_locks_.end() && it->second == instanceId)
                    m_locks_.erase(it);
                else break;
            }
            return false;
        }
        m_locks_[res] = instanceId;
    }
    return true;
}

void ResourceLocker::releaseAll(const std::string& instanceId) {
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        for (auto it = m_locks_.begin(); it != m_locks_.end(); ) {
            if (it->second == instanceId) it = m_locks_.erase(it);
            else ++it;
        }
    }
    m_cv_.notify_all();
}

std::optional<std::string> ResourceLocker::holder(const std::string& resource) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_locks_.find(resource);
    if (it == m_locks_.end()) return std::nullopt;
    return it->second;
}

bool ResourceLocker::isAvailable(const std::string& resource) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    return m_locks_.count(resource) == 0;
}

// ============================================================
// PriorityFIFOStrategy
// ============================================================
std::optional<PendingRequest> PriorityFIFOStrategy::selectNext(
    const std::vector<PendingRequest>& waitQueue,
    const std::unordered_map<std::string, std::shared_ptr<FlowInstance>>&) const
{
    if (waitQueue.empty()) return std::nullopt;
    // 找最高优先级的最早提交请求
    auto it = std::max_element(waitQueue.begin(), waitQueue.end(),
        [](const PendingRequest& a, const PendingRequest& b) {
            if (a.req.priority != b.req.priority)
                return a.req.priority < b.req.priority;
            return a.submitTime > b.submitTime;   // 更早的排前
        });
    return *it;
}

// ============================================================
// Scheduler
// ============================================================
Scheduler::Scheduler(std::shared_ptr<EventBus> eventBus)
    : m_eventBus_(std::move(eventBus))
    , m_resourceLocker_(std::make_unique<ResourceLocker>())
    , m_strategy_(std::make_shared<PriorityFIFOStrategy>())
{
    m_scheduleThread_ = std::thread(&Scheduler::scheduleLoop, this);
}

Scheduler::~Scheduler() {
    m_schedulerRunning_.store(false);
    m_cv_.notify_all();
    if (m_scheduleThread_.joinable()) m_scheduleThread_.join();
}

void Scheduler::registerFlow(std::shared_ptr<FlowDefinition> def) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_registry_[def->name] = std::move(def);
}

bool Scheduler::hasFlow(const std::string& name) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    return m_registry_.count(name) > 0;
}

std::string Scheduler::submit(SubmitRequest req) {
    if (req.requestId.empty()) req.requestId = generateUUID();

    if (!m_acceptingRequests_.load()) {
        LOG_WARN("[Scheduler] 调度器已停止接受新请求，忽略: {}", req.flowName);
        return "";
    }

    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        if (m_registry_.count(req.flowName) == 0) {
            LOG_WARN("[Scheduler] Unknown flow: {}", req.flowName);
            return "";
        }
        PendingRequest pending;
        pending.req        = req;
        pending.submitTime = std::chrono::steady_clock::now();
        m_waitQueue_.push_back(std::move(pending));
    }
    m_cv_.notify_one();

    if (m_eventBus_)
        m_eventBus_->publishAsync({SchedulerEventType::FlowSubmitted, req.requestId, req.flowName});

    return req.requestId;
}

void Scheduler::pause(const std::string& instanceId) {
    auto inst = getRunningInstance(instanceId);
    if (inst) inst->pause();
}

void Scheduler::resume(const std::string& instanceId) {
    auto inst = getRunningInstance(instanceId);
    if (inst) inst->resume();
}

void Scheduler::abort(const std::string& instanceId) {
    auto inst = getRunningInstance(instanceId);
    if (inst) inst->abort();
}

void Scheduler::handleRecovery(const RecoveryRequest& req) {
    auto inst = getRunningInstance(req.instanceId);
    if (!inst) return;

    switch (req.mode) {
    case RecoveryMode::RetryCurrentStep:
        inst->resume();
        break;
    case RecoveryMode::SkipCurrentStep:
        inst->skipCurrentStep();
        inst->resume();
        break;
    case RecoveryMode::ResumeFromStep:
        inst->resumeFromStep(req.targetStepName);
        inst->resume();
        break;
    case RecoveryMode::ResumeFromCheckpoint:
        if (m_persistence_) {
            auto snap = m_persistence_->loadLastCheckpoint(req.instanceId);
            if (snap) inst->restoreFromCheckpoint(*snap);
            inst->resume();
        }
        break;
    case RecoveryMode::AbortFlow:
        inst->abort();
        break;
    }
}

void Scheduler::pauseAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, inst] : m_running_) {
        if (inst->state() == FlowState::Running)
            inst->pause();
    }
    LOG_INFO("[Scheduler] 所有运行中的流程已暂停");
}

void Scheduler::resumeAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [id, inst] : m_running_) {
        if (inst->state() == FlowState::Paused)
            inst->resume();
    }
    LOG_INFO("[Scheduler] 所有已暂停的流程已恢复");
}

bool Scheduler::isIdle() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    return m_running_.empty() && m_waitQueue_.empty();
}

void Scheduler::stopAll() {
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_acceptingRequests_.store(false);
        // 优雅停止：让当前步骤执行完毕后暂停，不再接受新流程
        for (auto& [id, inst] : m_running_) {
            if (inst->state() == FlowState::Running)
                inst->pause();
        }
        m_waitQueue_.clear();
    }
    LOG_INFO("[Scheduler] SafeStop: 已停止接受新流程，等待当前流程完成");

    // 若已经空闲，立即触发回调
    if (isIdle() && m_onIdleCb_) m_onIdleCb_();
}

void Scheduler::emergencyStop() {
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_acceptingRequests_.store(false);
        for (auto& [id, inst] : m_running_)
            inst->abort();
        m_waitQueue_.clear();
    }
    if (m_eventBus_)
        m_eventBus_->publishAsync({SchedulerEventType::EmergencyStop, "", ""});
    LOG_WARN("[Scheduler] EmergencyStop: all flows aborted");
}

std::vector<FlowInstanceInfo> Scheduler::runningInstances() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<FlowInstanceInfo> result;
    for (auto& [id, inst] : m_running_) {
        FlowInstanceInfo info;
        info.instanceId      = inst->instanceId();
        info.flowName        = inst->context().flowName;
        info.state           = inst->state();
        info.currentStepName = inst->currentStepName();
        info.priority        = inst->context().priority;
        result.push_back(info);
    }
    return result;
}

FlowInstanceInfo Scheduler::getInstanceInfo(const std::string& id) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_running_.find(id);
    if (it == m_running_.end()) return {};
    auto& inst = it->second;
    FlowInstanceInfo info;
    info.instanceId      = inst->instanceId();
    info.flowName        = inst->context().flowName;
    info.state           = inst->state();
    info.currentStepName = inst->currentStepName();
    info.priority        = inst->context().priority;
    return info;
}

SchedulerStats Scheduler::stats() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    SchedulerStats s;
    s.runningCount   = static_cast<int>(m_running_.size());
    s.queuedCount    = static_cast<int>(m_waitQueue_.size());
    s.completedTotal = m_completedTotal_.load();
    s.failedTotal    = m_failedTotal_.load();
    s.abortedTotal   = m_abortedTotal_.load();
    return s;
}

void Scheduler::recoverFromPersistence() {
    if (!m_persistence_) return;
    auto snapshots = m_persistence_->loadAllRunning();
    LOG_INFO("[Scheduler] 发现 {} 个待恢复流程实例", snapshots.size());

    for (auto& snap : snapshots) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto defIt = m_registry_.find(snap.flowName);
        if (defIt == m_registry_.end()) {
            LOG_WARN("[Scheduler] 流程定义不存在，无法恢复: {}", snap.flowName);
            continue;
        }
        auto instance = std::make_shared<FlowInstance>(defIt->second, m_eventBus_);
        auto checkpoint = m_persistence_->loadLastCheckpoint(snap.instanceId);
        if (checkpoint) {
            instance->restoreFromCheckpoint(*checkpoint);
            LOG_INFO("[Scheduler] 实例 {} 从检查点 {} 恢复",
                snap.instanceId, checkpoint->checkpointId);
        }
        // 恢复路径未经过 tryScheduleNext 加锁，resourceOwnerId 为空即可
        instance->onCompleted([this, id = snap.instanceId](bool success) {
            onInstanceCompleted(id, "", success);
        });
        m_running_[snap.instanceId] = instance;
        if (m_eventBus_)
            m_eventBus_->publishAsync({SchedulerEventType::FlowRecovered,
                                       snap.instanceId, snap.flowName});
    }
}

// ── 私有方法 ──────────────────────────────────────────────────

void Scheduler::scheduleLoop() {
    while (m_schedulerRunning_.load()) {
        try {
            std::unique_lock<std::mutex> lk(m_mutex_);
            m_cv_.wait_for(lk, std::chrono::milliseconds(50), [this]{
                return !m_waitQueue_.empty() || !m_schedulerRunning_.load();
            });
            if (!m_schedulerRunning_.load()) break;
            if (!m_waitQueue_.empty())
                tryScheduleNext();
        } catch (const std::exception& ex) {
            LOG_ERROR("[Scheduler] scheduleLoop exception: {}", ex.what());
        } catch (...) {
            LOG_ERROR("[Scheduler] scheduleLoop unknown exception");
        }
    }
}

void Scheduler::tryScheduleNext() {
    // 检查并发上限
    if ((int)m_running_.size() >= m_maxConcurrent_) return;

    auto selected = m_strategy_->selectNext(m_waitQueue_, m_running_);
    if (!selected) return;

    // 从队列中移除
    m_waitQueue_.erase(
        std::remove_if(m_waitQueue_.begin(), m_waitQueue_.end(),
            [&selected](const PendingRequest& p){
                return p.req.requestId == selected->req.requestId;
            }),
        m_waitQueue_.end());

    auto defIt = m_registry_.find(selected->req.flowName);
    if (defIt == m_registry_.end()) return;
    auto def = defIt->second;

    // 检查实例上限
    int runningCount = 0;
    for (auto& [id, inst] : m_running_)
        if (inst->context().flowName == def->name) ++runningCount;
    if (runningCount >= def->maxInstances) {
        // 放回队列尾
        m_waitQueue_.push_back(*selected);
        return;
    }

    // 申请资源：holder 使用 submit 返回的 requestId，完成时按同一 ID 释放
    const std::string& requestId = selected->req.requestId;
    if (!def->requiredDevices.empty()) {
        if (!m_resourceLocker_->tryAcquireAll(requestId, def->requiredDevices, 100)) {
            m_waitQueue_.push_back(*selected);   // 资源不可用，等待下次
            return;
        }
    }

    // 创建流程实例
    auto instance = std::make_shared<FlowInstance>(def, m_eventBus_);
    if (m_persistence_) instance->setPersistence(m_persistence_);
    instance->context().initParams = selected->req.params;

    auto instanceId = instance->instanceId();

    instance->onCompleted([this, instanceId, requestId](bool success) {
        onInstanceCompleted(instanceId, requestId, success);
    });

    m_running_[instanceId] = instance;
    LOG_INFO("[Scheduler] 启动流程实例: {} (flow={})", instanceId, def->name);
    instance->start();

    // 持久化保存实例信息
    if (m_persistence_) {
        FlowInstanceSnapshot snap;
        snap.instanceId = instanceId;
        snap.flowName   = def->name;
        snap.state      = static_cast<int>(FlowState::Running);
        snap.priority   = selected->req.priority;
        snap.startTime  = std::chrono::system_clock::now();
        m_persistence_->saveInstance(snap);
    }
}

void Scheduler::onInstanceCompleted(const std::string& instanceId,
                                    const std::string& resourceOwnerId,
                                    bool success) {
    std::function<void()> idleCb;
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_running_.find(instanceId);
        if (it == m_running_.end()) return;

        auto state = it->second->state();
        if (!resourceOwnerId.empty())
            m_resourceLocker_->releaseAll(resourceOwnerId);
        m_running_.erase(it);

        if (state == FlowState::Completed)      ++m_completedTotal_;
        else if (state == FlowState::Failed)    ++m_failedTotal_;
        else if (state == FlowState::Aborted)   ++m_abortedTotal_;

        if (m_persistence_)
            m_persistence_->updateInstanceState(instanceId, state);

        LOG_INFO("[Scheduler] 实例 {} 完成 (state={})", instanceId,
                 static_cast<int>(state));

        // 不再接受新请求 且 全部运行实例已清空 → 触发空闲回调
        if (!m_acceptingRequests_.load() && m_running_.empty() && m_waitQueue_.empty()) {
            idleCb = m_onIdleCb_;
        }
    }
    m_cv_.notify_one();  // 唤醒调度循环，尝试调度下一个
    if (idleCb) idleCb();
    (void)success;
}

std::shared_ptr<FlowInstance> Scheduler::getRunningInstance(
    const std::string& instanceId) const
{
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_running_.find(instanceId);
    if (it == m_running_.end()) return nullptr;
    return it->second;
}

} // namespace scheduler
