#pragma once

#include "command_audit_registry.h"
#include "model/auth_session.h"

#include <QString>

namespace permission {

class AuditLogWriter
{
public:
    static void registerCallbacks();

    static void appendCommandAudit(const CommandAuditEvent &event);
    static void appendManualAudit(const ManualAuditEvent &event);

    static void appendLoginSuccess(const AuthSession &session);
    static void appendLoginFailure(const QString &username, const QString &reasonKey);

private:
    static bool shouldSkipCommand(const QString &commandName);
    static QString moduleCodeFromKey(const QString &key);
    static bool appendRecord(const struct AuditLogRecord &record);
};

} // namespace permission
