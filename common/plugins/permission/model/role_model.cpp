#include "model/role_model.h"

#ifdef interface
#undef interface
#endif

#include "model/permission_grant_service.h"
#include "repository/datastore.h"
#include "repository/interface/idatastore.h"
#include "service/perm_entity_guard.h"
#include "service/perm_system_constants.h"

#include <QDateTime>

namespace permission {
namespace {

QString slugifyRoleCode(const QString &name)
{
    QString s;
    for (const QChar c : name) {
        if (c.isLetterOrNumber())
            s += c.toLower();
        else if (c.isSpace() || c == QLatin1Char('_') || c == QLatin1Char('-'))
            s += QLatin1Char('_');
    }
    while (s.contains(QStringLiteral("__")))
        s.replace(QStringLiteral("__"), QStringLiteral("_"));
    if (s.startsWith(QLatin1Char('_')))
        s = s.mid(1);
    if (s.endsWith(QLatin1Char('_')))
        s.chop(1);
    if (s.isEmpty())
        s = QStringLiteral("custom");
    if (!s.isEmpty() && s.at(0).isDigit())
        s = QStringLiteral("role_") + s;
    return s.left(64);
}

} // namespace

RoleModel::RoleModel(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

void RoleModel::reload()
{
    m_byCode.clear();
    if (!m_store)
        return;
    RoleQueryFilter filter;
    filter.includeHidden = false;
    for (const auto &r : m_store->roles().list(filter)) {
        if (!r.isHidden)
            m_byCode.insert(r.code, r);
    }
}

QVector<RoleRecord> RoleModel::allRoles() const
{
    return m_byCode.values().toVector();
}

QString RoleModel::displayName(const QString &code) const
{
    return m_byCode.value(code).name;
}

QString RoleModel::tagColor(const QString &code) const
{
    const QString c = m_byCode.value(code).tagColor;
    return c.isEmpty() ? QStringLiteral("#6b7280") : c;
}

bool RoleModel::findByCode(const QString &code, RoleRecord &out) const
{
    if (!m_byCode.contains(code))
        return false;
    out = m_byCode.value(code);
    return true;
}

QVector<RoleRecord> RoleModel::listRoles() const
{
    if (!m_store)
        return {};
    return m_store->roles().list({});
}

bool RoleModel::findById(qint64 id, RoleRecord &out) const
{
    if (!m_store)
        return false;
    return m_store->roles().findById(id, out);
}

bool RoleModel::isRoleNameTaken(const QString &name, qint64 excludeRoleId) const
{
    if (!m_store)
        return false;

    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return false;

    RoleQueryFilter filter;
    filter.includeHidden   = true;
    filter.includeDisabled = true;
    for (const auto &r : m_store->roles().list(filter)) {
        if (r.id == excludeRoleId)
            continue;
        if (r.name.compare(trimmed, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

OperationResult RoleModel::createRole(const RoleCreateForm &form, qint64 *createdIdOut)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }

    const QString name = form.name.trimmed();
    if (name.isEmpty()) {
        result.message = QStringLiteral("role_name_required");
        return result;
    }
    if (isRoleNameTaken(name, 0)) {
        result.message = QStringLiteral("role_name_duplicate");
        return result;
    }

    QString code = form.code.trimmed();
    if (code.isEmpty()) {
        const QString base = slugifyRoleCode(name);
        code               = base;
        int suffix         = 2;
        RoleRecord existing;
        while (m_store->roles().findByCode(code, existing)) {
            code = QStringLiteral("%1_%2").arg(base).arg(suffix++);
        }
    }

    RoleRecord r;
    r.code        = code;
    r.name        = name;
    r.description = form.description.trimmed();
    r.tagColor    = form.tagColor;

    QString err;
    if (!m_store->roles().insert(r, &err)) {
        result.message = err.isEmpty() ? QStringLiteral("insert_failed") : err;
        return result;
    }

    RoleRecord minimal;
    if (m_store->roles().findByCode(QString::fromUtf8(kRoleCodeMinimal), minimal)) {
        const QVector<qint64> minimalIds = m_store->roles().permissionIdsOfRole(minimal.id);
        if (!minimalIds.isEmpty()
            && !m_store->roles().assignPermissions(r.id, minimalIds, &err)) {
            result.message =
                err.isEmpty() ? QStringLiteral("copy_minimal_grants_failed") : err;
            return result;
        }
    }

    reload();
    if (createdIdOut)
        *createdIdOut = r.id;
    result.ok = true;
    return result;
}

OperationResult RoleModel::updateRole(const RoleRecord &record)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }

    const QString name = record.name.trimmed();
    if (name.isEmpty()) {
        result.message = QStringLiteral("role_name_required");
        return result;
    }
    if (isRoleNameTaken(name, record.id)) {
        result.message = QStringLiteral("role_name_duplicate");
        return result;
    }

    QString err;
    if (!m_store->roles().update(record, &err)) {
        result.message = err;
        return result;
    }

    reload();
    result.ok = true;
    return result;
}

QVector<PermissionRecord> RoleModel::permissionTree() const
{
    if (!m_store)
        return {};
    PermissionQueryFilter filter;
    filter.includeDisabled = true;
    return m_store->permissions().list(filter);
}

QVector<qint64> RoleModel::grantedPermissionIds(qint64 roleId) const
{
    if (!m_store)
        return {};
    return m_store->roles().permissionIdsOfRole(roleId);
}

OperationResult RoleModel::saveRolePermissions(qint64 roleId,
                                               const QVector<qint64> &permissionIds)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }

    RoleRecord role;
    if (!findById(roleId, role)) {
        result.message = QStringLiteral("Role not found");
        return result;
    }
    if (isSuperAdminRole(role)) {
        result.message = QStringLiteral("protected_role_grants");
        return result;
    }

    const QVector<PermissionRecord> allPerms = permissionTree();
    const GrantNormalizeResult norm = PermissionGrantService::normalize(allPerms, permissionIds);

    QString err;
    if (!m_store->roles().assignPermissions(roleId, norm.normalized, &err)) {
        result.message = err;
        return result;
    }

    result.ok = true;
    if (!norm.removedIds.isEmpty()) {
        result.info = QStringLiteral("pruned_invalid_grants:%1").arg(norm.removedIds.size());
    }
    return result;
}

OperationResult syncSuperAdminGrantsFromCatalog(IDataStore *store)
{
    OperationResult result;
    IDataStore *const dataStore = store ? store : DataStore::instance();
    if (!dataStore) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }

    RoleRecord role;
    if (!dataStore->roles().findByCode(QString::fromUtf8(kRoleCodeSuperAdmin), role)) {
        result.message = QStringLiteral("super_admin role not found");
        return result;
    }

    PermissionQueryFilter filter;
    filter.includeDisabled = true;
    const QVector<PermissionRecord> allPerms = dataStore->permissions().list(filter);

    QVector<qint64> ids;
    ids.reserve(allPerms.size());
    for (const auto &p : allPerms) {
        if (p.id > 0)
            ids.append(p.id);
    }

    QString err;
    if (!dataStore->roles().assignPermissions(role.id, ids, &err)) {
        result.message = err.isEmpty() ? QStringLiteral("sync_super_admin_grants_failed") : err;
        return result;
    }

    result.ok = true;
    return result;
}

} // namespace permission
