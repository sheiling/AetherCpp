#include "json_file_persistence.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

namespace scheduler {

JsonFilePersistence::JsonFilePersistence(std::string dir) : m_dir_(std::move(dir)) {
    ensureDir();
}

void JsonFilePersistence::ensureDir() const {
    MKDIR(m_dir_.c_str());
}

std::string JsonFilePersistence::instancesFile()   const { return m_dir_ + "/instances.json";   }
std::string JsonFilePersistence::checkpointsFile() const { return m_dir_ + "/checkpoints.json"; }
std::string JsonFilePersistence::logsFile()        const { return m_dir_ + "/logs.json";        }

json JsonFilePersistence::loadJsonFile(const std::string& path) const {
    try {
        std::ifstream f(path);
        if (!f.is_open()) return json::object();
        json j; f >> j; return j;
    } catch (...) { return json::object(); }
}

void JsonFilePersistence::saveJsonFile(const std::string& path, const json& j) const {
    std::ofstream f(path);
    if (f.is_open()) f << j.dump(2);
}

void JsonFilePersistence::saveInstance(const FlowInstanceSnapshot& snap) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(instancesFile());
    j[snap.instanceId] = snap.toJson();
    saveJsonFile(instancesFile(), j);
}

void JsonFilePersistence::updateInstanceState(const std::string& id, FlowState s) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(instancesFile());
    if (j.contains(id)) j[id]["state"] = static_cast<int>(s);
    saveJsonFile(instancesFile(), j);
}

std::vector<FlowInstanceSnapshot> JsonFilePersistence::loadAllRunning() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<FlowInstanceSnapshot> result;
    auto j = loadJsonFile(instancesFile());
    for (auto& [key, val] : j.items()) {
        int state = val.value("state", -1);
        // 只恢复 Running / Paused / AlarmPaused 的实例
        if (state == static_cast<int>(FlowState::Running)    ||
            state == static_cast<int>(FlowState::Paused)     ||
            state == static_cast<int>(FlowState::AlarmPaused))
        {
            FlowInstanceSnapshot snap;
            snap.instanceId  = val.value("instanceId", "");
            snap.flowName    = val.value("flowName",   "");
            snap.state       = state;
            snap.contextData = val.value("contextData", json{});
            snap.hsmSnapshot = val.value("hsmSnapshot", json{});
            snap.priority    = val.value("priority",    0);
            result.push_back(snap);
        }
    }
    return result;
}

void JsonFilePersistence::saveCheckpoint(const CheckpointSnapshot& snap) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(checkpointsFile());
    j[snap.instanceId] = snap.toJson();
    saveJsonFile(checkpointsFile(), j);
}

std::optional<CheckpointSnapshot>
JsonFilePersistence::loadLastCheckpoint(const std::string& instanceId) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(checkpointsFile());
    if (!j.contains(instanceId)) return std::nullopt;
    try {
        return CheckpointSnapshot::fromJson(j[instanceId]);
    } catch (...) { return std::nullopt; }
}

void JsonFilePersistence::appendActivityLog(ActivityLog log) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(logsFile());
    auto& arr = j[log.instanceId];
    if (!arr.is_array()) arr = json::array();
    json entry;
    entry["stepName"]   = log.stepName;
    entry["flowName"]   = log.flowName;
    entry["status"]     = static_cast<int>(log.status);
    entry["durationMs"] = log.durationMs;
    entry["errorCode"]  = log.errorCode;
    entry["message"]    = log.message;
    arr.push_back(entry);
    saveJsonFile(logsFile(), j);
}

std::vector<ActivityLog>
JsonFilePersistence::queryActivityLogs(const std::string& instanceId, int limit) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto j = loadJsonFile(logsFile());
    std::vector<ActivityLog> result;
    if (!j.contains(instanceId)) return result;
    auto& arr = j[instanceId];
    int start = (int)arr.size() - limit;
    if (start < 0) start = 0;
    for (int i = start; i < (int)arr.size(); ++i) {
        ActivityLog log;
        log.instanceId = instanceId;
        log.stepName   = arr[i].value("stepName",   "");
        log.flowName   = arr[i].value("flowName",   "");
        log.status     = static_cast<StepResult::Status>(arr[i].value("status", 1));
        log.durationMs = arr[i].value("durationMs", 0LL);
        log.errorCode  = arr[i].value("errorCode",  0);
        log.message    = arr[i].value("message",    "");
        result.push_back(log);
    }
    return result;
}

} // namespace scheduler
