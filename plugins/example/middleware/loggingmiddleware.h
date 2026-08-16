#ifndef LOGGINGMIDDLEWARE_H
#define LOGGINGMIDDLEWARE_H

#include "imiddleware.h"
#include <functional>
#include <QString>

/**
 * @brief 属性变更日志中间件
 *
 * 以"环绕"（around）方式拦截所有属性变更，清晰展示执行路径：
 *
 *   → 进入（前置）: propertyName  oldValue → newValue
 *       [后续中间件 / 属性实际写入]
 *   ← 完成（后置）: propertyName = finalValue ✓
 *
 * 不修改任何数据，始终调用 next()。
 *
 * 使用方式：
 * @code
 * auto logging = std::make_shared<LoggingMiddleware>();
 * logging->setLogCallback([this](const QString &msg) {
 *     m_logEdit->append(msg);
 * });
 * viewModel->addPropertyMiddleware(logging);
 * @endcode
 */
class LoggingMiddleware : public IPropertyMiddleware
{
public:
    using LogCallback = std::function<void(const QString &message)>;

    /// 设置日志回调（在 UI 线程中调用，可直接更新控件）
    void setLogCallback(LogCallback cb) { m_logCb = std::move(cb); }

    void process(PropertyContext &ctx, Next next) override;

private:
    LogCallback m_logCb;
};

/**
 * @brief 命令执行日志中间件
 *
 * 以"环绕"（around）方式记录命令执行路径：
 *
 *   → 进入: commandName
 *       [后续中间件 / execute()]
 *   ← 完成: commandName ✓   或   ← 取消: commandName ✗
 *
 * 使用方式：
 * @code
 * auto cmdLog = std::make_shared<LoggingCommandMiddleware>();
 * cmdLog->setLogCallback([this](const QString &msg) { m_logEdit->append(msg); });
 * command->addMiddleware(cmdLog);
 * @endcode
 */
class LoggingCommandMiddleware : public ICommandMiddleware
{
public:
    using LogCallback = std::function<void(const QString &message)>;

    void setLogCallback(LogCallback cb) { m_logCb = std::move(cb); }

    void process(CommandContext &ctx, Next next) override;

private:
    LogCallback m_logCb;
};

/**
 * @brief 命令执行计时中间件
 *
 * 以"环绕"（around）方式记录命令执行耗时，清晰展示 around 模式：
 *
 *   → 开始计时: commandName
 *       [后续中间件 / execute()]
 *   ← 结束计时: commandName  耗时 N ms ✓
 *
 * 典型用途：
 *   - 性能监控
 *   - 慢操作告警
 *   - 演示 around 执行路径
 *
 * 使用方式：
 * @code
 * auto timing = std::make_shared<TimingCommandMiddleware>();
 * timing->setLogCallback([this](const QString &msg) { m_logEdit->append(msg); });
 * command->addMiddleware(timing);  // 建议注册为最外层中间件
 * @endcode
 */
class TimingCommandMiddleware : public ICommandMiddleware
{
public:
    using LogCallback = std::function<void(const QString &message)>;

    void setLogCallback(LogCallback cb) { m_logCb = std::move(cb); }

    void process(CommandContext &ctx, Next next) override;

private:
    LogCallback m_logCb;
};

#endif // LOGGINGMIDDLEWARE_H
