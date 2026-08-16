#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <common/3rdparty/nlohmann/json.hpp>

namespace scheduler {

using json = nlohmann::json;

// ============================================================
// CheckpointSnapshot — 断点快照（可序列化为 JSON）
// ============================================================
struct CheckpointSnapshot {
    std::string              instanceId;
    std::string              flowName;
    std::string              checkpointId;       // 检查点步骤名
    json                     contextData;        // FlowContext 序列化
    json                     hsmSnapshot;        // HSM 状态快照
    std::vector<std::string> completedSteps;     // 已完成步骤列表
    std::vector<std::string> compensationStack;  // 补偿栈（已完成步骤）
    std::chrono::system_clock::time_point savedAt;

    json toJson() const {
        json j;
        j["instanceId"]      = instanceId;
        j["flowName"]        = flowName;
        j["checkpointId"]    = checkpointId;
        j["contextData"]     = contextData;
        j["hsmSnapshot"]     = hsmSnapshot;
        j["completedSteps"]  = completedSteps;
        j["compensationStack"] = compensationStack;
        j["savedAt"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            savedAt.time_since_epoch()).count();
        return j;
    }

    static CheckpointSnapshot fromJson(const json& j) {
        CheckpointSnapshot snap;
        snap.instanceId       = j.value("instanceId", "");
        snap.flowName         = j.value("flowName", "");
        snap.checkpointId     = j.value("checkpointId", "");
        snap.contextData      = j.value("contextData", json{});
        snap.hsmSnapshot      = j.value("hsmSnapshot", json{});
        if (j.contains("completedSteps"))
            snap.completedSteps = j["completedSteps"].get<std::vector<std::string>>();
        if (j.contains("compensationStack"))
            snap.compensationStack = j["compensationStack"].get<std::vector<std::string>>();
        if (j.contains("savedAt")) {
            auto ms = j["savedAt"].get<int64_t>();
            snap.savedAt = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(ms));
        }
        return snap;
    }
};

// ============================================================
// FlowInstanceSnapshot — 流程实例完整快照（用于持久化）
// ============================================================
struct FlowInstanceSnapshot {
    std::string instanceId;
    std::string flowName;
    int         state{0};       // FlowState 的整数值
    json        contextData;
    json        hsmSnapshot;
    int         priority{0};
    std::chrono::system_clock::time_point startTime;

    json toJson() const {
        json j;
        j["instanceId"]  = instanceId;
        j["flowName"]    = flowName;
        j["state"]       = state;
        j["contextData"] = contextData;
        j["hsmSnapshot"] = hsmSnapshot;
        j["priority"]    = priority;
        j["startTime"]   = std::chrono::duration_cast<std::chrono::milliseconds>(
            startTime.time_since_epoch()).count();
        return j;
    }
};

} // namespace scheduler
