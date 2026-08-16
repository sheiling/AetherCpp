#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

namespace permission {

/// 审计列表行（ViewModel → View）
struct AuditLogListItem {
    qint64    id           = 0;
    QDateTime occurredAt;
    QString   username;
    QString   roleNames;
    QString   actionType;
    QString   actionName;
    QString   target;
    QString   result;
    QString   ip;
    QString   moduleCode;
    QString   detailJson;
};

} // namespace permission
