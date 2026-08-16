#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <memory>
#include <chrono>
#include <common/3rdparty/nlohmann/json.hpp>

#include "../scheduler_global.h"
#include "../event/event_bus.h"

namespace scheduler {

using json = nlohmann::json;

// ============================================================
// 报警级别（5级）
// ============================================================
enum class AlarmLevel {
    Info,        // L0: 信息，记录日志，不影响流程
    Warning,     // L1: 警告，界面提示，流程继续
    Fault,       // L2: 可恢复故障，暂停当前流程
    Stop,        // L3: 停机故障，停止所有流程
    Emergency,   // L4: 急停级，立即急停
};

// ============================================================
// 报警处置动作
// ============================================================
enum class AlarmAction {
    LogOnly,            // 仅记录日志
    Notify,             // 通知 UI
    PauseFlow,          // 暂停当前流程
    StopAllFlows,       // 停止所有流程（优雅）
    EmergencyStopAll,   // 立即急停
};

// ============================================================
// AlarmInfo — 报警信息
// ============================================================
struct AlarmInfo {
    int         code        {0};
    AlarmLevel  level       {AlarmLevel::Info};
    AlarmAction action      {AlarmAction::LogOnly};
    std::string message;
    std::string instanceId;     // 触发报警的流程实例
    std::string stepName;       // 触发报警的步骤
    std::string operatorId;     // 确认人（可选）
    int         retryCount  {0};
    bool        acknowledged{false};
    std::chrono::system_clock::time_point timestamp;
    json        context;        // 附加诊断信息

    json toJson() const {
        json j;
        j["code"]         = code;
        j["level"]        = static_cast<int>(level);
        j["action"]       = static_cast<int>(action);
        j["message"]      = message;
        j["instanceId"]   = instanceId;
        j["stepName"]     = stepName;
        j["acknowledged"] = acknowledged;
        j["operatorId"]   = operatorId;
        j["context"]      = context;
        return j;
    }
};

inline const char* alarmLevelStr(AlarmLevel l) {
    switch (l) {
    case AlarmLevel::Info:      return "INFO";
    case AlarmLevel::Warning:   return "WARN";
    case AlarmLevel::Fault:     return "FAULT";
    case AlarmLevel::Stop:      return "STOP";
    case AlarmLevel::Emergency: return "EMERGENCY";
    default: return "UNKNOWN";
    }
}

// ============================================================
// AlarmManager — 报警管理器
// ============================================================
class SCHEDULER_EXPORT AlarmManager {
public:
    explicit AlarmManager(std::shared_ptr<EventBus> eventBus = nullptr);

    // ── 规则注册 ─────────────────────────────────────────────
    void registerAlarm(int code, AlarmLevel level, AlarmAction action,
                       std::string message);

    // ── 触发报警 ─────────────────────────────────────────────
    void raise(int code,
               const std::string& instanceId = "",
               const std::string& stepName   = "",
               json ctx = {});

    // ── 确认 / 清除 ──────────────────────────────────────────
    void acknowledge(int code, const std::string& operatorId = "");
    void clear(int code);
    void clearAll();

    // ── 查询 ─────────────────────────────────────────────────
    std::vector<AlarmInfo> activeAlarms() const;
    bool hasActiveAlarm(AlarmLevel minLevel = AlarmLevel::Info) const;

    // ── 外部处置回调（由 MachineFSM 注入）───────────────────
    // scheduler 触发 Stop/Emergency 时调用
    void onActionRequired(std::function<void(AlarmAction, const AlarmInfo&)> cb) {
        m_actionCb_ = std::move(cb);
    }

    // ── 报警通知订阅 ─────────────────────────────────────────
    void onAlarmRaised(std::function<void(const AlarmInfo&)> cb) {
        m_raisedCb_ = std::move(cb);
    }

private:
    struct AlarmRule {
        AlarmLevel  level;
        AlarmAction action;
        std::string message;
    };

    mutable std::mutex m_mutex_;
    std::unordered_map<int, AlarmRule>  m_rules_;
    std::unordered_map<int, AlarmInfo>  m_active_;
    std::shared_ptr<EventBus>           m_eventBus_;

    std::function<void(AlarmAction, const AlarmInfo&)> m_actionCb_;
    std::function<void(const AlarmInfo&)>              m_raisedCb_;
};

} // namespace scheduler
