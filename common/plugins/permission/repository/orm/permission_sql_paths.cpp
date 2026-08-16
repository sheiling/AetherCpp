#include "repository/orm/permission_sql_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

namespace permission {
namespace {

bool sqlBundleExists(const QString &root)
{
    const QDir dir(root);
    return QFile::exists(dir.filePath(QStringLiteral("sqlite/001_schema.sql")))
           || QFile::exists(dir.filePath(QStringLiteral("mysql/001_schema.sql")));
}

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

QString permissionSqlRootPath()
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

QString permissionSqlFilePath(StorageBackend backend, const QString &fileName)
{
    const QString root = permissionSqlRootPath();
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

} // namespace permission
