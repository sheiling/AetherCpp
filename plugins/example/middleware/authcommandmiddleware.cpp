#include "authcommandmiddleware.h"
#include <QDebug>

void AuthCommandMiddleware::process(CommandContext &ctx, Next next)
{
    if (!m_permitted) {
        // 权限不足：阻止命令，通知回调
        ctx.cancelled = true;
        QString msg = QString("[AUTH] 命令 '%1' 被拒绝：权限不足").arg(ctx.commandName);
        qDebug().noquote() << msg;
        if (m_onDenied)
            m_onDenied(ctx.commandName);
        return;  // 不调用 next，后续中间件及 execute() 均不执行
    }

    next(ctx);
}
