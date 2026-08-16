#include "compensation_manager.h"
#include "../step/i_step.h"
#include "../context/flow_context.h"
#include <algorithm>

namespace scheduler {

void CompensationManager::recordCompleted(const std::string& stepName,
                                           std::shared_ptr<IStep> compensationStep)
{
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_stack_.emplace_back(stepName, std::move(compensationStep));
}

void CompensationManager::compensateAll(FlowContext& ctx) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    // LIFO：反向执行
    for (auto it = m_stack_.rbegin(); it != m_stack_.rend(); ++it) {
        if (it->second) {
            try {
                it->second->execute(ctx);
            } catch (...) {}
        }
    }
    m_stack_.clear();
}

void CompensationManager::compensateFrom(const std::string& fromStep, FlowContext& ctx) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    // 找到 fromStep 的位置，从那之后反向补偿
    auto it = std::find_if(m_stack_.begin(), m_stack_.end(),
        [&fromStep](const StackEntry& e){ return e.first == fromStep; });
    if (it == m_stack_.end()) return;

    for (auto rit = m_stack_.rbegin(); rit != m_stack_.rend(); ++rit) {
        if (&(*rit) < &(*it)) break;  // 到达 fromStep 之前，停止
        if (rit->second) {
            try { rit->second->execute(ctx); } catch (...) {}
        }
    }
}

void CompensationManager::clear() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_stack_.clear();
}

std::vector<std::string> CompensationManager::getCompletedStepNames() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<std::string> names;
    for (auto& e : m_stack_)
        names.push_back(e.first);
    return names;
}

} // namespace scheduler
