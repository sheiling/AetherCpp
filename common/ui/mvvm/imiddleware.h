#ifndef IMIDDLEWARE_H
#define IMIDDLEWARE_H

#include "mvvm_global.h"
#include "middlewarecontext.h"
#include <functional>
#include <memory>

/**
 * @brief 属性变更中间件接口
 *
 * 实现此接口以拦截 ViewModel 属性变更。
 *
 * 调用约定：
 *   - 调用 next(ctx) 将控制权传递给管道中的下一个中间件；
 *   - 不调用 next 则阻断后续所有中间件（但不自动设置 cancelled）；
 *   - 若需阻止属性真正写入，应设置 ctx.cancelled = true。
 *
 * 推荐"环绕"（around）写法：
 * @code
 * void process(PropertyContext &ctx, Next next) override
 * {
 *     // --- 前置处理 ---
 *     qDebug() << "before:" << ctx.propertyName;
 *     next(ctx);
 *     // --- 后置处理 ---
 *     qDebug() << "after, cancelled=" << ctx.cancelled;
 * }
 * @endcode
 */
class MVVM_EXPORT IPropertyMiddleware
{
public:
    virtual ~IPropertyMiddleware() = default;

    /// Next 函数类型：调用它将控制权传递给下一个中间件
    using Next = std::function<void(PropertyContext &)>;

    virtual void process(PropertyContext &ctx, Next next) = 0;
};

/**
 * @brief 命令执行中间件接口
 *
 * 实现此接口以拦截 Command::run() 调用。
 *
 * 调用约定与 IPropertyMiddleware 相同。
 * 若需阻止命令执行，设置 ctx.cancelled = true 并不调用 next 即可。
 */
class MVVM_EXPORT ICommandMiddleware
{
public:
    virtual ~ICommandMiddleware() = default;

    using Next = std::function<void(CommandContext &)>;

    virtual void process(CommandContext &ctx, Next next) = 0;
};

/// 便捷别名
using PropertyMiddlewarePtr = std::shared_ptr<IPropertyMiddleware>;
using CommandMiddlewarePtr  = std::shared_ptr<ICommandMiddleware>;

#endif // IMIDDLEWARE_H
