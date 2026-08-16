#pragma once

#include "i_flow_persistence.h"
#include <string>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include "../scheduler_global.h"

namespace scheduler {

// ============================================================
// JsonFilePersistence — JSON 文件持久化（调试/轻量场景）
//   存储路径：dir/instances.json  dir/checkpoints.json  dir/logs.json
// ============================================================
class SCHEDULER_EXPORT JsonFilePersistence : public IFlowPersistence {
public:
    explicit JsonFilePersistence(std::string dir = "./flow_data");

    void saveInstance(const FlowInstanceSnapshot& snap)              override;
    void updateInstanceState(const std::string& id, FlowState s)    override;
    std::vector<FlowInstanceSnapshot> loadAllRunning()        const  override;

    void saveCheckpoint(const CheckpointSnapshot& snap)              override;
    std::optional<CheckpointSnapshot>
         loadLastCheckpoint(const std::string& instanceId)    const  override;

    void appendActivityLog(ActivityLog log)                          override;
    std::vector<ActivityLog>
         queryActivityLogs(const std::string& instanceId,
                           int limit = 100)                   const  override;

private:
    void ensureDir() const;
    std::string instancesFile()   const;
    std::string checkpointsFile() const;
    std::string logsFile()        const;

    json loadJsonFile(const std::string& path) const;
    void saveJsonFile(const std::string& path, const json& j) const;

    std::string          m_dir_;
    mutable std::mutex   m_mutex_;
};

} // namespace scheduler
