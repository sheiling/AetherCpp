#include "service/audit_log_writer.h"

#include "data/domain/domain_types.h"
#include "model/auth_service.h"
#include "repository/datastore.h"
#include "repository/interface/idatastore.h"
#include "service/perm_system_constants.h"

#include "viewmodel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>

namespace permission {
namespace {

QString viewModelTypeName(ViewModel *vm)
{
    if (!vm)
        return {};
    return QString::fromLatin1(vm->metaObject()->className());
}

void fillOperator(AuditLogRecord &record)
{
    const AuthSession session = AuthService::currentSession();
    if (session.isValid()) {
        record.userId    = session.userId;
        record.username  = session.username;
        if (!session.primaryRoleCode.isEmpty())
            record.roleNames = session.primaryRoleCode;
        else
            record.roleNames = session.roleCodes.join(QStringLiteral(", "));
        return;
    }

    record.userId   = 0;
    record.username = QString::fromUtf8(kMinimalUsername);
    record.roleNames.clear();
}

QString detailJsonWithSource(const QString &source, qint64 elapsedMs = -1,
                             const QString &viewModelType = QString())
{
    QJsonObject obj;
    obj.insert(QStringLiteral("source"), source);
    if (elapsedMs >= 0)
        obj.insert(QStringLiteral("elapsedMs"), elapsedMs);
    if (!viewModelType.isEmpty())
        obj.insert(QStringLiteral("viewModel"), viewModelType);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

} // namespace

void AuditLogWriter::registerCallbacks()
{
    AuditLogRegistry::setCommandAuditCallback(
        [](const CommandAuditEvent &e) { appendCommandAudit(e); });
    AuditLogRegistry::setManualAuditCallback(
        [](const ManualAuditEvent &e) { appendManualAudit(e); });
}

bool AuditLogWriter::shouldSkipCommand(const QString &commandName)
{
    const QString name = commandName.trimmed();
    if (name.isEmpty())
        return true;

    static const QStringList kSkipSuffixes = {
        QStringLiteral(".query"),
        QStringLiteral(".prevPage"),
        QStringLiteral(".nextPage"),
        QStringLiteral(".resetFilter"),
        QStringLiteral(".selectAll"),
        QStringLiteral(".clear"),
    };
    for (const QString &suffix : kSkipSuffixes) {
        if (name.endsWith(suffix, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

QString AuditLogWriter::moduleCodeFromKey(const QString &key)
{
    const int dot = key.indexOf(QLatin1Char('.'));
    if (dot <= 0)
        return key.trimmed();
    return key.left(dot);
}

bool AuditLogWriter::appendRecord(const AuditLogRecord &record)
{
    IDataStore *store = DataStore::instance();
    if (!store || !store->isOpen()) {
        qWarning() << "[AuditLogWriter] Data store not open, skip audit:"
                   << record.actionType << record.actionName;
        return false;
    }

    QString err;
    if (!store->auditLogs().append(record, &err)) {
        qWarning() << "[AuditLogWriter] append failed:" << err << record.actionName;
        return false;
    }
    return true;
}

void AuditLogWriter::appendCommandAudit(const CommandAuditEvent &event)
{
    if (shouldSkipCommand(event.commandName))
        return;

    AuditLogRecord record;
    record.occurredAt  = event.occurredAt;
    fillOperator(record);
    record.actionType  = QStringLiteral("command");
    record.actionName  = event.commandName;
    record.target      = viewModelTypeName(event.viewModel);
    record.result      = event.cancelled ? QStringLiteral("cancelled") : QStringLiteral("success");
    record.moduleCode  = moduleCodeFromKey(event.commandName);
    record.detailJson  = detailJsonWithSource(QStringLiteral("command"), event.elapsedMs,
                                              record.target);

    appendRecord(record);
}

void AuditLogWriter::appendManualAudit(const ManualAuditEvent &event)
{
    AuditLogRecord record;
    record.occurredAt  = event.occurredAt;
    fillOperator(record);
    record.actionType  = event.actionType;
    record.actionName  = event.actionName;
    record.target      = event.target;
    record.result      = event.result.isEmpty() ? QStringLiteral("success") : event.result;
    record.moduleCode  = event.moduleCode;
    record.detailJson  = event.detailJson.isEmpty()
                             ? detailJsonWithSource(QStringLiteral("manual"))
                             : event.detailJson;

    appendRecord(record);
}

void AuditLogWriter::appendLoginSuccess(const AuthSession &session)
{
    AuditLogRecord record;
    record.occurredAt  = QDateTime::currentDateTime();
    record.userId      = session.userId;
    record.username    = session.username;
    record.roleNames   = session.primaryRoleCode.isEmpty()
                             ? session.roleCodes.join(QStringLiteral(", "))
                             : session.primaryRoleCode;
    record.actionType  = QStringLiteral("auth");
    record.actionName  = QStringLiteral("login");
    record.target      = QStringLiteral("user:%1").arg(session.username);
    record.result      = QStringLiteral("success");
    record.moduleCode  = QStringLiteral("auth");
    record.detailJson  = detailJsonWithSource(QStringLiteral("manual"));

    appendRecord(record);
}

void AuditLogWriter::appendLoginFailure(const QString &username, const QString &reasonKey)
{
    const QString user = username.trimmed();

    AuditLogRecord record;
    record.occurredAt  = QDateTime::currentDateTime();
    record.userId      = 0;
    record.username    = user.isEmpty() ? QString::fromUtf8(kMinimalUsername) : user;
    record.roleNames.clear();
    record.actionType  = QStringLiteral("auth");
    record.actionName  = QStringLiteral("login_failed");
    record.target      = QStringLiteral("user:%1").arg(user.isEmpty() ? QStringLiteral("unknown") : user);
    record.result      = QStringLiteral("failure");
    record.moduleCode  = QStringLiteral("auth");

    QJsonObject obj;
    obj.insert(QStringLiteral("source"), QStringLiteral("manual"));
    if (!reasonKey.isEmpty())
        obj.insert(QStringLiteral("reason"), reasonKey);
    record.detailJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    appendRecord(record);
}

} // namespace permission
