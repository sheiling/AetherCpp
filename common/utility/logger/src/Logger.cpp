/**
 * @file   Logger.cpp
 * @brief  框架日志模块实现
 *
 * 内部包含自定义 monthly_hybrid_file_sink，用于实现：
 *   "按日期切割 + 按文件大小切割 + 月份级别目录" 的混合轮转策略。
 */

#include "Logger.h"

// ── spdlog 核心头文件 ──────────────────────────────────────────────────────────
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>

// ── 标准库 ────────────────────────────────────────────────────────────────────
#include <atomic>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace logger {

// =============================================================================
//  monthly_hybrid_file_sink
//
//  混合轮转 Sink：月份目录 + 按日期切割 + 按文件大小切割
//
//  文件路径规则：
//    index == 0 : {baseDir}/{YYYY}/{MM}/{appName}_{YYYY-MM-DD}.log
//    index  > 0 : {baseDir}/{YYYY}/{MM}/{appName}_{YYYY-MM-DD}.{N}.log
//
//  轮转触发时机：
//    1. 日志消息的本地日期与当前记录日期不同 → 日期轮转（index 清零，新文件）
//    2. 写入后文件体积超过 maxSize         → 大小轮转（index 递增，新文件）
// =============================================================================
template <typename Mutex>
class monthly_hybrid_file_sink final : public spdlog::sinks::base_sink<Mutex> {
public:
    /**
     * @param base_dir   日志根目录（YYYY/MM 子目录由 Sink 自动创建）
     * @param app_name   应用名，用于生成文件名
     * @param max_size   单文件最大字节数（0 = 不限制）
     * @param max_files  同日期内最多保留的文件数量（0 = 不限制）
     */
    monthly_hybrid_file_sink(std::string base_dir,
                              std::string app_name,
                              std::size_t max_size,
                              std::size_t max_files)
        : base_dir_(std::move(base_dir))
        , app_name_(std::move(app_name))
        , max_size_(max_size)
        , max_files_(max_files)
    {
        tm t = local_time_now();
        cur_year_  = t.tm_year + 1900;
        cur_month_ = t.tm_mon  + 1;
        cur_day_   = t.tm_mday;
        // 启动时跳过已存在的同名文件，避免覆盖历史日志
        cur_index_ = find_next_index(t);
        file_helper_.open(build_filepath(t, cur_index_), false);
    }

protected:
    // sink_it_ 在父类 log() 持锁后被调用，无需二次加锁
    void sink_it_(const spdlog::details::log_msg& msg) override {
        time_t tt = spdlog::log_clock::to_time_t(msg.time);
        tm t      = local_time(tt);

        // ── 日期轮转 ──────────────────────────────────────────────────────────
        if (t.tm_mday != cur_day_ || t.tm_mon + 1 != cur_month_ ||
            t.tm_year + 1900 != cur_year_) {
            cur_year_  = t.tm_year + 1900;
            cur_month_ = t.tm_mon  + 1;
            cur_day_   = t.tm_mday;
            cur_index_ = 0;
            file_helper_.open(build_filepath(t, cur_index_), false);
        }

        // 格式化消息
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        // ── 大小轮转（写入前检查，防止单条大消息导致超限） ──────────────────
        if (max_size_ > 0 && file_helper_.size() + formatted.size() > max_size_) {
            ++cur_index_;
            file_helper_.open(build_filepath(t, cur_index_), false);
            // 删除超出保留数量的旧文件
            if (max_files_ > 0) {
                int old_idx = static_cast<int>(cur_index_) - static_cast<int>(max_files_);
                if (old_idx >= 0) {
                    std::string old_path = build_filepath(t, old_idx);
                    spdlog::details::os::remove_if_exists(old_path);
                }
            }
        }

        file_helper_.write(formatted);
    }

