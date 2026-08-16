#ifndef SECS_CHANNEL_HH
#define SECS_CHANNEL_HH

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>
#include <chrono>
#include <cstdint>

namespace SECS
{

template <typename T>
class Channel {
public:
    // 构造函数，设置缓冲区大小
    Channel() : buffer_size_(1) {}
    explicit Channel(size_t buffer_size) : buffer_size_(buffer_size) {}

    // 发送数据到channel
    void send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        // 如果缓冲区已满，等待消费者取走数据
        not_full_.wait(lock, [this]() { return queue_.size() < buffer_size_; });
        queue_.push(value);
        not_empty_.notify_one(); // 通知消费者有新数据
    }

    // 从channel接收数据
    T receive() {
        std::unique_lock<std::mutex> lock(mutex_);
        // 如果缓冲区为空，等待生产者发送数据
        not_empty_.wait(lock, [this]() { return !queue_.empty(); });
        T value = queue_.front();
        queue_.pop();
        not_full_.notify_one(); // 通知生产者有空闲空间
        return value;
    }

    // 从channel接收数据
    std::optional<T> receive_for(uint64_t timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        uint64_t beginTm = getCurrentMs();
        uint64_t originTimeout = timeout;
        while (true)
        {
            if(!not_empty_.wait_for(lock, std::chrono::milliseconds(timeout), [this]() { return !queue_.empty(); })) {
                // 判断是否超时false
                uint64_t currentTm = getCurrentMs();
                // 超时
                if (currentTm - beginTm >= originTimeout) {
                    return std::nullopt;
                }
                timeout -= (currentTm - beginTm);
            } else {
                // 正常唤醒
                break;
            }
        }
             
        T value = queue_.front();
        queue_.pop();
        not_full_.notify_one(); // 通知生产者有空闲空间
        return value;
    }
private:
    uint64_t getCurrentMs() {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto value = now_ms.time_since_epoch().count();
        return value;
    }
private:
    std::queue<T> queue_;                // 数据队列
    size_t buffer_size_;                 // 缓冲区大小
    std::mutex mutex_;                   // 互斥锁
    std::condition_variable not_empty_;  // 用于消费者等待的条件变量
    std::condition_variable not_full_;   // 用于生产者等待的条件变量
};

}

#endif