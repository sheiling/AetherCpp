#ifndef COMMAND_AUDIT_REGISTRY_H
#define COMMAND_AUDIT_REGISTRY_H

#include "mvvm_global.h"

#include <QDateTime>
#include <QString>
#include <functional>

class ViewModel;

/// Command::run() 完成后由全局审计中间件填充
struct MVVM_EXPORT CommandAuditEvent
{
    QString    commandName;
    QDateTime  occurredAt;
    bool       cancelled = false;
    qint64     elapsedMs = 0;
    ViewModel *viewModel = nullptr;
};

/// 非 Command 路径（登出、设备启动等）由调用方填充
struct MVVM_EXPORT ManualAuditEvent
{
    QString   actionType;
    QString   actionName;
    QString   target;
    QString   result = QStringLiteral("success");
    QString   moduleCode;
    QString   detailJson;
    QDateTime occurredAt = QDateTime::currentDateTime();
};

class MVVM_EXPORT AuditLogRegistry
{
public:
    using CommandAuditCallback = std::function<void(const CommandAuditEvent &)>;
    using ManualAuditCallback    = std::function<void(const ManualAuditEvent &)>;

    static void setCommandAuditCallback(CommandAuditCallback cb);
    static void setManualAuditCallback(ManualAuditCallback cb);

    static bool hasCommandCallback();
    static bool hasManualCallback();

    static void emitCommandAudit(const CommandAuditEvent &event);
    static void emitManualAudit(const ManualAuditEvent &event);

private:
    AuditLogRegistry() = delete;
};

#endif // COMMAND_AUDIT_REGISTRY_H