    void flush_() override { file_helper_.flush(); }

private:
    // 构造带年月合并目录和日期索引的文件路径
    // 目录格式：{base_dir}/{YYYYMM}/
    // 文件格式：{appName}_{YYYY-MM-DD}[.N].log
    std::string build_filepath(const tm& t, int idx) const {
        char buf[512];
        if (idx == 0) {
            std::snprintf(buf, sizeof(buf),
                "%s/%04d%02d/%s_%04d-%02d-%02d.log",
                base_dir_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1,
                app_name_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        } else {
            std::snprintf(buf, sizeof(buf),
                "%s/%04d%02d/%s_%04d-%02d-%02d.%d.log",
                base_dir_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1,
                app_name_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, idx);
        }
        return buf;
    }

    // 查找今天下一个空闲的文件索引（保证启动时不覆盖已有日志）
    int find_next_index(const tm& t) const {
        for (int i = 0; i < 10000; ++i) {
            if (!spdlog::details::os::path_exists(build_filepath(t, i))) {
                return i;
            }
        }
        return 0;
    }

    static tm local_time_now() {
        auto now  = spdlog::log_clock::now();
        time_t tt = spdlog::log_clock::to_time_t(now);
        return local_time(tt);
    }

    static tm local_time(time_t tt) {
        tm t{};
#ifdef _WIN32
        localtime_s(&t, &tt);
#else
        localtime_r(&tt, &t);
#endif
        return t;
    }

    std::string base_dir_;
    std::string app_name_;
    std::size_t max_size_;
    std::size_t max_files_;

    int cur_year_  = -1;
    int cur_month_ = -1;
    int cur_day_   = -1;
    int cur_index_ =  0;

    spdlog::details::file_helper file_helper_;
};

using monthly_hybrid_file_sink_mt = monthly_hybrid_file_sink<std::mutex>;
using monthly_hybrid_file_sink_st = monthly_hybrid_file_sink<spdlog::details::null_mutex>;

// =============================================================================
//  Logger 内部状态（文件作用域）
// =============================================================================
namespace {

std::shared_ptr<spdlog::logger> s_logger;
std::atomic<bool>               s_initialized{false};
std::mutex                      s_init_mutex;

// 持久化双路 Sink（供 setOutputTarget 动态切换使用）
spdlog::sink_ptr s_console_sink;
spdlog::sink_ptr s_file_sink;

// ── 工具函数：LogLevel 转换为 spdlog::level ──────────────────────────────────
spdlog::level::level_enum to_spdlog_level(LogLevel lv) noexcept {
    switch (lv) {
        case LogLevel::Trace:    return spdlog::level::trace;
        case LogLevel::Debug:    return spdlog::level::debug;
        case LogLevel::Info:     return spdlog::level::info;
        case LogLevel::Warn:     return spdlog::level::warn;
        case LogLevel::Error:    return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        case LogLevel::Off:      return spdlog::level::off;
        default:                 return spdlog::level::info;
    }
}

// ── 工具函数：生成当前月份目录路径 ───────────────────────────────────────────
std::string current_month_dir(const std::string& base_dir) {
    auto  now = spdlog::log_clock::now();
    time_t tt = spdlog::log_clock::to_time_t(now);
    tm t{};
#ifdef _WIN32
    localtime_s(&t, &tt);
#else
    localtime_r(&tt, &t);
#endif
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s/%04d%02d",
        base_dir.c_str(), t.tm_year + 1900, t.tm_mon + 1);
    return buf;
}

} // anonymous namespace

