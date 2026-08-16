#include "service/perm_permission_backend.h"

#include "data/domain/domain_types.h"
#include "model/auth_service.h"
#include "repository/datastore.h"

namespace permission {

PermPermissionBackend::PermPermissionBackend(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

bool PermPermissionBackend::isReady() const
{
    return m_store != nullptr;
}

QVector<PermissionRecord> PermPermissionBackend::catalog() const
{
    if (!m_store)
        return {};
    return m_store->permissions().tree();
}

QSet<QString> PermPermissionBackend::expandGrantKeys(const QVector<PermissionRecord> &all,
                                                     const QVector<qint64>           &grantedIds)
{
    QHash<qint64, PermissionRecord> byId;
    byId.reserve(all.size());
    for (const PermissionRecord &r : all) {
        if (!r.enabled)
            continue;
        byId.insert(r.id, r);
    }

    QSet<QString> grants;
    for (qint64 id : grantedIds) {
        const auto it = byId.constFind(id);
        if (it == byId.constEnd())
            continue;
        grants.insert(it.value().key);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        const QSet<QString> snapshot = grants;
        for (const QString &key : snapshot) {
            qint64 parentId = 0;
            for (const PermissionRecord &r : all) {
                if (r.key == key) {
                    parentId = r.parentId;
                    break;
                }
            }
            if (parentId <= 0)
                continue;
            const auto pit = byId.constFind(parentId);
            if (pit == byId.constEnd() || !pit->enabled)
                continue;
            const QString parentKey = pit.value().key;
            if (!grants.contains(parentKey)) {
                grants.insert(parentKey);
                changed = true;
            }
        }
    }

    return grants;
}

QSet<QString> PermPermissionBackend::loadGrantKeysForRole(qint64 roleId)
{
    if (!m_store || roleId <= 0)
        return {};

    const QVector<PermissionRecord> all     = catalog();
    const QVector<qint64>           granted = m_store->roles().permissionIdsOfRole(roleId);
    return expandGrantKeys(all, granted);
}

bool PermPermissionBackend::primaryRoleIdForUser(qint64 userId, qint64 *roleIdOut) const
{
    if (!roleIdOut || !m_store || userId <= 0)
        return false;

    UserRecord user;
    if (!m_store->users().findById(userId, user))
        return false;

    if (user.primaryRoleId > 0) {
        *roleIdOut = user.primaryRoleId;
        return true;
    }

    const QVector<qint64> roleIds = m_store->users().roleIdsOfUser(userId);
    if (roleIds.isEmpty())
        return false;

    *roleIdOut = roleIds.first();
    return true;
}

bool PermPermissionBackend::findUserIdByRoleCode(const QString &roleCode,
                                                qint64       *userIdOut) const
{
    if (!userIdOut || !m_store || roleCode.isEmpty())
        return false;

    RoleRecord role;
    if (!m_store->roles().findByCode(roleCode, role))
        return false;

    UserQueryFilter filter;
    filter.roleCode = roleCode;
    filter.limit    = 1000;
    const QVector<UserRecord> users = m_store->users().list(filter);
    for (const UserRecord &u : users) {
        if (u.primaryRoleId == role.id || u.roleCodes.contains(roleCode)) {
            *userIdOut = u.id;
            return true;
        }
    }
    return false;
}

void PermPermissionBackend::clearAuthSession()
{
    AuthService::clearSession();
}

} // namespace permission
