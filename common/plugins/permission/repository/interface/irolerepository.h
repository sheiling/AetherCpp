#pragma once

#include "data/domain/domain_types.h"

#include <QString>
#include <QVector>

namespace permission {

class IRoleRepository
{
public:
    virtual ~IRoleRepository() = default;

    virtual QVector<RoleRecord> list(const RoleQueryFilter &filter) = 0;
    virtual bool                findById(qint64 id, RoleRecord &out) = 0;
    virtual bool                findByCode(const QString &code, RoleRecord &out) = 0;
    virtual bool                insert(RoleRecord &record, QString *errorString = nullptr) = 0;
    virtual bool                update(const RoleRecord &record, QString *errorString = nullptr) = 0;
    virtual bool                remove(qint64 id, QString *errorString = nullptr) = 0;

    /// 替换角色的全部权限关联
    virtual bool assignPermissions(qint64 roleId,
                                   const QVector<qint64> &permissionIds,
                                   QString *errorString = nullptr) = 0;

    virtual QVector<qint64> permissionIdsOfRole(qint64 roleId) = 0;
};

} // namespace permission
