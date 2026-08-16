#pragma once

#include <string>
#include <any>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <common/3rdparty/nlohmann/json.hpp>

namespace scheduler {

using json = nlohmann::json;

// ============================================================
// FlowState — 流程实例生命周期状态
// ============================================================
enum class FlowState {
    Created,        // 已创建，未提交
    Queued,         // 已入队，等待资源
    Running,        // 运行中
    Paused,         // 已暂停（可恢复）
    AlarmPaused,    // 报警暂停（等待人工处理）
    Completed,      // 正常完成
    Failed,         // 执行失败（未触发中止）
    Aborted,        // 主动中止（已执行补偿）
};

inline const char* flowStateToString(FlowState s) {
    switch (s) {
    case FlowState::Created:     return "Created";
    case FlowState::Queued:      return "Queued";
    case FlowState::Running:     return "Running";
    case FlowState::Paused:      return "Paused";
    case FlowState::AlarmPaused: return "AlarmPaused";
    case FlowState::Completed:   return "Completed";
    case FlowState::Failed:      return "Failed";
    case FlowState::Aborted:     return "Aborted";
    default:                     return "Unknown";
    }
}

// ============================================================
// StepResult — 步骤执行结果
// ============================================================
struct StepResult {
    enum class Status { Success, Failed, Skipped, Timeout };

    Status      status   { Status::Failed };
    std::string message;
    int         errorCode{ 0 };
    std::any    output;

    bool isOk()     const { return status == Status::Success; }
    bool isFailed() const { return status == Status::Failed;  }
    bool isSkipped()const { return status == Status::Skipped; }

    static StepResult ok(std::any out = {}) {
        return {Status::Success, "OK", 0, std::move(out)};
    }
    static StepResult fail(std::string msg, int c = -1) {
        return {Status::Failed, std::move(msg), c};
    }
    static StepResult skip() {
        return {Status::Skipped, "Skipped", 0};
    }
    static StepResult timeout(std::string msg = "Timeout") {
        return {Status::Timeout, std::move(msg), -2};
    }
};

// ============================================================
// VariableScope — 变量作用域
// ============================================================
enum class VariableScope { STEP, FLOW, GLOBAL };

// ============================================================
// FlowInstanceInfo — 运行实例摘要（用于查询/监控）
// ============================================================
struct FlowInstanceInfo {
    std::string instanceId;
    std::string flowName;
    FlowState   state;
    std::string currentStepName;
    std::chrono::system_clock::time_point startTime;
    int         priority{0};
};

// ============================================================
// SchedulerStats — 调度器统计信息
// ============================================================
struct SchedulerStats {
    int runningCount{0};
    int queuedCount{0};
    int completedTotal{0};
    int failedTotal{0};
    int abortedTotal{0};
};

} // namespace scheduler
