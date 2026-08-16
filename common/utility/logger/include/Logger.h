#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "logger_global.h"

// ── spdlog（宏依赖：SPDLOG_LOGGER_CALL / SPDLOG_FUNCTION）──────────────────────
#include <spdlog/spdlog.h>

namespace logger {

// =============================================================================
// LogLevel — 日志级别
// =============================================================================
/**
 * @brief 日志级别，与 spdlog::level 一一对应
 */
enum class LogLevel : int {
    Trace    = 0,  ///< 最详细的调试跟踪
    Debug    = 1,  ///< 调试信息
    Info     = 2,  ///< 一般运行信息
    Warn     = 3,  ///< 警告
    Error    = 4,  ///< 错误
    Critical = 5,  ///< 严重错误
    Off      = 6   ///< 关闭日志
};

// =============================================================================
// OutputTarget — 日志输出目标
// =============================================================================
/**
 * @brief 控制日志输出位置
 *
 * 可在 LogConfig 中配置初始值，也可在运行时通过 Logger::setOutputTarget() 动态切换。
 */
enum class OutputTarget {
    Console,  ///< 仅输出到控制台（stdout）
    File,     ///< 仅写入日志文件
    Both      ///< 同时输出到控制台与文件（默认）
};

// =============================================================================
// RotateStrategy — 日志文件轮转策略
// =============================================================================
/**
 * @brief 日志文件轮转策略（所有模式均支持年月合并级别目录）
 *
 * 文件路径结构（年份与月份合并为一级目录 YYYYMM）：
 *   - None / Size : {logDir}/{YYYYMM}/{appName}.log
 *   - Daily       : {logDir}/{YYYYMM}/{appName}_{YYYY-MM-DD}.log
 *   - Both        : {logDir}/{YYYYMM}/{appName}_{YYYY-MM-DD}[.N].log
 *
 * 目录示例（2026年3月）：logs/202603/
 */
enum class RotateStrategy {
    None,   ///< 不轮转，追加写入单一文件
    Daily,  ///< 按日期切割：每天零点创建新文件，目录按月份分级
    Size,   ///< 按文件大小切割：超出 maxFileSize 时轮转
    Both    ///< 混合切割：按日期 + 按文件大小，同日期内超限追加序号
};

// =============================================================================
// FlushMode — 日志刷盘模式
// =============================================================================
/**
 * @brief 控制日志写入 sink 后是否立即 flush 到磁盘
 */
enum class FlushMode : int {
    Buffered = 0,  ///< 异步缓冲，仅 Error+ 立即刷盘（默认）
    Sync     = 1,  ///< 同步输出，Info+ 每条立即刷盘
};

// =============================================================================
// LogConfig — 日志配置
// =============================================================================
/**
 * @brief 日志系统配置结构体，所有字段均提供合理默认值
 *
 * 使用示例：
 * @code
 *   logger::LogConfig cfg;
 *   cfg.appName        = "MyService";
 *   cfg.logDir         = "logs";
 *   cfg.level          = logger::LogLevel::Debug;
 *   cfg.rotateStrategy = logger::RotateStrategy::Both;
 *   logger::Logger::init(cfg);
 * @endcode
 */
struct LogConfig {
    // ── 基础配置 ──────────────────────────────────────────────────────────────
    std::string appName = "app";           ///< 应用/Logger 名称，影响文件名与注册名
    std::string logDir  = "logs";          ///< 日志根目录（自动创建 YYYY/MM 子目录）
    LogLevel    level   = LogLevel::Info;  ///< 最低输出级别

    /**
     * @brief 自定义输出格式 Pattern（留空使用内置默认格式）
     *
     * 默认格式包含：时间戳 | 级别 | 线程ID | 源文件:行号 | 函数名 | 消息体
     *
     * 常用占位符：
     *   %Y-%m-%d %H:%M:%S  时间戳
     *   %e                 毫秒
     *   %^...%$            彩色区域（含括号内所有文本）
     *   %l / %L            级别文本（小写 / 首字母大写）
     *   %-Nl               左对齐并补空格至 N 位
     *   %t                 线程 ID
     *   %n                 Logger 名称
     *   %v                 日志消息体
     *   %s                 源文件名（basename，需使用 LOG_* 宏）
     *   %#                 源文件行号（需使用 LOG_* 宏）
     *   %!                 函数名（需使用 LOG_* 宏）
     *   %P                 进程 ID
     */
    std::string pattern;

