#include "alarm_manager.h"
#include <Logger.h>

namespace scheduler {

AlarmManager::AlarmManager(std::shared_ptr<EventBus> eventBus)
    : m_eventBus_(std::move(eventBus))
{}

void AlarmManager::registerAlarm(int code, AlarmLevel level,
                                   AlarmAction action, std::string message) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_rules_[code] = {level, action, std::move(message)};
}

void AlarmManager::raise(int code,
                          const std::string& instanceId,
                          const std::string& stepName,
                          json ctx) {
    AlarmInfo info;
    AlarmAction action = AlarmAction::LogOnly;
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_rules_.find(code);
        if (it != m_rules_.end()) {
            info.level   = it->second.level;
            info.action  = it->second.action;
            info.message = it->second.message;
            action       = it->second.action;
        } else {
            info.level   = AlarmLevel::Fault;
            info.action  = AlarmAction::PauseFlow;
            info.message = "Unknown alarm code: " + std::to_string(code);
        }
        info.code       = code;
        info.instanceId = instanceId;
        info.stepName   = stepName;
        info.context    = std::move(ctx);
        info.timestamp  = std::chrono::system_clock::now();
        m_active_[code] = info;
    }

    LOG_WARN("[AlarmManager] 报警 [{}][{}]: {} (instance={}, step={})",
             alarmLevelStr(info.level), code, info.message, instanceId, stepName);

    if (m_raisedCb_) m_raisedCb_(info);
    if (m_actionCb_) m_actionCb_(action, info);

    if (m_eventBus_) {
        SchedulerEvent evt;
        evt.type       = SchedulerEventType::AlarmRaised;
        evt.instanceId = instanceId;
        evt.stepName   = stepName;
        evt.payload    = info;
        m_eventBus_->publishAsync(std::move(evt));
    }
}

void AlarmManager::acknowledge(int code, const std::string& operatorId) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_active_.find(code);
    if (it == m_active_.end()) return;
    it->second.acknowledged = true;
    it->second.operatorId   = operatorId;
    LOG_INFO("[AlarmManager] 报警 {} 已确认 by {}", code, operatorId);
}

void AlarmManager::clear(int code) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_active_.erase(code);
    LOG_INFO("[AlarmManager] 报警 {} 已清除", code);
}

void AlarmManager::clearAll() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_active_.clear();
}

std::vector<AlarmInfo> AlarmManager::activeAlarms() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<AlarmInfo> result;
    for (auto& [code, info] : m_active_)
        result.push_back(info);
    return result;
}

bool AlarmManager::hasActiveAlarm(AlarmLevel minLevel) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [code, info] : m_active_)
        if (info.level >= minLevel) return true;
    return false;
}

} // namespace scheduler
