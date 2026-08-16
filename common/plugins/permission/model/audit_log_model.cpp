#include "model/audit_log_model.h"

#include "repository/datastore.h"
#include "repository/interface/idatastore.h"

namespace permission {

AuditLogModel::AuditLogModel(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

AuditLogListItem AuditLogModel::toListItem(const AuditLogRecord &record)
{
    AuditLogListItem item;
    item.id          = record.id;
    item.occurredAt  = record.occurredAt;
    item.username    = record.username;
    item.roleNames   = record.roleNames;
    item.actionType  = record.actionType;
    item.actionName  = record.actionName;
    item.target      = record.target;
    item.result      = record.result;
    item.ip          = record.ip;
    item.moduleCode  = record.moduleCode;
    item.detailJson  = record.detailJson;
    return item;
}

QVector<AuditLogListItem> AuditLogModel::list(const AuditLogQueryFilter &filter) const
{
    QVector<AuditLogListItem> items;
    if (!m_store)
        return items;

    const auto rows = m_store->auditLogs().list(filter);
    items.reserve(rows.size());
    for (const auto &r : rows)
        items.append(toListItem(r));
    return items;
}

qint64 AuditLogModel::count(const AuditLogQueryFilter &filter) const
{
    if (!m_store)
        return 0;
    return m_store->auditLogs().count(filter);
}

bool AuditLogModel::findById(qint64 id, AuditLogRecord &out) const
{
    if (!m_store || id <= 0)
        return false;

    AuditLogQueryFilter filter;
    filter.limit = 0;
    const auto rows = m_store->auditLogs().list(filter);
    for (const auto &r : rows) {
        if (r.id == id) {
            out = r;
            return true;
        }
    }
    return false;
}

} // namespace permission
