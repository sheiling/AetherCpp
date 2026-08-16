#pragma once

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <atomic>

namespace scheduler {

// ============================================================
// 简单 UUID v4 生成器（无外部依赖）
// ============================================================
inline std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dist;

    uint64_t hi = dist(gen);
    uint64_t lo = dist(gen);

    // 设置版本（v4）和变体位
    hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8)  << (hi >> 32)          << '-'
        << std::setw(4)  << ((hi >> 16) & 0xFFFF) << '-'
        << std::setw(4)  << (hi & 0xFFFF)         << '-'
        << std::setw(4)  << (lo >> 48)            << '-'
        << std::setw(12) << (lo & 0x0000FFFFFFFFFFFFULL);
    return oss.str();
}

// ============================================================
// 简单自增 ID（用于 EventBus 订阅者 ID 等）
// ============================================================
inline int generateSubscriberId() {
    static std::atomic<int> counter{1};
    return counter.fetch_add(1, std::memory_order_relaxed);
}

} // namespace scheduler
