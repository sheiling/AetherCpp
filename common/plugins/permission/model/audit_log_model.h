#pragma once

#include "data/domain/domain_types.h"
#include "model/audit_log_management_model.h"

#include <QVector>

namespace permission {

class IDataStore;

class AuditLogModel
{
public:
    explicit AuditLogModel(IDataStore *store = nullptr);

    QVector<AuditLogListItem> list(const AuditLogQueryFilter &filter) const;
    qint64                    count(const AuditLogQueryFilter &filter) const;
    bool                      findById(qint64 id, AuditLogRecord &out) const;

    static AuditLogListItem toListItem(const AuditLogRecord &record);

private:
    IDataStore *m_store = nullptr;
};

} // namespace permission