    // ── 输出目标 ──────────────────────────────────────────────────────────────
    OutputTarget target      = OutputTarget::Both;  ///< 输出目标（控制台/文件/两者）
    bool         enableColor = true;                ///< 控制台彩色输出（仅对控制台有效）

    // ── 轮转策略（默认混合轮转） ────────────────────────────────────────────────
    RotateStrategy rotateStrategy = RotateStrategy::Both;

    // Size 轮转参数
    std::size_t maxFileSize     = 10ULL * 1024 * 1024;  ///< 单文件最大字节（默认 10 MB）
    std::size_t maxRotatedFiles = 10;                   ///< 保留的历史文件最大数量

    // Daily 轮转参数
    int      rotationHour   = 0;   ///< 每日轮转时刻：小时（0-23），默认 0 点
    int      rotationMinute = 0;   ///< 每日轮转时刻：分钟（0-59），默认 0 分
    uint16_t maxDailyFiles  = 30;  ///< 保留最近 N 天的日志文件（0 = 不限制）

    // ── 刷盘策略 ──────────────────────────────────────────────────────────────
    /**
     * @brief 日志刷盘模式
     *
     * - Buffered：异步缓冲（默认）。内存缓冲批量写文件，仅 Error 及以上立即 flush。
     * - Sync：同步输出。每条 Info 及以上写入后立即 flush 到文件/控制台 sink。
     *
     * 可在 app.toml [logger] flushMode 配置：0=Buffered，1=Sync。
     */
    FlushMode flushMode = FlushMode::Buffered;
};

// =============================================================================
// Logger — 日志门面（Facade / Singleton）
// =============================================================================
/**
 * @brief 框架日志门面类
 *
 * 线程安全。所有接口均为静态方法，无需持有实例。
 * 建议在程序入口调用 init()，程序退出前调用 shutdown()。
 *
 * 推荐使用便捷宏（自动携带源文件、行号、函数信息）：
 *   LOG_TRACE / LOG_DEBUG / LOG_INFO / LOG_WARN / LOG_ERROR / LOG_CRITICAL
 */
class LOGGER_EXPORT Logger {
public:
    /**
     * @brief  初始化日志系统（程序启动时调用一次）
     * @param  config  日志配置，所有字段均有默认值
     * @return 成功返回 true；失败在 stderr 打印错误并返回 false
     * @note   重复调用无效（已初始化则直接返回 true）
     */
    static bool init(const LogConfig& config = {});

    /**
     * @brief 刷新所有 Sink 缓冲区并释放资源（程序退出前调用）
     */
    static void shutdown();

    /**
     * @brief 获取底层 spdlog::logger 实例
     * @return 未初始化时返回 nullptr
     */
    static std::shared_ptr<spdlog::logger> get();

    /**
     * @brief 运行时动态修改最低日志级别
     */
    static void setLevel(LogLevel level);

    /**
     * @brief 运行时动态修改所有 Sink 的输出格式
     */
    static void setPattern(const std::string& pattern);

    /**
     * @brief 运行时动态切换日志输出目标
     *
     * 在 init() 时始终创建控制台和文件两路 Sink；
     * 调用本方法可在任意时刻切换哪路 Sink 生效，无需重启。
     *
     * @note 若 init() 时未配置文件路径，切换到 File / Both 不会产生文件输出。
     */
    static void setOutputTarget(OutputTarget target);

    /**
     * @brief 查询是否已完成初始化
     */
    static bool isInitialized();

    Logger()                         = delete;
    ~Logger()                        = delete;
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
};

}  // namespace logger

// =============================================================================
// 便捷日志宏
//
// 通过 SPDLOG_LOGGER_CALL 自动携带：
//   - 源文件名（%s）
//   - 源文件行号（%#）
//   - 函数名（%!）
//
// 用法：LOG_INFO("Server started on port {}", 8080);
// =============================================================================
#define LOG_TRACE(...)    do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::trace,    __VA_ARGS__); } while (0)
#define LOG_DEBUG(...)    do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::debug,    __VA_ARGS__); } while (0)
#define LOG_INFO(...)     do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::info,     __VA_ARGS__); } while (0)
#define LOG_WARN(...)     do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::warn,     __VA_ARGS__); } while (0)
#define LOG_ERROR(...)    do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::err,      __VA_ARGS__); } while (0)
#define LOG_CRITICAL(...) do { if (auto _l = ::logger::Logger::get()) SPDLOG_LOGGER_CALL(_l, spdlog::level::critical, __VA_ARGS__); } while (0)
