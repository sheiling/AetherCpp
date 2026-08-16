/**
 * @file    DbConfig.cpp
 * @brief   DbConfig 工厂方法与辅助函数实现
 */
#include "DbConfig.h"

namespace orm {

// ─────────────────────────────────────────────────────────────────────────────
// 工厂方法
// ─────────────────────────────────────────────────────────────────────────────

DbConfig DbConfig::sqlite(const QString &filePath, bool trace)
{
    DbConfig cfg;
    cfg.driver   = DbDriver::SQLite;
    cfg.database = filePath;
    cfg.traceSql = trace;
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
    cfg.driver   = DbDriver::MySQL;
    cfg.database = database;
    cfg.host     = host;
    cfg.port     = (port > 0 ? port : 3306);
    cfg.username = username;
    cfg.password = password;
    cfg.traceSql = trace;
    return cfg;
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助函数
// ─────────────────────────────────────────────────────────────────────────────

QString DbConfig::resolvedDriverName() const
{
    if (driver == DbDriver::Custom && !driverName.isEmpty())
        return driverName;

    switch (driver) {
    case DbDriver::SQLite:     return QStringLiteral("QSQLITE");
    case DbDriver::MySQL:      return QStringLiteral("QMYSQL");
    case DbDriver::PostgreSQL: return QStringLiteral("QPSQL");
    case DbDriver::ODBC:       return QStringLiteral("QODBC");
    default:                   return driverName;
    }
}

QString DbConfig::toString() const
{
    QString drv = resolvedDriverName();
    if (driver == DbDriver::SQLite)
        return QStringLiteral("[%1] file=%2").arg(drv, database);

    return QStringLiteral("[%1] host=%2:%3 db=%4 user=%5")
        .arg(drv, host)
        .arg(port)
        .arg(database, username);
}

} // namespace orm
