#ifndef _UTILITY_FAIR_LOCK_HH
#define _UTILITY_FAIR_LOCK_HH

#include <mutex>
#include <queue>
#include <condition_variable>
#include <iostream>

namespace common::regaccess {

class FairLock {
public:
    void lock() {
        std::unique_lock<std::mutex> lk(mtx_);
        if (next_ == UINT64_MAX) {
            // 处理溢出，例如等待 current 追上来
            while (current_ != UINT64_MAX) {
                cond_.wait(lk);
            }
            next_ = 0;
            current_ = 0;
        }

        const uint64_t request = next_++;
        while (request != current_) {
            cond_.wait(lk);
        }
    }

    void unlock() {
        std::unique_lock<std::mutex> lk(mtx_);
        current_++;
        cond_.notify_all();
    }


public:
    class ScopedLock {
    public:
        explicit ScopedLock(FairLock& lock) : lock_(lock) {
            lock_.lock();
        }

        ~ScopedLock() {
            lock_.unlock();
        }

        // 禁止拷贝和移动
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;

    private:
        FairLock& lock_;
    };

private:
    std::mutex mtx_;
    std::condition_variable cond_;
    uint64_t next_ = 0;
    uint64_t current_ = 0;
};

} // namespace common::regaccess

#endif