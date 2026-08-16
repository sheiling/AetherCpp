#pragma once

#include <string>
#include <vector>
#include <optional>
#include "../engine/checkpoint_snapshot.h"
#include "../core/flow_types.h"

namespace scheduler {

// ============================================================
// ActivityLog — 步骤活动日志
// ============================================================
struct ActivityLog {
    std::string             instanceId;
    std::string             flowName;
    std::string             stepName;
    std::string             stepType;
    StepResult::Status      status;
    int64_t                 durationMs{0};
    int                     errorCode{0};
    std::string             message;
    std::chrono::system_clock::time_point timestamp;
};

// ============================================================
// IFlowPersistence — 持久化接口
// ============================================================
class IFlowPersistence {
public:
    virtual ~IFlowPersistence() = default;

    // ── 流程实例快照 ─────────────────────────────────────────
    virtual void saveInstance(const FlowInstanceSnapshot& snap)                 = 0;
    virtual void updateInstanceState(const std::string& id, FlowState s)        = 0;
    virtual std::vector<FlowInstanceSnapshot> loadAllRunning()            const = 0;

    // ── 检查点 ───────────────────────────────────────────────
    virtual void saveCheckpoint(const CheckpointSnapshot& snap)                 = 0;
    virtual std::optional<CheckpointSnapshot>
                 loadLastCheckpoint(const std::string& instanceId)        const = 0;

    // ── 日志 ─────────────────────────────────────────────────
    virtual void appendActivityLog(ActivityLog log)                             = 0;
    virtual std::vector<ActivityLog>
                 queryActivityLogs(const std::string& instanceId,
                                   int limit = 100)                       const = 0;
};

} // namespace scheduler
