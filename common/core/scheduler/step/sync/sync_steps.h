#pragma once

#include "../i_step.h"
#include "station_barrier.h"
#include <mutex>
#include <chrono>
#include <Logger.h>

namespace scheduler {

// ============================================================
// BarrierStep — 工位屏障同步步骤
//   工位到达后阻塞等待所有参与者就绪，然后一起继续
//
// 支持两种构造方式：
//   1. 直接传入 StationBarrier 指针（代码构建时使用）
//   2. 传入 barrierName + stationId（JSON 加载时使用，运行时懒加载）
// ============================================================
class BarrierStep : public IStep {
public:
    // 方式 1：直接引用屏障对象
    BarrierStep(std::shared_ptr<StationBarrier> barrier,
                std::string stationId)
        : m_barrier_(std::move(barrier))
        , m_stationId_(std::move(stationId))
    {
        m_config_.type = typeName();
        if (m_barrier_) m_barrierName_ = m_barrier_->name();
    }

    // 方式 2：通过名称懒查找（JSON 配置时使用）
    BarrierStep(std::string barrierName, std::string stationId)
        : m_barrierName_(std::move(barrierName))
        , m_stationId_(std::move(stationId))
    {
        m_config_.type = typeName();
    }

    StepResult execute(FlowContext& /*ctx*/) override {
        // 懒加载：尚未持有指针时从 BarrierRegistry 查找
        if (!m_barrier_) {
            m_barrier_ = BarrierRegistry::instance().getBarrier(m_barrierName_);
            if (!m_barrier_) {
                return StepResult::fail("[BarrierStep] 未找到屏障: " + m_barrierName_);
            }
        }

        LOG_INFO("[BarrierStep] 工位 {} 到达屏障: {}", m_stationId_, m_barrier_->name());
        m_barrier_->arrive(m_stationId_);

        int timeout = m_config_.timeoutMs > 0 ? m_config_.timeoutMs : 30000;
        bool ok = m_barrier_->waitAll(timeout);

        if (ok) {
            LOG_INFO("[BarrierStep] 屏障 {} 就绪，继续执行", m_barrier_->name());
            m_barrier_->reset();   // 准备下一次使用（持续模式）
            return StepResult::ok();
        } else {
            LOG_WARN("[BarrierStep] 屏障 {} 等待超时（工位: {}）",
                     m_barrier_->name(), m_stationId_);
            return StepResult::timeout("BarrierStep timeout: " + m_barrier_->name());
        }
    }

    std::string typeName() const override { return "sync.barrier"; }

private:
    std::shared_ptr<StationBarrier> m_barrier_;
    std::string                     m_barrierName_;
    std::string                     m_stationId_;
};

// ============================================================
// WaitStep — 等待固定时间步骤（ms）
// ============================================================
class WaitStep : public IStep {
public:
    explicit WaitStep(int waitMs) : m_waitMs_(waitMs) {
        m_config_.type = typeName();
    }

    StepResult execute(FlowContext& /*ctx*/) override {
        LOG_DEBUG("[WaitStep] 等待 {}ms ...", m_waitMs_);
        std::this_thread::sleep_for(std::chrono::milliseconds(m_waitMs_));
        return StepResult::ok();
    }

    std::string typeName() const override { return "sync.wait"; }

private:
    int m_waitMs_;
};

// ============================================================
// MutexStep — 共享资源互斥步骤（具名互斥锁 + 子步骤）
// ============================================================
class MutexStep : public IStep {
public:
    MutexStep(std::shared_ptr<std::timed_mutex> mtx,
              std::shared_ptr<IStep>            child)
        : m_mtx_(std::move(mtx))
        , m_child_(std::move(child))
    {
        m_config_.type = typeName();
    }

    StepResult execute(FlowContext& ctx) override {
        int timeout = m_config_.timeoutMs > 0 ? m_config_.timeoutMs : 5000;
        LOG_DEBUG("[MutexStep] 申请互斥锁（超时 {}ms）...", timeout);

        if (!m_mtx_->try_lock_for(std::chrono::milliseconds(timeout))) {
            LOG_WARN("[MutexStep] 互斥锁申请超时");
            return StepResult::timeout("MutexStep: lock timeout");
        }

        std::lock_guard<std::timed_mutex> guard(*m_mtx_, std::adopt_lock);
        LOG_DEBUG("[MutexStep] 已获取互斥锁，执行子步骤");
        auto result = m_child_->executeWithRetry(ctx);
        return result;
    }

    std::string typeName() const override { return "sync.mutex"; }

private:
    std::shared_ptr<std::timed_mutex> m_mtx_;
    std::shared_ptr<IStep>            m_child_;
};

} // namespace scheduler
