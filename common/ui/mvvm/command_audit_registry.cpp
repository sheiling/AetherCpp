#include "command_audit_registry.h"

namespace {
AuditLogRegistry::CommandAuditCallback g_commandCb;
AuditLogRegistry::ManualAuditCallback g_manualCb;
} // namespace

void AuditLogRegistry::setCommandAuditCallback(CommandAuditCallback cb)
{
    g_commandCb = std::move(cb);
}

void AuditLogRegistry::setManualAuditCallback(ManualAuditCallback cb)
{
    g_manualCb = std::move(cb);
}

bool AuditLogRegistry::hasCommandCallback()
{
    return static_cast<bool>(g_commandCb);
}

bool AuditLogRegistry::hasManualCallback()
{
    return static_cast<bool>(g_manualCb);
}

void AuditLogRegistry::emitCommandAudit(const CommandAuditEvent &event)
{
    if (g_commandCb)
        g_commandCb(event);
}

void AuditLogRegistry::emitManualAudit(const ManualAuditEvent &event)
{
    if (g_manualCb)
        g_manualCb(event);
}
