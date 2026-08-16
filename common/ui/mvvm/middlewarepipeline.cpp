#include "middlewarepipeline.h"

// ============================================================================
// PropertyMiddlewarePipeline
// ============================================================================

void PropertyMiddlewarePipeline::add(PropertyMiddlewarePtr middleware)
{
    if (middleware)
        m_middlewares.push_back(std::move(middleware));
}

void PropertyMiddlewarePipeline::clear()
{
    m_middlewares.clear();
}

bool PropertyMiddlewarePipeline::isEmpty() const
{
    return m_middlewares.empty();
}

int PropertyMiddlewarePipeline::count() const
{
    return static_cast<int>(m_middlewares.size());
}

void PropertyMiddlewarePipeline::run(PropertyContext &ctx, Terminal terminal) const
{
    dispatchAt(0, ctx, terminal);
}

/**
 * 索引递归分发（around 模式）：
 *   - 未到达末端：构造 next，调用当前中间件的 process()
 *   - 到达末端（index >= size）：调用 terminal（即属性写入逻辑）
 *
 * 执行顺序（以 2 个中间件为例）：
 *   dispatchAt(0) → MW[0].process(ctx, next0)
 *     next0: dispatchAt(1) → MW[1].process(ctx, next1)
 *       next1: dispatchAt(2) → terminal(ctx)   ← 属性写入
 *     MW[1] 后置逻辑
 *   MW[0] 后置逻辑
 */
void PropertyMiddlewarePipeline::dispatchAt(std::size_t index,
                                             PropertyContext &ctx,
                                             const Terminal &terminal) const
{
    if (index >= m_middlewares.size()) {
        // 链条末端：执行属性写入终端逻辑
        if (terminal)
            terminal(ctx);
        return;
    }

    const auto &mw = m_middlewares[index];
    // 注意：terminal 按值捕获，避免潜在悬垂引用
    mw->process(ctx, [this, index, terminal](PropertyContext &c) {
        dispatchAt(index + 1, c, terminal);
    });
}

// ============================================================================
// CommandMiddlewarePipeline
// ============================================================================

void CommandMiddlewarePipeline::add(CommandMiddlewarePtr middleware)
{
    if (middleware)
        m_middlewares.push_back(std::move(middleware));
}

void CommandMiddlewarePipeline::clear()
{
    m_middlewares.clear();
}

bool CommandMiddlewarePipeline::isEmpty() const
{
    return m_middlewares.empty();
}

int CommandMiddlewarePipeline::count() const
{
    return static_cast<int>(m_middlewares.size());
}

void CommandMiddlewarePipeline::run(CommandContext &ctx, Terminal terminal) const
{
    dispatchAt(0, ctx, terminal);
}

/**
 * 索引递归分发（around 模式）：
 *   - 未到达末端：构造 next，调用当前中间件的 process()
 *   - 到达末端：调用 terminal（即 execute() 逻辑）
 *
 * 执行顺序（以 3 个中间件为例）：
 *   MW[0].pre → MW[1].pre → MW[2].pre
 *     → terminal: execute()
 *   ← MW[2].post ← MW[1].post ← MW[0].post
 */
void CommandMiddlewarePipeline::dispatchAt(std::size_t index,
                                            CommandContext &ctx,
                                            const Terminal &terminal) const
{
    if (index >= m_middlewares.size()) {
        // 链条末端：执行命令终端逻辑
        if (terminal)
            terminal(ctx);
        return;
    }

    const auto &mw = m_middlewares[index];
    mw->process(ctx, [this, index, terminal](CommandContext &c) {
        dispatchAt(index + 1, c, terminal);
    });
}
