#ifndef MIDDLEWAREPIPELINE_H
#define MIDDLEWAREPIPELINE_H

#include "mvvm_global.h"
#include "imiddleware.h"
#include <functional>
#include <vector>

/**
 * @brief 属性变更中间件管道
 *
 * 维护一组有序的 IPropertyMiddleware，调用 run() 时按注册顺序
 * 依次调用每个中间件的 process()。
 *
 * 管道采用"洋葱"（around）模式：
 *
 *   Middleware1 → Middleware2 → terminal(写入属性) → Middleware2 → Middleware1
 *
 * 即：
 *   - 每个中间件的 "前置逻辑" 在调用 next() 之前执行
 *   - 每个中间件的 "后置逻辑" 在调用 next() 之后执行
 *   - terminal（属性实际写入）嵌在链条最深处
 *
 * 若中间件不调用 next，管道在此处停止（terminal 也不会执行）。
 *
 * 使用示例：
 * @code
 * PropertyMiddlewarePipeline pipeline;
 * pipeline.add(std::make_shared<LoggingMiddleware>());
 * pipeline.add(std::make_shared<ValidationMiddleware>());
 *
 * PropertyContext ctx("price", oldVal, newVal, vm);
 * pipeline.run(ctx, [&](PropertyContext &c) {
 *     if (!c.cancelled) { ... actual write ... }
 * });
 * @endcode
 */
class MVVM_EXPORT PropertyMiddlewarePipeline
{
public:
    /// 终端处理器类型（属性实际写入逻辑）
    using Terminal = std::function<void(PropertyContext &)>;

    /// 向管道末尾追加一个中间件（nullptr 被忽略）
    void add(PropertyMiddlewarePtr middleware);

    /// 移除所有中间件
    void clear();

    /// 管道是否为空
    bool isEmpty() const;

    /// 已注册的中间件数量
    int count() const;

    /**
     * @brief 运行管道（around 模式）
     *
     * @param ctx       属性变更上下文（中间件可修改 newValue / cancelled）
     * @param terminal  链条末端的终端处理器（属性写入逻辑），可为 nullptr
     *
     * 执行路径示意：
     *   MW[0].pre → MW[1].pre → terminal(ctx) → MW[1].post → MW[0].post
     */
    void run(PropertyContext &ctx, Terminal terminal = nullptr) const;

private:
    /// 递归分发：从 index 开始调用剩余中间件，terminal 透传到链条末端
    void dispatchAt(std::size_t index, PropertyContext &ctx,
                    const Terminal &terminal) const;

    std::vector<PropertyMiddlewarePtr> m_middlewares;
};

/**
 * @brief 命令执行中间件管道
 *
 * 与 PropertyMiddlewarePipeline 结构相同，处理 CommandContext。
 *
 * 执行路径示意：
 *   MW[0].pre → MW[1].pre → terminal(execute) → MW[1].post → MW[0].post
 */
class MVVM_EXPORT CommandMiddlewarePipeline
{
public:
    /// 终端处理器类型（命令实际执行逻辑）
    using Terminal = std::function<void(CommandContext &)>;

    void add(CommandMiddlewarePtr middleware);
    void clear();
    bool isEmpty() const;
    int  count() const;

    /**
     * @brief 运行管道（around 模式）
     *
     * @param ctx       命令上下文（中间件可设置 cancelled）
     * @param terminal  链条末端的终端处理器（命令执行逻辑），可为 nullptr
     */
    void run(CommandContext &ctx, Terminal terminal = nullptr) const;

private:
    void dispatchAt(std::size_t index, CommandContext &ctx,
                    const Terminal &terminal) const;

    std::vector<CommandMiddlewarePtr> m_middlewares;
};

#endif // MIDDLEWAREPIPELINE_H
