#pragma once

#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <chrono>
#include <unordered_map>
#include <memory>
#include "../../scheduler_global.h"

namespace scheduler {

// ============================================================
// StationBarrier — 多工位屏障同步原语
//   所有参与工位调用 arrive() 后一起继续
// ============================================================
class SCHEDULER_EXPORT StationBarrier {
public:
    StationBarrier(std::string name, std::vector<std::string> participantIds)
        : m_name_(std::move(name))
        , m_participants_(participantIds.begin(), participantIds.end())
    {}

    const std::string& name() const { return m_name_; }

    void arrive(const std::string& stationId) {
        {
            std::lock_guard<std::mutex> lk(m_mutex_);
            if (m_participants_.count(stationId) == 0)
                throw std::runtime_error("[Barrier] Unknown station: " + stationId);
            m_arrived_.insert(stationId);
        }
        m_cv_.notify_all();
    }

    // 阻塞直到所有工位到达，或超时
    bool waitAll(int timeoutMs = 30000) {
        std::unique_lock<std::mutex> lk(m_mutex_);
        auto pred = [this]{ return m_arrived_.size() >= m_participants_.size(); };
        if (timeoutMs > 0)
            return m_cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs), pred);
        m_cv_.wait(lk, pred);
        return true;
    }

    void reset() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_arrived_.clear();
    }

    int arrivedCount() const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return static_cast<int>(m_arrived_.size());
    }

private:
    std::string           m_name_;
    std::set<std::string> m_participants_;
    std::set<std::string> m_arrived_;
    mutable std::mutex    m_mutex_;
    std::condition_variable m_cv_;
};

// ============================================================
// BarrierRegistry — 全局屏障注册中心（单例）
//   LineLoader 创建屏障时注册，BarrierStep 从此处懒查找
// ============================================================
class SCHEDULER_EXPORT BarrierRegistry {
public:
    static BarrierRegistry& instance() {
        static BarrierRegistry inst;
        return inst;
    }

    void registerBarrier(const std::string& name,
                         std::shared_ptr<StationBarrier> barrier) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_barriers_[name] = std::move(barrier);
    }

    std::shared_ptr<StationBarrier> getBarrier(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_barriers_.find(name);
        return it != m_barriers_.end() ? it->second : nullptr;
    }

    bool hasBarrier(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return m_barriers_.count(name) > 0;
    }

    void remove(const std::string& name) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_barriers_.erase(name);
    }

    void clearAll() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_barriers_.clear();
    }

private:
    BarrierRegistry() = default;
    mutable std::mutex m_mutex_;
    std::unordered_map<std::string, std::shared_ptr<StationBarrier>> m_barriers_;
};

// ============================================================
// StationSignalBus — 工位间信号总线（单例）
// ============================================================
class SCHEDULER_EXPORT StationSignalBus {
public:
    static StationSignalBus& instance() {
        static StationSignalBus inst;
        return inst;
    }

    // 发送信号
    void signal(const std::string& name, const std::string& fromStation = "") {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_signals_[name] = true;
        m_from_[name]    = fromStation;
        m_cv_.notify_all();
    }

    // 等待信号（阻塞，超时返回 false）
    bool waitSignal(const std::string& name, int timeoutMs = 30000) {
        std::unique_lock<std::mutex> lk(m_mutex_);
        auto pred = [this, &name]{ return m_signals_.count(name) > 0 && m_signals_[name]; };
        if (timeoutMs > 0)
            return m_cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs), pred);
        m_cv_.wait(lk, pred);
        return true;
    }

    // 清除信号（消费后清除）
    void clear(const std::string& name) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_signals_.erase(name);
        m_from_.erase(name);
    }

    bool hasSignal(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_signals_.find(name);
        return it != m_signals_.end() && it->second;
    }

private:
    StationSignalBus() = default;
    mutable std::mutex m_mutex_;
    std::condition_variable m_cv_;
    std::unordered_map<std::string, bool>        m_signals_;
    std::unordered_map<std::string, std::string> m_from_;
};

} // namespace scheduler
