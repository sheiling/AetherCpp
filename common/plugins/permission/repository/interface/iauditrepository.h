#pragma once

#include "data/domain/domain_types.h"

#include <QString>
#include <QVector>

namespace permission {

class IAuditRepository
{
public:
    virtual ~IAuditRepository() = default;

    virtual QVector<AuditLogRecord> list(const AuditLogQueryFilter &filter) = 0;
    virtual qint64                  count(const AuditLogQueryFilter &filter) = 0;
    virtual bool                    append(const AuditLogRecord &record,
                                           QString *errorString = nullptr) = 0;
};

} // namespace permission
