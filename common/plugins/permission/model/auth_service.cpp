#include "model/auth_service.h"

#include "data/domain/domain_types.h"
#include "model/user_model.h"
#include "repository/datastore.h"
#include "repository/interface/idatastore.h"

#ifdef interface
#undef interface
#endif

#include "repository/local/local_db_manager.h"

#include <QSqlQuery>

#include <QHash>

namespace permission {
namespace {

AuthSession g_session;

QString mockPasswordHashFor(const QString &username)
{
    static const QHash<QString, QString> kTable = []() {
        QHash<QString, QString> m;
        m.insert(QStringLiteral("admin"), UserModel::hashPassword(QStringLiteral("admin")));
        m.insert(QStringLiteral("zhang.san"), UserModel::hashPassword(QStringLiteral("123456")));
        m.insert(QStringLiteral("lisi"), UserModel::hashPassword(QStringLiteral("123456")));
        m.insert(QStringLiteral("wang.wu"), UserModel::hashPassword(QStringLiteral("123456")));
        m.insert(QStringLiteral("minimal"), UserModel::hashPassword(QStringLiteral("123456")));
        return m;
    }();
    return kTable.value(username);
}

bool ormPasswordHashFor(const QString &username, QString &hashOut)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT password_hash FROM perm_user WHERE username = ?"));
    q.addBindValue(username);
    if (!q.exec() || !q.next())
        return false;
    hashOut = q.value(0).toString();
    return !hashOut.isEmpty();
}

} // namespace

AuthService::AuthService(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

AuthResult AuthService::login(const QString &username, const QString &plainPassword) const
{
    AuthResult result;
    const QString user = username.trimmed();
    if (user.isEmpty()) {
        result.message = QStringLiteral("username_required");
        return result;
    }
    if (plainPassword.isEmpty()) {
        result.message = QStringLiteral("password_required");
        return result;
    }

    UserRecord record;
    if (!verifyPassword(user, plainPassword, record)) {
        result.message = QStringLiteral("invalid_credentials");
        return result;
    }
    if (record.accountStatus != 1) {
        result.message = QStringLiteral("account_disabled");
        return result;
    }

    result.ok                  = true;
    result.session.userId      = record.id;
    result.session.username    = record.username;
    result.session.displayName = record.displayName;
    result.session.roleCodes   = record.roleCodes;
    result.session.roleId      = record.primaryRoleId;

    if (result.session.roleId > 0 && m_store) {
        RoleRecord role;
        if (m_store->roles().findById(result.session.roleId, role))
            result.session.primaryRoleCode = role.code;
    }
    if (result.session.primaryRoleCode.isEmpty() && !record.roleCodes.isEmpty())
        result.session.primaryRoleCode = record.roleCodes.first();

    return result;
}

bool AuthService::verifyPassword(const QString &username, const QString &plainPassword,
                                 UserRecord &userOut) const
{
    if (!m_store || !m_store->users().findByUsername(username, userOut))
        return false;

    const QString expectedHash = UserModel::hashPassword(plainPassword);
    if (DataStore::currentConfig().backend == StorageBackend::Mock) {
        const QString mockHash = mockPasswordHashFor(username);
        return !mockHash.isEmpty() && mockHash == expectedHash;
    }

    QString storedHash;
    if (!ormPasswordHashFor(username, storedHash))
        return false;
    return storedHash == expectedHash;
}

AuthSession AuthService::currentSession() { return g_session; }

void AuthService::setCurrentSession(const AuthSession &session) { g_session = session; }

void AuthService::clearSession() { g_session = {}; }

} // namespace permission
