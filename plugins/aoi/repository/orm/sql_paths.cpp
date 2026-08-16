/**
 * @file sql_paths.cpp
 * @brief aoiSqlRootPath / aoiSqliteFilePath 实现
 */

#include "repository/orm/sql_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

namespace aoi::repository {
namespace {

bool schemaBundleExists(const QString &root)
{
    return QFile::exists(QDir(root).filePath(
        QStringLiteral("sqlite/006_aoi_inspection_history_schema.sql")));
}

QString findProjectRootSqlPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; ++i) {
        const QString candidate = dir.filePath(QStringLiteral("config/sql"));
        if (schemaBundleExists(candidate))
            return candidate;
        if (!dir.cdUp())
            break;
    }
    return {};
}

} // namespace

QString aoiSqlRootPath()
{
    const QString exeRelative =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config/sql"));

#ifdef QT_DEBUG
    const QString projectSql = findProjectRootSqlPath();
    if (!projectSql.isEmpty())
        return projectSql;
#endif

    if (schemaBundleExists(exeRelative))
        return exeRelative;

    const QString fallback = findProjectRootSqlPath();
    if (!fallback.isEmpty())
        return fallback;

    return exeRelative;
}

QString aoiSqliteFilePath(const QString &fileName)
{
    const QString root = aoiSqlRootPath();
    if (root.isEmpty())
        return {};
    return QDir(root).filePath(QStringLiteral("sqlite/") + fileName);
}

} // namespace aoi::repository
