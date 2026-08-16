#include "repository/orm/schema_initializer.h"

#include "repository/orm/permission_sql_paths.h"

#include "repository/local/local_db_manager.h"

#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace permission {
namespace {

/// 去掉以 -- 开头的行注释，避免按 ';' 分段后误把「注释 + INSERT」整段当成注释跳过
QString stripLineComments(const QString &sql)
{
    QStringList kept;
    const QStringList lines = sql.split(QLatin1Char('\n'));
    kept.reserve(lines.size());
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QStringLiteral("--")))
            continue;
        kept.append(line);
    }
    return kept.join(QLatin1Char('\n'));
}

} // namespace

QString SchemaInitializer::schemaFilePath(StorageBackend backend, int version)
{
    const QString file = QStringLiteral("%1_schema.sql").arg(version, 3, 10, QChar(QLatin1Char('0')));
    return permissionSqlFilePath(backend, file);
}

bool SchemaInitializer::executeSqlScript(const QString &filePath, QString *errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = QStringLiteral("Cannot open schema file: %1").arg(filePath);
        if (errorString)
            *errorString = msg;
        qWarning() << "[SchemaInitializer]" << msg;
        return false;
    }

    const QString script = stripLineComments(QString::fromUtf8(file.readAll()));
    file.close();

    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        const QString msg = QStringLiteral("Database is not open for schema initialization.");
        if (errorString)
            *errorString = msg;
        return false;
    }

    const QStringList rawParts = script.split(QLatin1Char(';'));
    QSqlQuery query(db);

    for (const QString &part : rawParts) {
        const QString stmt = stripLineComments(part).trimmed();
        if (stmt.isEmpty())
            continue;

        if (!query.exec(stmt)) {
            const QString msg = QStringLiteral("Schema initialization failed: %1 | SQL: %2")
                                    .arg(query.lastError().text(), stmt.left(120));
            if (errorString)
                *errorString = msg;
            qWarning() << "[SchemaInitializer]" << msg;
            return false;
        }
    }

    return true;
}

bool SchemaInitializer::executeScript(const QString &filePath, QString *errorString)
{
    return executeSqlScript(filePath, errorString);
}

bool SchemaInitializer::apply(StorageBackend backend, QString *errorString)
{
    if (backend != StorageBackend::Sqlite && backend != StorageBackend::MySql) {
        if (errorString)
            *errorString = QStringLiteral("Schema initialization only applies to SQLite/MySQL.");
        return false;
    }

    const QString path001 = schemaFilePath(backend, 1);
    if (!QFile::exists(path001)) {
        if (errorString)
            *errorString = QStringLiteral("Schema file not found: %1").arg(path001);
        return false;
    }

    if (!executeSqlScript(path001, errorString))
        return false;

    qDebug() << "[SchemaInitializer] Schema ready for" << storageBackendName(backend);
    return true;
}

} // namespace permission
