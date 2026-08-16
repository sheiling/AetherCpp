#ifndef MIDDLEWARECONTEXT_H
#define MIDDLEWARECONTEXT_H

#include "mvvm_global.h"
#include <QString>
#include <QVariant>

// 前向声明（避免循环包含）
class ViewModel;

/**
 * @brief 属性变更中间件上下文
 *
 * 在 ViewModel::setProperty() 触发时创建并传入管道。
 * 中间件可读写 newValue（修改最终写入值），
 * 也可将 cancelled 置为 true 以阻止此次变更。
 *
 * 典型用途：
 *   - 日志记录（读取 propertyName / oldValue / newValue）
 *   - 数据校验（修改 newValue 或设置 cancelled）
 *   - 审计追踪（记录变更历史）
 */
struct MVVM_EXPORT PropertyContext
{
    QString    propertyName; ///< 属性名
    QVariant   oldValue;     ///< 变更前的值（只读）
    QVariant   newValue;     ///< 变更后的值（中间件可修改）
    ViewModel *viewModel;    ///< 所属 ViewModel（可用于读取其他属性）
    bool       cancelled;    ///< true = 阻止本次属性变更

    PropertyContext(const QString &name,
                    const QVariant &oldVal,
                    const QVariant &newVal,
                    ViewModel      *vm)
        : propertyName(name)
        , oldValue(oldVal)
        , newValue(newVal)
        , viewModel(vm)
        , cancelled(false)
    {}
};

/**
 * @brief 命令执行中间件上下文
 *
 * 在 Command::run() 触发时创建并传入管道。
 * 中间件可将 cancelled 置为 true 以阻止命令执行。
 *
 * 典型用途：
 *   - 权限检查（cancelled = true 阻止未授权操作）
 *   - 命令日志（记录哪个命令被触发）
 *   - 执行计时（前置/后置打点）
 */
struct MVVM_EXPORT CommandContext
{
    QString    commandName; ///< 命令名称
    ViewModel *viewModel;   ///< 关联的 ViewModel（可为 nullptr）
    bool       cancelled;   ///< true = 阻止命令执行

    explicit CommandContext(const QString &name, ViewModel *vm = nullptr)
        : commandName(name)
        , viewModel(vm)
        , cancelled(false)
    {}
};

#endif // MIDDLEWARECONTEXT_H
