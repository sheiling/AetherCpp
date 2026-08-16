#include "repository/mysql/mysql_data_store.h"

#include "repository/orm/schema_initializer.h"

#include "repository/local/local_db_manager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace permission {
namespace {

bool isValidMysqlDatabaseName(const QString &name)
{
    if (name.isEmpty())
        return false;
    for (const QChar c : name) {
        if (!c.isLetterOrNumber() && c != QLatin1Char('_'))
            return false;
    }
    return true;
}

bool ensureMysqlDatabaseExists(const orm::DbConfig &cfg, QString *errorString)
{
    const QString dbName = cfg.database.trimmed();
    if (!isValidMysqlDatabaseName(dbName)) {
        if (errorString)
            *errorString = QStringLiteral("Invalid MySQL database name: %1").arg(dbName);
        return false;
    }

    const QString connName = QStringLiteral("permission_mysql_bootstrap");
    if (QSqlDatabase::contains(connName))
        QSqlDatabase::removeDatabase(connName);

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), connName);
    db.setHostName(cfg.host);
    if (cfg.port > 0)
        db.setPort(cfg.port);
    db.setUserName(cfg.username);
    db.setPassword(cfg.password);
    if (!cfg.connectOptions.isEmpty())
        db.setConnectOptions(cfg.connectOptions);

    if (!db.open()) {
        const QString msg =
            QStringLiteral("MySQL bootstrap connect failed: %1").arg(db.lastError().text());
        if (errorString)
            *errorString = msg;
        QSqlDatabase::removeDatabase(connName);
        return false;
    }

    const QString ddl = QStringLiteral(
                            "CREATE DATABASE IF NOT EXISTS `%1` "
                            "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")
                            .arg(dbName);
    QSqlQuery query(db);
    if (!query.exec(ddl)) {
        const QString msg =
            QStringLiteral("CREATE DATABASE failed: %1").arg(query.lastError().text());
        if (errorString)
            *errorString = msg;
        db.close();
        QSqlDatabase::removeDatabase(connName);
        return false;
    }

    db.close();
    QSqlDatabase::removeDatabase(connName);
    return true;
}

} // namespace

MysqlDataStore::MysqlDataStore(StorageConfig config)
    : OrmDataStore(std::move(config), StorageBackend::MySql)
{
}

bool MysqlDataStore::connectDatabase(QString *errorString)
{
    orm::DbConfig cfg = config().dbConfig;
    if (cfg.resolvedDriverName() != QStringLiteral("QMYSQL")) {
        if (errorString)
            *errorString = QStringLiteral("MySQL backend requires DbConfig with QMYSQL driver.");
        return false;
    }

    if (config().autoCreateDatabase) {
        if (!ensureMysqlDatabaseExists(cfg, errorString))
            return false;
    }

    if (!orm::DbManager::instance().init(cfg)) {
        if (errorString)
            *errorString = orm::DbManager::instance().lastError();
        return false;
    }
    return true;
}

bool MysqlDataStore::createSchema(QString *errorString)
{
    return SchemaInitializer::apply(StorageBackend::MySql, errorString);
}

} // namespace permission
