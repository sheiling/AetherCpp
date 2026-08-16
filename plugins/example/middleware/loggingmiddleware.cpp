#include "loggingmiddleware.h"
#include <QDebug>
#include <QElapsedTimer>

// ============================================================================
// LoggingMiddleware  —  属性变更日志（around 模式）
// ============================================================================
//
// 执行路径示意（属性管道 LoggingMiddleware → AuditLogMiddleware）：
//
//   [属性日志] → 进入: price  0 → 999
//     [审计日志] → ...
//       ★ 属性实际写入: price = 999
//     [审计日志] ← 记录审计条目
//   [属性日志] ← 完成: price = 999 ✓
//
// ============================================================================

void LoggingMiddleware::process(PropertyContext &ctx, Next next)
{
    // ── 前置日志（→ 进入） ──────────────────────────────────────────────────
    QString preMsg = QString::fromUtf8("[属性日志] \xe2\x86\x92 进入: %1  [%2] \xe2\x86\x92 [%3]")
                         .arg(ctx.propertyName)
                         .arg(ctx.oldValue.toString())
                         .arg(ctx.newValue.toString());
    qDebug().noquote() << preMsg;
    if (m_logCb) m_logCb(preMsg);

    // ── 调用后续中间件（属性写入嵌在最深处） ───────────────────────────────
    next(ctx);

    // ── 后置日志（← 返回） ──────────────────────────────────────────────────
    QString postMsg;
    if (ctx.cancelled) {
        postMsg = QString::fromUtf8("[属性日志] \xe2\x86\x90 取消: %1 \xe2\x9c\x97").arg(ctx.propertyName);
    } else {
        postMsg = QString::fromUtf8("[属性日志] \xe2\x86\x90 完成: %1 = [%2] \xe2\x9c\x93")
                      .arg(ctx.propertyName)
                      .arg(ctx.newValue.toString());
    }
    qDebug().noquote() << postMsg;
    if (m_logCb) m_logCb(postMsg);
}

// ============================================================================
// LoggingCommandMiddleware  —  命令执行日志（around 模式）
// ============================================================================
//
// 执行路径示意（命令管道 Timing → Auth → Logging → execute）：
//
//   [计时] → 开始: addProduct
//     [AUTH] → 验证: addProduct
//       [命令日志] → 进入: addProduct
//         ★ execute(): addProduct 实际执行
//       [命令日志] ← 完成: addProduct ✓
//     [AUTH] ← 通过: addProduct ✓
//   [计时] ← 结束: addProduct  耗时 N ms ✓
//
// ============================================================================

void LoggingCommandMiddleware::process(CommandContext &ctx, Next next)
{
    // ── 前置日志（→ 进入） ──────────────────────────────────────────────────
    QString preMsg = QString::fromUtf8("[命令日志] \xe2\x86\x92 进入: %1").arg(ctx.commandName);
    qDebug().noquote() << preMsg;
    if (m_logCb) m_logCb(preMsg);

    // ── 调用后续中间件（execute() 嵌在最深处） ─────────────────────────────
    next(ctx);

    // ── 后置日志（← 返回） ──────────────────────────────────────────────────
    QString postMsg;
    if (ctx.cancelled) {
        postMsg = QString::fromUtf8("[命令日志] \xe2\x86\x90 取消: %1 \xe2\x9c\x97").arg(ctx.commandName);
    } else {
        postMsg = QString::fromUtf8("[命令日志] \xe2\x86\x90 完成: %1 \xe2\x9c\x93").arg(ctx.commandName);
    }
    qDebug().noquote() << postMsg;
    if (m_logCb) m_logCb(postMsg);
}

// ============================================================================
// TimingCommandMiddleware  —  命令执行计时（around 模式，最外层）
// ============================================================================
//
// 作为最外层中间件注册时，完整包裹所有内层中间件及 execute()，
// 准确测量命令全链路耗时。
//
// ============================================================================

void TimingCommandMiddleware::process(CommandContext &ctx, Next next)
{
    QElapsedTimer timer;
    timer.start();

    // ── 前置（→ 开始计时） ────────────────────────────────────────────────
    QString preMsg = QString::fromUtf8("[计时] \xe2\x86\x92 开始: %1").arg(ctx.commandName);
    qDebug().noquote() << preMsg;
    if (m_logCb) m_logCb(preMsg);

    // ── 调用后续中间件链（含实际 execute()） ───────────────────────────────
    next(ctx);

    // ── 后置（← 结束计时） ────────────────────────────────────────────────
    qint64 elapsed = timer.elapsed();
    QString postMsg;
    if (ctx.cancelled) {
        postMsg = QString::fromUtf8("[计时] \xe2\x86\x90 取消: %1  (%2 ms) \xe2\x9c\x97")
                      .arg(ctx.commandName)
                      .arg(elapsed);
    } else {
        postMsg = QString::fromUtf8("[计时] \xe2\x86\x90 结束: %1  耗时 %2 ms \xe2\x9c\x93")
                      .arg(ctx.commandName)
                      .arg(elapsed);
    }
    qDebug().noquote() << postMsg;
    if (m_logCb) m_logCb(postMsg);
}
