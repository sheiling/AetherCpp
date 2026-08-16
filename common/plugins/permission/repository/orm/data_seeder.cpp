#include "repository/orm/data_seeder.h"

#include "repository/orm/permission_sql_paths.h"
#include "repository/orm/schema_initializer.h"
#include "service/perm_system_constants.h"

#include "repository/local/local_db_manager.h"

#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace permission {
namespace {

bool roleCodeExists(const QString &code, QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed check.");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT 1 FROM perm_role WHERE code = ? LIMIT 1"));
    query.addBindValue(code);
    if (!query.exec()) {
        if (errorString)
            *errorString = query.lastError().text();
        return false;
    }
    return query.next();
}

bool usernameExists(const QString &username, QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed check.");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT 1 FROM perm_user WHERE username = ? LIMIT 1"));
    query.addBindValue(username);
    if (!query.exec()) {
        if (errorString)
            *errorString = query.lastError().text();
        return false;
    }
    return query.next();
}

bool permissionKeyExists(const QString &key, QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed check.");
        return false;
    }

    const bool isMysql = db.driverName() == QLatin1String("QMYSQL");
    const QString sql = isMysql
                            ? QStringLiteral("SELECT 1 FROM perm_permission WHERE `key` = ? LIMIT 1")
                            : QStringLiteral("SELECT 1 FROM perm_permission WHERE key = ? LIMIT 1");

    QSqlQuery query(db);
    query.prepare(sql);
    query.addBindValue(key);
    if (!query.exec()) {
        if (errorString)
            *errorString = query.lastError().text();
        return false;
    }
    return query.next();
}

bool seededRoleGrantsExist(QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed check.");
        return false;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "SELECT COUNT(DISTINCT rp.role_id) FROM perm_role_permission rp "
        "INNER JOIN perm_role r ON r.id = rp.role_id "
        "WHERE r.code IN ('super_admin', 'minimal')");
    if (!query.exec(sql)) {
        if (errorString)
            *errorString = query.lastError().text();
        return false;
    }
    if (!query.next())
        return false;
    return query.value(0).toLongLong() >= 2;
}

bool isUsersAndRolesSeedComplete(QString *errorString)
{
    if (!roleCodeExists(QString::fromUtf8(kRoleCodeSuperAdmin), errorString))
        return false;
    if (!roleCodeExists(QString::fromUtf8(kRoleCodeMinimal), errorString))
        return false;
    if (!usernameExists(QStringLiteral("admin"), errorString))
        return false;
    if (!usernameExists(QString::fromUtf8(kMinimalUsername), errorString))
        return false;
    return true;
}

bool isPermissionsSeedComplete(QString *errorString)
{
    if (!permissionKeyExists(QStringLiteral("nav.permission"), errorString))
        return false;
    return seededRoleGrantsExist(errorString);
}

bool hasUsersAndRolesArtifacts(QString *errorString)
{
    const qint64 roles = DataSeeder::tableRowCount(QStringLiteral("perm_role"), errorString);
    if (roles < 0)
        return false;
    const qint64 users = DataSeeder::tableRowCount(QStringLiteral("perm_user"), errorString);
    if (users < 0)
        return false;
    const qint64 links = DataSeeder::tableRowCount(QStringLiteral("perm_user_role"), errorString);
    if (links < 0)
        return false;
    return roles > 0 || users > 0 || links > 0;
}

bool hasPermissionArtifacts(QString *errorString)
{
    const qint64 perms = DataSeeder::tableRowCount(QStringLiteral("perm_permission"), errorString);
    if (perms < 0)
        return false;
    const qint64 grants = DataSeeder::tableRowCount(QStringLiteral("perm_role_permission"), errorString);
    if (grants < 0)
        return false;
    return perms > 0 || grants > 0;
}

bool executeSeedScript(const QString &filePath, QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed apply.");
        return false;
    }

    if (!db.transaction()) {
        if (errorString)
            *errorString = db.lastError().text();
        return false;
    }

    if (!SchemaInitializer::executeScript(filePath, errorString)) {
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        if (errorString)
            *errorString = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool applySeedBundle(StorageBackend backend, const QString &seedFileName,
                     bool (*isComplete)(QString *), bool (*hasArtifacts)(QString *),
                     QString *errorString)
{
    if (backend != StorageBackend::Sqlite && backend != StorageBackend::MySql)
        return true;

    QString completeCheckErr;
    QString *const completeErrOut = errorString ? errorString : &completeCheckErr;
    if (isComplete(completeErrOut)) {
        qDebug() << "[DataSeeder] Seed bundle already complete, skip" << seedFileName;
        return true;
    }
    if (!completeErrOut->isEmpty())
        return false;

    if (hasArtifacts(errorString)) {
        const QString msg =
            QStringLiteral("Inconsistent seed state for %1; clear related tables and retry.")
                .arg(seedFileName);
        if (errorString)
            *errorString = msg;
        qWarning() << "[DataSeeder]" << msg;
        return false;
    }

    const QString path = permissionSqlFilePath(backend, seedFileName);
    if (path.isEmpty() || !QFile::exists(path)) {
        const QString msg = QStringLiteral("Seed file not found: %1").arg(path);
        if (errorString)
            *errorString = msg;
        qWarning() << "[DataSeeder]" << msg;
        return false;
    }

    if (!executeSeedScript(path, errorString))
        return false;

    qDebug() << "[DataSeeder] Applied" << seedFileName << "for" << storageBackendName(backend);
    return true;
}

} // namespace

qint64 DataSeeder::tableRowCount(const QString &tableName, QString *errorString)
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (errorString)
            *errorString = QStringLiteral("Database is not open for seed check.");
        return -1;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(tableName))) {
        if (errorString)
            *errorString = query.lastError().text();
        return -1;
    }
    if (!query.next())
        return 0;
    return query.value(0).toLongLong();
}

QString DataSeeder::seedFilePath(StorageBackend backend, const QString &fileName)
{
    return permissionSqlFilePath(backend, fileName);
}

bool DataSeeder::applyUsersAndRolesIfEmpty(StorageBackend backend, QString *errorString)
{
    return applySeedBundle(backend, QStringLiteral("002_seed_users.sql"), isUsersAndRolesSeedComplete,
                           hasUsersAndRolesArtifacts, errorString);
}

bool DataSeeder::applyPermissionsIfEmpty(StorageBackend backend, QString *errorString)
{
    return applySeedBundle(backend, QStringLiteral("003_seed_permissions.sql"),
                           isPermissionsSeedComplete, hasPermissionArtifacts, errorString);
}

} // namespace permission
