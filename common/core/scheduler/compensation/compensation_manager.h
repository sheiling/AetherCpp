#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "../scheduler_global.h"

namespace scheduler {

class IStep;
class FlowContext;

// ============================================================
// CompensationManager — 补偿事务管理器
//   正向执行时：recordCompleted() 将已完成步骤的补偿步骤入栈
//   流程中止时：compensateAll() 反向执行所有补偿步骤（LIFO）
// ============================================================
class SCHEDULER_EXPORT CompensationManager {
public:
    // 记录步骤完成及对应的补偿步骤
    void recordCompleted(const std::string& stepName,
                          std::shared_ptr<IStep> compensationStep);

    // 反向执行所有补偿步骤
    void compensateAll(FlowContext& ctx);

    // 从某步骤之后的补偿
    void compensateFrom(const std::string& fromStep, FlowContext& ctx);

    // 清空（流程正常完成时调用）
    void clear();

    // 快照/恢复（用于断点续执行）
    std::vector<std::string> getCompletedStepNames() const;

private:
    using StackEntry = std::pair<std::string, std::shared_ptr<IStep>>;
    std::vector<StackEntry> m_stack_;  // LIFO：后完成的步骤先补偿
    mutable std::mutex      m_mutex_;
};

} // namespace scheduler
