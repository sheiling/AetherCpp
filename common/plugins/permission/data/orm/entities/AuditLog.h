#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QxOrm.h>
#include <QtCore/QString>

struct AuditLog
{
    long    id          = 0;
    QString occurredAt;
    long    userId      = 0;
    QString username;
    QString roleNames;
    QString actionType;
    QString actionName;
    QString target;
    QString result;
    QString ip;
    QString moduleCode;
    QString detailJson;
};

QX_REGISTER_HPP(AuditLog, qx::trait::no_base_class_defined, 0)

namespace qx {
template <> void register_class(QxClass<AuditLog> &t);
}

#endif // COMMON_ORM_ENABLED