// =============================================================================
//  Logger::init
// =============================================================================
bool Logger::init(const LogConfig& cfg) {
    std::lock_guard<std::mutex> guard(s_init_mutex);
    if (s_initialized.load(std::memory_order_acquire)) {
        return true;  // 幂等：重复调用直接返回
    }

    try {
        // ── 始终构建控制台 Sink ────────────────────────────────────────────────
        s_console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(
            cfg.enableColor ? spdlog::color_mode::automatic
                            : spdlog::color_mode::never);

        // ── 始终构建文件 Sink ──────────────────────────────────────────────────
        // 两路 Sink 均保留引用，供 setOutputTarget() 运行时动态切换使用。
        switch (cfg.rotateStrategy) {

            // ── None：单一文件，月份目录 ──────────────────────────────────────
            case RotateStrategy::None: {
                std::string path = current_month_dir(cfg.logDir)
                                   + "/" + cfg.appName + ".log";
                s_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, false);
                break;
            }

            // ── Daily：按日期切割，年月合并目录 ──────────────────────────────
            // 路径模板（strftime 格式）：
            //   {logDir}/%Y%m/{appName}_%Y-%m-%d.log
            // 示例：logs/202603/app_2026-03-12.log
            case RotateStrategy::Daily: {
                std::string tpl = cfg.logDir + "/%Y%m/"
                                  + cfg.appName + "_%Y-%m-%d.log";
                s_file_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(
                    tpl,
                    cfg.rotationHour,
                    cfg.rotationMinute,
                    false,
                    cfg.maxDailyFiles);
                break;
            }

            // ── Size：按文件大小切割，月份目录 ───────────────────────────────
            case RotateStrategy::Size: {
                std::string path = current_month_dir(cfg.logDir)
                                   + "/" + cfg.appName + ".log";
                s_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    path,
                    cfg.maxFileSize,
                    cfg.maxRotatedFiles);
                break;
            }

            // ── Both：日期 + 大小混合切割（自定义 Sink） ─────────────────────
            case RotateStrategy::Both: {
                s_file_sink = std::make_shared<monthly_hybrid_file_sink_mt>(
                    cfg.logDir,
                    cfg.appName,
                    cfg.maxFileSize,
                    cfg.maxRotatedFiles);
                break;
            }
        }

        // ── 按初始 OutputTarget 决定活跃 Sink 列表 ───────────────────────────
        std::vector<spdlog::sink_ptr> sinks;
        switch (cfg.target) {
            case OutputTarget::Console:
                sinks.push_back(s_console_sink);
                break;
            case OutputTarget::File:
                sinks.push_back(s_file_sink);
                break;
            case OutputTarget::Both:
            default:
                sinks.push_back(s_console_sink);
                sinks.push_back(s_file_sink);
                break;
        }

        // ── 创建并配置 Logger ─────────────────────────────────────────────────
        s_logger = std::make_shared<spdlog::logger>(
            cfg.appName, sinks.begin(), sinks.end());

        s_logger->set_level(to_spdlog_level(cfg.level));

        // ── 设置输出格式 ──────────────────────────────────────────────────────
        // 默认格式包含：时间戳 | 级别 | 线程ID | 源文件:行号 | 函数名 | 消息体
        // 格式说明：
        //   %Y-%m-%d %H:%M:%S.%e   时间戳（精确到毫秒）
        //   %^%-8l%$               级别（左对齐8字符，颜色区域）
        //   %t                     线程 ID
        //   %s:%#                  源文件名:行号（由 LOG_* 宏提供）
        //   %!                     函数名（由 LOG_* 宏提供）
        //   %v                     日志消息体
        const std::string& fmt = cfg.pattern.empty()
            ? std::string("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] [tid:%-5t] [%s:%# %!()] %v")
            : cfg.pattern;
        s_logger->set_pattern(fmt);

        // 刷盘策略：Buffered=仅 Error+ 刷盘；Sync=Info+ 每条立即刷盘（app.toml logger.flushMode）
        switch (cfg.flushMode) {
        case FlushMode::Sync:
            s_logger->flush_on(spdlog::level::info);
            break;
        case FlushMode::Buffered:
        default:
            s_logger->flush_on(spdlog::level::err);
            break;
        }

        // 注册到 spdlog 全局注册表
        spdlog::register_logger(s_logger);

        s_initialized.store(true, std::memory_order_release);
        return true;

    } catch (const spdlog::spdlog_ex& ex) {
        std::fprintf(stderr, "[Logger] init failed (spdlog_ex): %s\n", ex.what());
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "[Logger] init failed: %s\n", ex.what());
    }
    return false;
}

// =============================================================================
//  Logger 其他接口实现
// =============================================================================
void Logger::shutdown() {
    std::lock_guard<std::mutex> guard(s_init_mutex);
    if (s_logger) {
        s_logger->flush();
        spdlog::drop(s_logger->name());
        s_logger.reset();
    }
    s_console_sink.reset();
    s_file_sink.reset();
    s_initialized.store(false, std::memory_order_release);
}

std::shared_ptr<spdlog::logger> Logger::get() {
    return s_logger;
}

void Logger::setLevel(LogLevel level) {
    if (s_logger) {
        s_logger->set_level(to_spdlog_level(level));
    }
}

void Logger::setPattern(const std::string& pattern) {
    if (s_logger) {
        s_logger->set_pattern(pattern);
    }
}

void Logger::setOutputTarget(OutputTarget target) {
    std::lock_guard<std::mutex> guard(s_init_mutex);
    if (!s_logger) return;

    auto& sinks = s_logger->sinks();
    sinks.clear();

    switch (target) {
        case OutputTarget::Console:
            if (s_console_sink) sinks.push_back(s_console_sink);
            break;
        case OutputTarget::File:
            if (s_file_sink) sinks.push_back(s_file_sink);
            break;
        case OutputTarget::Both:
        default:
            if (s_console_sink) sinks.push_back(s_console_sink);
            if (s_file_sink)    sinks.push_back(s_file_sink);
            break;
    }

    // 保底：若两路 Sink 均无效（极端情况），使用空 Sink 防止崩溃
    if (sinks.empty()) {
        sinks.push_back(std::make_shared<spdlog::sinks::null_sink_mt>());
    }
}

bool Logger::isInitialized() {
    return s_initialized.load(std::memory_order_acquire);
}

}  // namespace logger
