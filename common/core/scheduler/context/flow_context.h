#pragma once

#include <string>
#include <any>
#include <optional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <common/3rdparty/nlohmann/json.hpp>
#include "../core/flow_types.h"
#include "../scheduler_global.h"

namespace scheduler {

// ============================================================
// GlobalVariableStore — 跨流程全局变量（单例，线程安全）
// ============================================================
class SCHEDULER_EXPORT GlobalVariableStore {
public:
    static GlobalVariableStore& instance();

    template<typename T>
    void set(const std::string& key, T&& value) {
        std::unique_lock<std::shared_mutex> lk(m_mutex_);
        m_data_[key] = std::forward<T>(value);
    }

    template<typename T>
    T get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lk(m_mutex_);
        auto it = m_data_.find(key);
        if (it == m_data_.end())
            throw std::runtime_error("[GlobalVariableStore] Key not found: " + key);
        return std::any_cast<T>(it->second);
    }

    template<typename T>
    std::optional<T> tryGet(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lk(m_mutex_);
        auto it = m_data_.find(key);
        if (it == m_data_.end()) return std::nullopt;
        try { return std::any_cast<T>(it->second); }
        catch (...) { return std::nullopt; }
    }

    bool has(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lk(m_mutex_);
        return m_data_.count(key) > 0;
    }

    void remove(const std::string& key) {
        std::unique_lock<std::shared_mutex> lk(m_mutex_);
        m_data_.erase(key);
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lk(m_mutex_);
        m_data_.clear();
    }

private:
    mutable std::shared_mutex                    m_mutex_;
    std::unordered_map<std::string, std::any>    m_data_;
};

// ============================================================
// FlowContext — 三级变量作用域
//   STEP  (栈，步骤退出后自动销毁)
//   FLOW  (流程范围，实例生命周期)
//   GLOBAL(全局单例)
//
// 查找顺序：STEP → FLOW → GLOBAL
// ============================================================
class SCHEDULER_EXPORT FlowContext {
public:
    // ── 写入 ─────────────────────────────────────────────────
    template<typename T>
    void set(const std::string& key, T&& value,
             VariableScope scope = VariableScope::FLOW)
    {
        switch (scope) {
        case VariableScope::STEP:
            if (m_stepScopes_.empty()) m_stepScopes_.emplace_back();
            m_stepScopes_.back()[key] = std::forward<T>(value);
            break;
        case VariableScope::FLOW: {
            std::unique_lock<std::shared_mutex> lk(m_flowMutex_);
            m_flowData_[key] = std::forward<T>(value);
            break;
        }
        case VariableScope::GLOBAL:
            GlobalVariableStore::instance().set(key, std::forward<T>(value));
            break;
        }
    }

    // ── 读取（作用域链查找）──────────────────────────────────
    template<typename T>
    T get(const std::string& key) const {
        // 1. STEP scope（栈顶优先）
        for (auto it = m_stepScopes_.rbegin(); it != m_stepScopes_.rend(); ++it) {
            auto jt = it->find(key);
            if (jt != it->end()) return std::any_cast<T>(jt->second);
        }
        // 2. FLOW scope
        {
            std::shared_lock<std::shared_mutex> lk(m_flowMutex_);
            auto it = m_flowData_.find(key);
            if (it != m_flowData_.end()) return std::any_cast<T>(it->second);
        }
        // 3. GLOBAL scope
        return GlobalVariableStore::instance().get<T>(key);
    }

    template<typename T>
    std::optional<T> tryGet(const std::string& key) const {
        for (auto it = m_stepScopes_.rbegin(); it != m_stepScopes_.rend(); ++it) {
            auto jt = it->find(key);
            if (jt != it->end()) {
                try { return std::any_cast<T>(jt->second); } catch(...) {}
            }
        }
        {
            std::shared_lock<std::shared_mutex> lk(m_flowMutex_);
            auto it = m_flowData_.find(key);
            if (it != m_flowData_.end()) {
                try { return std::any_cast<T>(it->second); } catch(...) {}
            }
        }
        return GlobalVariableStore::instance().tryGet<T>(key);
    }

    bool has(const std::string& key) const;
    void remove(const std::string& key, VariableScope scope = VariableScope::FLOW);

    // ── 步骤作用域生命周期 ─────────────────────────────────────
    void pushStepScope() { m_stepScopes_.emplace_back(); }
    void popStepScope()  { if (!m_stepScopes_.empty()) m_stepScopes_.pop_back(); }

    // ── 序列化/反序列化（用于检查点快照）─────────────────────
    json  serialize()                   const;
    void  deserialize(const json& data);

    // ── 元信息 ────────────────────────────────────────────────
    std::string    flowName;
    std::string    instanceId;
    int            priority{0};
    json           initParams;

    // ── 执行历史 ──────────────────────────────────────────────
    struct StepRecord {
        std::string         name;
        StepResult::Status  status;
        int64_t             durationMs;
        std::string         message;
    };
    std::vector<StepRecord> history;

private:
    std::vector<std::unordered_map<std::string, std::any>> m_stepScopes_;
    mutable std::shared_mutex                               m_flowMutex_;
    std::unordered_map<std::string, std::any>               m_flowData_;
};

} // namespace scheduler
