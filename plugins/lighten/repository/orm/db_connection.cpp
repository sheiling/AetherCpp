/**
 * @file db_connection.cpp
 * @brief DbConnection 打开/关闭与 MySQL 自动建库实现
 */

#include "repository/orm/db_connection.h"

#include <DbConfig.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/** @brief DbConnection 独立连接打开/关闭实现 */
namespace lighten {
namespace {

/** 校验 MySQL 库名（字母数字下划线） */
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

/** MySQL 建库引导连接（无库名，仅 CREATE DATABASE） */
bool ensureMysqlDatabaseExists(const orm::DbConfig &cfg, QString *errorString)
{
    const QString dbName = cfg.database.trimmed();
    if (!isValidMysqlDatabaseName(dbName)) {
        if (errorString)
            *errorString = QStringLiteral("Invalid MySQL database name: %1").arg(dbName);
        return false;
    }

    const QString connName = QStringLiteral("lighten_mysql_bootstrap");
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

QString DbConnection::connectionName()
{
    return QStringLiteral("lighten_inspection_storage");
}

bool DbConnection::open(const StorageConfig &config, QString *errorString)
{
    close();

    orm::DbConfig cfg = config.dbConfig;
    if (config.backend == StorageBackend::Sqlite) {
        if (cfg.database.isEmpty())
            cfg = orm::DbConfig::sqlite(config.sqliteFilePath, cfg.traceSql);
        QFileInfo fi(cfg.database);
        QDir().mkpath(fi.absolutePath());
    }

    if (config.backend == StorageBackend::MySql && config.autoCreateDatabase) {
        if (!ensureMysqlDatabaseExists(cfg, errorString))
            return false;
    }

    const QString conn = connectionName();
    if (QSqlDatabase::contains(conn))
        QSqlDatabase::removeDatabase(conn);

    QSqlDatabase db = QSqlDatabase::addDatabase(cfg.resolvedDriverName(), conn);
    db.setDatabaseName(cfg.database);
    if (cfg.driver != orm::DbDriver::SQLite) {
        db.setHostName(cfg.host);
        if (cfg.port > 0)
            db.setPort(cfg.port);
        db.setUserName(cfg.username);
        db.setPassword(cfg.password);
    }
    if (!cfg.connectOptions.isEmpty())
        db.setConnectOptions(cfg.connectOptions);

    if (!db.open()) {
        const QString msg = QStringLiteral("Database open failed: %1").arg(db.lastError().text());
        if (errorString)
            *errorString = msg;
        QSqlDatabase::removeDatabase(conn);
        return false;
    }

    qDebug() << "[LightenDb] open OK:" << cfg.toString();
    return true;
}

void DbConnection::close()
{
    const QString conn = connectionName();
    if (!QSqlDatabase::contains(conn))
        return;
    {
        QSqlDatabase db = QSqlDatabase::database(conn);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(conn);
}

bool DbConnection::isOpen()
{
    if (!QSqlDatabase::contains(connectionName()))
        return false;
    return QSqlDatabase::database(connectionName()).isOpen();
}

QSqlDatabase DbConnection::database()
{
    return QSqlDatabase::database(connectionName());
}

} // namespace lighten
