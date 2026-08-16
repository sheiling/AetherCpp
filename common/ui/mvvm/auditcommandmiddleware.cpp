#include "auditcommandmiddleware.h"

#include "command_audit_registry.h"

#include <QElapsedTimer>

void AuditCommandMiddleware::process(CommandContext &ctx, Next next)
{
    QElapsedTimer timer;
    timer.start();

    next(ctx);

    if (!AuditLogRegistry::hasCommandCallback())
        return;

    CommandAuditEvent event;
    event.commandName = ctx.commandName;
    event.occurredAt  = QDateTime::currentDateTime();
    event.cancelled   = ctx.cancelled;
    event.elapsedMs   = timer.elapsed();
    event.viewModel   = ctx.viewModel;

    AuditLogRegistry::emitCommandAudit(event);
}
