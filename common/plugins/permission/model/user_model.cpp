#include "model/user_model.h"

#ifdef interface
#undef interface
#endif

#include "repository/datastore.h"
#include "repository/interface/idatastore.h"
#include "service/perm_entity_guard.h"

#include <QCryptographicHash>

namespace permission {

UserModel::UserModel(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

QString UserModel::hashPassword(const QString &plain)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(plain.toUtf8(), QCryptographicHash::Md5).toHex());
}

bool UserModel::shouldUpdatePassword(const QString &passwordField)
{
    return !passwordField.isEmpty() && passwordField != QLatin1String(kPasswordPlaceholder);
}

QVector<UserRecord> UserModel::list(const UserQueryFilter &filter) const
{
    if (!m_store)
        return {};
    return m_store->users().list(filter);
}

qint64 UserModel::count(const UserQueryFilter &filter) const
{
    if (!m_store)
        return 0;
    return m_store->users().count(filter);
}

bool UserModel::findById(qint64 id, UserRecord &out) const
{
    if (!m_store)
        return false;
    return m_store->users().findById(id, out);
}

OperationResult UserModel::createUser(const UserEditorForm &form, const RoleRecord &role)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }
    if (!canAssignRoleCode(role.code)) {
        result.message = QStringLiteral("role_not_assignable");
        return result;
    }

    UserRecord u;
    u.username     = form.username;
    u.displayName  = form.displayName;
    u.department   = form.department;
    u.remark       = form.remark;
    u.accountStatus = form.accountStatus;

    QString err;
    if (!m_store->users().insert(u, &err)) {
        result.message = err;
        return result;
    }
    if (!m_store->users().resetPassword(u.id, hashPassword(form.password), &err)) {
        result.message = err;
        return result;
    }
    if (!m_store->users().assignRoles(u.id, {role.id}, role.id, &err)) {
        result.message = err;
        return result;
    }

    result.ok = true;
    return result;
}

OperationResult UserModel::updateUser(const UserEditorForm &form, const RoleRecord &role)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }

    UserRecord rec;
    if (!m_store->users().findById(form.editingUserId, rec)) {
        result.message = QStringLiteral("User not found");
        return result;
    }

    rec.displayName = form.displayName;
    rec.department  = form.department;
    rec.remark      = form.remark;
    if (!canAssignRoleCode(role.code)) {
        result.message = QStringLiteral("role_not_assignable");
        return result;
    }
    if (!canChangeUserAccountStatus(rec) && form.accountStatus != rec.accountStatus) {
        result.message = QStringLiteral("protected_user_status");
        return result;
    }
    rec.accountStatus = form.accountStatus;

    QString err;
    if (!m_store->users().update(rec, &err)) {
        result.message = err;
        return result;
    }
    if (shouldUpdatePassword(form.password)
        && !m_store->users().resetPassword(form.editingUserId, hashPassword(form.password), &err)) {
        result.message = err;
        return result;
    }
    if (!m_store->users().assignRoles(form.editingUserId, {role.id}, role.id, &err)) {
        result.message = err;
        return result;
    }

    result.ok = true;
    return result;
}

OperationResult UserModel::resetPassword(qint64 userId, const QString &plainPassword)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }
    QString err;
    if (!m_store->users().resetPassword(userId, hashPassword(plainPassword), &err)) {
        result.message = err;
        return result;
    }
    result.ok = true;
    return result;
}

OperationResult UserModel::setAccountStatus(qint64 userId, int accountStatus)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }
    UserRecord rec;
    if (findById(userId, rec) && !canChangeUserAccountStatus(rec)) {
        result.message = QStringLiteral("protected_user_status");
        return result;
    }
    QString err;
    if (!m_store->users().setAccountStatus(userId, accountStatus, &err)) {
        result.message = err;
        return result;
    }
    result.ok = true;
    return result;
}

OperationResult UserModel::assignRoles(qint64 userId, qint64 roleId)
{
    OperationResult result;
    if (!m_store) {
        result.message = QStringLiteral("Data store unavailable");
        return result;
    }
    RoleRecord role;
    if (m_store->roles().findById(roleId, role) && !canAssignRoleCode(role.code)) {
        result.message = QStringLiteral("role_not_assignable");
        return result;
    }
    QString err;
    if (!m_store->users().assignRoles(userId, {roleId}, roleId, &err)) {
        result.message = err;
        return result;
    }
    result.ok = true;
    return result;
}

} // namespace permission
