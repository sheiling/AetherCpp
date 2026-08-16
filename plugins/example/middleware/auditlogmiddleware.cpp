#include "auditlogmiddleware.h"

void AuditLogMiddleware::process(PropertyContext &ctx, Next next)
{
    // 先让后续中间件/操作执行
    next(ctx);

    // 仅在变更未被取消时记录审计条目
    if (!ctx.cancelled) {
        Entry entry;
        entry.timestamp    = QDateTime::currentDateTime();
        entry.propertyName = ctx.propertyName;
        entry.oldValue     = ctx.oldValue;
        entry.newValue     = ctx.newValue;

        m_history.append(entry);

        if (m_onEntryAdded)
            m_onEntryAdded(entry);
    }
}
