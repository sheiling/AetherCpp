#ifndef AUTHCOMMANDMIDDLEWARE_H
#define AUTHCOMMANDMIDDLEWARE_H

#include "imiddleware.h"
#include <functional>
#include <QString>

/**
 * @brief 权限校验命令中间件
 *
 * 演示如何通过命令中间件实现权限控制：
 *   - 当 permitted = true 时，命令正常执行；
 *   - 当 permitted = false 时，设置 ctx.cancelled = true，阻止命令执行，
 *     并通过回调通知 UI 显示"权限不足"提示。
 *
 * 使用方式：
 * @code
 * auto auth = std::make_shared<AuthCommandMiddleware>();
 * auth->setOnDenied([this](const QString &cmd) {
 *     m_logEdit->append(QString("[AUTH] 命令 '%1' 被拒绝：权限不足").arg(cmd));
 * });
 * command->addMiddleware(auth);
 *
 * // 运行时切换权限
 * auth->setPermitted(false);  // 禁止
 * auth->setPermitted(true);   // 允许
 * @endcode
 */
class AuthCommandMiddleware : public ICommandMiddleware
{
public:
    using DeniedCallback = std::function<void(const QString &commandName)>;

    /// 设置是否允许命令执行（默认允许）
    void setPermitted(bool permitted) { m_permitted = permitted; }
    bool isPermitted() const { return m_permitted; }

    /// 设置权限拒绝回调
    void setOnDenied(DeniedCallback cb) { m_onDenied = std::move(cb); }

    void process(CommandContext &ctx, Next next) override;

private:
    bool            m_permitted = true;
    DeniedCallback  m_onDenied;
};

#endif // AUTHCOMMANDMIDDLEWARE_H
