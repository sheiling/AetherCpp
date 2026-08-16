#pragma once

#include "data/domain/domain_types.h"

#include <QString>
#include <QVector>

namespace permission {

class IPermissionResourceRepository
{
public:
    virtual ~IPermissionResourceRepository() = default;

    virtual QVector<PermissionRecord> list(const PermissionQueryFilter &filter) = 0;
    virtual QVector<PermissionRecord> tree() = 0; ///< 按 sort_order 排序的扁平列表（UI 自行组树）
    virtual bool                      findById(qint64 id, PermissionRecord &out) = 0;
    virtual bool findByKey(const QString &key, PermissionRecord &out) = 0;
    virtual bool                      insert(PermissionRecord &record, QString *errorString = nullptr) = 0;
    virtual bool                      update(const PermissionRecord &record, QString *errorString = nullptr) = 0;
    virtual bool                      remove(qint64 id, QString *errorString = nullptr) = 0;
};

} // namespace permission
