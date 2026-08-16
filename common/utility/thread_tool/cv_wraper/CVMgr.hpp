#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "CVWrap.hpp"

namespace common
{
namespace utility
{

/**
 * @brief 条件变量管理器（短命名+带命名空间）
 * @details 根据不同的 KEY 管理对应的条件变量，支持超时等待和自动清理
 */
template <typename KeyType>
class CVMgr
{
public:
    using CVPtr = std::shared_ptr<CVWrap>;
    using KeyList = std::vector<KeyType>;

private:
    mutable std::mutex m_manager_mutex;
    std::unordered_map<KeyType, CVPtr> m_cv_map;

    // 清理超时未使用的条件变量
    struct Info
    {
        CVPtr cv;
        std::chrono::steady_clock::time_point last_used;
        int ref_count {0};
    };

    std::unordered_map<KeyType, Info> m_cv_info_map;
    std::chrono::seconds m_cleanup_timeout {30}; // 30秒清理超时

    // 获取或创建条件变量
    CVPtr getOrCreate(const KeyType& key)
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);

        auto it = m_cv_map.find(key);
        if (it != m_cv_map.end())
        {
            auto& info = m_cv_info_map[key];
            info.last_used = std::chrono::steady_clock::now();
            info.ref_count++;
            return it->second;
        }

        auto cv = std::make_shared<CVWrap>();
        m_cv_map[key] = cv;
        m_cv_info_map[key] = {cv, std::chrono::steady_clock::now(), 1};
        return cv;
    }

    // 释放条件变量的引用
    void release(const KeyType& key)
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);

        auto it = m_cv_info_map.find(key);
        if (it != m_cv_info_map.end())
        {
            it->second.ref_count--;
            if (it->second.ref_count <= 0)
            {
                // 可以清理，但先不立即删除，等待超时清理
                it->second.last_used = std::chrono::steady_clock::now();
            }
        }
    }

public:
    CVMgr() = default;

    static CVMgr<KeyType>& getInstance()
    {
        static CVMgr<KeyType> instance;
        return instance;
    }

    // 禁止拷贝
    CVMgr(const CVMgr&) = delete;
    CVMgr& operator=(const CVMgr&) = delete;

    // 设置清理超时时间
    void set_cleanup_timeout(std::chrono::seconds timeout)
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        m_cleanup_timeout = timeout;
    }

    // 等待特定 KEY 的条件变量
    bool wait(const KeyType& key, unsigned int timeout_ms = 0)
    {
        auto cv = getOrCreate(key);

        bool result = false;
        if (timeout_ms > 0)
        {
            result = cv->wait_for(timeout_ms);
        }
        else
        {
            cv->wait();
            result = true;
        }

        release(key);
        return result;
    }

    // 等待特定 KEY 的条件变量（带自定义条件）
    template <typename Predicate>
    bool wait(const KeyType& key, Predicate predicate, unsigned int timeout_ms = 0)
    {
        auto cv = getOrCreate(key);

        bool result = false;
        if (timeout_ms > 0)
        {
            result = cv->wait_for(timeout_ms, predicate);
        }
        else
        {
            cv->wait(predicate);
            result = true;
        }

        release(key);
        return result;
    }

    // 唤醒特定 KEY 的一个等待线程
    void notify_one(const KeyType& key)
    {
        auto cv = getOrCreate(key);
        if (cv)
        {
            cv->notify_one();
        }
        release(key);
    }

    // 唤醒特定 KEY 的所有等待线程
    void notify_all(const KeyType& key)
    {
        auto cv = getOrCreate(key);
        if (cv)
        {
            cv->notify_all();
        }
        release(key);
    }

    // 唤醒所有 KEY 的所有等待线程
    // @warning 谨慎使用，可能会影响性能
    void notify_all_keys()
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        for (auto& pair : m_cv_map)
        {
            pair.second->notify_all();
        }
    }

    // 删除特定 KEY 的条件变量
    bool remove(const KeyType& key)
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        size_t count = m_cv_map.erase(key);
        m_cv_info_map.erase(key);
        return count > 0;
    }

    // 清理超时未使用的条件变量
    void cleanup()
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        auto now = std::chrono::steady_clock::now();

        auto it = m_cv_info_map.begin();
        while (it != m_cv_info_map.end())
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - it->second.last_used);

            if (it->second.ref_count <= 0 && elapsed > m_cleanup_timeout)
            {
                m_cv_map.erase(it->first);
                it = m_cv_info_map.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // 获取所有 KEY 列表
    KeyList get_all_keys() const
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        KeyList keys;
        keys.reserve(m_cv_map.size());
        for (const auto& pair : m_cv_map)
        {
            keys.push_back(pair.first);
        }
        return keys;
    }

    // 检查 KEY 是否存在
    bool contains(const KeyType& key) const
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        return m_cv_map.find(key) != m_cv_map.end();
    }

    // 获取条件变量数量
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        return m_cv_map.size();
    }

    // 清空所有条件变量
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_manager_mutex);
        m_cv_map.clear();
        m_cv_info_map.clear();
    }
};

} // namespace utility
} // namespace common

