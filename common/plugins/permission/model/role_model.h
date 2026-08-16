#pragma once

#include "data/domain/domain_types.h"
#include "model/permission_result.h"
#include "model/role_management_model.h"

#include <QMap>
#include <QString>
#include <QVector>

namespace permission {

class IDataStore;

/// 角色业务 Model（无 Q_OBJECT）
class RoleModel
{
public:
    explicit RoleModel(IDataStore *store = nullptr);

    void reload();
    QVector<RoleRecord> allRoles() const;
    const QMap<QString, RoleRecord> &rolesByCode() const { return m_byCode; }

    QString displayName(const QString &code) const;
    QString tagColor(const QString &code) const;
    bool    findByCode(const QString &code, RoleRecord &out) const;

    QVector<RoleRecord>         listRoles() const;
    bool                        findById(qint64 id, RoleRecord &out) const;
    bool                        isRoleNameTaken(const QString &name, qint64 excludeRoleId = 0) const;
    OperationResult             createRole(const RoleCreateForm &form, qint64 *createdIdOut = nullptr);
    OperationResult             updateRole(const RoleRecord &record);
    QVector<PermissionRecord>   permissionTree() const;
    QVector<qint64>             grantedPermissionIds(qint64 roleId) const;
    OperationResult             saveRolePermissions(qint64 roleId,
                                                      const QVector<qint64> &permissionIds);

private:
    IDataStore                *m_store = nullptr;
    QMap<QString, RoleRecord>  m_byCode;
};

/// 将 super_admin 角色授权与 perm_permission 全表对齐（方案 A，资源变更后调用）
OperationResult syncSuperAdminGrantsFromCatalog(IDataStore *store = nullptr);

} // namespace permission
