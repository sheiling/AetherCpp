/**
 * @file sql_paths.cpp
 * @brief inspectionSqlRootPath / inspectionSqlFilePath 实现
 */

#include "repository/orm/sql_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

/** @brief config/sql 路径解析实现 */
namespace lighten {
namespace {

/** 判断目录下是否包含检测历史与 KPI schema 脚本 */
bool sqlBundleExists(const QString &root)
{
    const QDir dir(root);
    const bool sqliteBundle =
        QFile::exists(dir.filePath(QStringLiteral("sqlite/004_lighten_inspection_history_schema.sql")))
        && QFile::exists(dir.filePath(QStringLiteral("sqlite/005_lighten_production_kpi_schema.sql")));
    const bool mysqlBundle =
        QFile::exists(dir.filePath(QStringLiteral("mysql/004_lighten_inspection_history_schema.sql")))
        && QFile::exists(dir.filePath(QStringLiteral("mysql/005_lighten_production_kpi_schema.sql")));
    return sqliteBundle || mysqlBundle;
}

/** 自 exe 目录向上查找含 config/sql 的工程根 */
QString findProjectRootSqlPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; ++i) {
        const QString candidate = dir.filePath(QStringLiteral("config/sql"));
        if (sqlBundleExists(candidate))
            return candidate;
        if (!dir.cdUp())
            break;
    }
    return {};
}

} // namespace

QString inspectionSqlRootPath()
{
    const QString exeRelative =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config/sql"));

#ifdef QT_DEBUG
    const QString projectSql = findProjectRootSqlPath();
    if (!projectSql.isEmpty())
        return projectSql;
#endif

    if (sqlBundleExists(exeRelative))
        return exeRelative;

    const QString fallback = findProjectRootSqlPath();
    if (!fallback.isEmpty())
        return fallback;

    return exeRelative;
}

QString inspectionSqlFilePath(StorageBackend backend, const QString &fileName)
{
    const QString root = inspectionSqlRootPath();
    if (root.isEmpty())
        return {};

    switch (backend) {
    case StorageBackend::Sqlite:
        return QDir(root).filePath(QStringLiteral("sqlite/") + fileName);
    case StorageBackend::MySql:
        return QDir(root).filePath(QStringLiteral("mysql/") + fileName);
    default:
        return {};
    }
}

} // namespace lighten
