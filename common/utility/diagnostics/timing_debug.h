#pragma once

/**
 * @file timing_debug.h
 * @brief 通用耗时诊断：由 app.toml [timing_debug] 各开关控制，RAII 输出中文分阶段日志
 *
 * 扩展示例：
 *   [timing_debug]
 *   CaptureTiming = true
 *   MotionTiming  = false
 *
 *   TimingDebugScope scope(TimingDebugKeys::MotionTiming, "运控.轴到位", axisName);
 */

#include <Config.h>
#include <Logger.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>

namespace common::utility {

/** 与 app.toml [timing_debug] 字段名一致 */
struct TimingDebugKeys {
    static constexpr const char *CaptureTiming = "CaptureTiming";
    // static constexpr const char *MotionTiming = "MotionTiming";
};

inline bool isTimingDebugEnabled(const char *categoryKey)
{
    if (categoryKey == nullptr || categoryKey[0] == '\0') {
        return false;
    }
    const std::string configKey = std::string("timing_debug.") + categoryKey;
    return config::ConfigManager::get<bool>(configKey, false);
}

inline void logTimingDebug(const char *stageLabel, const char *detail, int64_t elapsedMs)
{
    if (detail != nullptr && detail[0] != '\0') {
        LOG_INFO("[耗时诊断] {} | {} | {} 毫秒", stageLabel, detail, elapsedMs);
    } else {
        LOG_INFO("[耗时诊断] {} | {} 毫秒", stageLabel, elapsedMs);
    }
}

/** RAII：析构时输出阶段耗时；categoryKey 未开启时不计时 */
class TimingDebugScope
{
public:
    TimingDebugScope(const char *categoryKey, const char *stageLabel, std::string detail = {})
        : stageLabel_(stageLabel)
        , detail_(std::move(detail))
        , enabled_(isTimingDebugEnabled(categoryKey))
    {
        if (enabled_) {
            start_ = std::chrono::steady_clock::now();
        }
    }

    ~TimingDebugScope()
    {
        if (!enabled_) {
            return;
        }
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_);
        logTimingDebug(stageLabel_, detail_.c_str(), elapsed.count());
    }

    TimingDebugScope(const TimingDebugScope &)            = delete;
    TimingDebugScope &operator=(const TimingDebugScope &) = delete;

private:
    const char *stageLabel_;
    std::string detail_;
    bool        enabled_;
    std::chrono::steady_clock::time_point start_{};
};

} // namespace common::utility
