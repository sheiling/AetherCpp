#pragma once

#include <condition_variable>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>

namespace common
{
namespace utility
{

// 条件变量通用封装（带命名空间+更短命名）
class CVWrap
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_notified = false;

public:
    // 无条件等待
    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_notified; });
        m_notified = false;
    }

    // 带自定义谓词等待
    template <typename Predicate>
    void wait(Predicate predicate)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, predicate);
    }

    // 超时等待（毫秒）；true=被唤醒，false=超时
    bool wait_for(unsigned int timeout_ms)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]() {
            return m_notified;
        });
    }

    // 超时等待（毫秒）+ 条件谓词；true=满足，false=超时
    template <typename Predicate>
    bool wait_for(unsigned int timeout_ms, Predicate predicate)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), predicate);
    }

    // 唤醒一个等待线程
    void notify_one()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_notified = true;
        }
        m_cv.notify_one();
    }

    // 唤醒所有等待线程
    void notify_all()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_notified = true;
        }
        m_cv.notify_all();
    }

    // 重置通知状态
    void reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_notified = false;
    }
};

} // namespace utility
} // namespace common

