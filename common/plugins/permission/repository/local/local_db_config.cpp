/**
 * @file    local_db_config.cpp
 * @brief   DbConfig 方法实现（无 QxOrm 依赖）
 */
#include "local_db_config.h"

#include <QSqlDatabase>

namespace orm {

// ─────────────────────────────────────────────────────────────────────────────
// 工厂方法
// ─────────────────────────────────────────────────────────────────────────────

DbConfig DbConfig::sqlite(const QString &filePath, bool trace)
{
    DbConfig cfg;
    cfg.driver       = DbDriver::SQLite;
    cfg.database     = filePath;
    cfg.traceSql     = trace;
    cfg.traceSqlError = true;
    return cfg;
}

DbConfig DbConfig::mysql(const QString &database,
                         const QString &host,
                         int            port,
                         const QString &username,
                         const QString &password,
                         bool           trace)
{
    DbConfig cfg;
    cfg.driver        = DbDriver::MySQL;
    cfg.database      = database;
    cfg.host          = host;
    cfg.port          = port;
    cfg.username      = username;
    cfg.password      = password;
    cfg.traceSql      = trace;
    cfg.traceSqlError = true;
    return cfg;
}

// ─────────────────────────────────────────────────────────────────────────────
// 查询方法
// ─────────────────────────────────────────────────────────────────────────────

QString DbConfig::resolvedDriverName() const
{
    if (!driverName.isEmpty())
        return driverName;

    switch (driver) {
    case DbDriver::SQLite:
        return QStringLiteral("QSQLITE");
    case DbDriver::MySQL:
        return QStringLiteral("QMYSQL");
    case DbDriver::PostgreSQL:
        return QStringLiteral("QPSQL");
    case DbDriver::ODBC:
        return QStringLiteral("QODBC");
    case DbDriver::Custom:
        // 如果 driver 是 Custom 但 driverName 为空，返回空
        return {};
    }
    return {};
}

QString DbConfig::toString() const
{
    QString s;
    switch (driver) {
    case DbDriver::SQLite:
        s = QStringLiteral("SQLite:");
        break;
    case DbDriver::MySQL:
        s = QStringLiteral("MySQL:");
        break;
    case DbDriver::PostgreSQL:
        s = QStringLiteral("PostgreSQL:");
        break;
    case DbDriver::ODBC:
        s = QStringLiteral("ODBC:");
        break;
    case DbDriver::Custom:
        s = QStringLiteral("Custom:");
        break;
    }

    s += database;
    if (driver != DbDriver::SQLite) {
        s += QStringLiteral("@");
        s += host;
        if (port > 0)
            s += QStringLiteral(":%1").arg(port);
    }
    return s;
}

} // namespace orm
