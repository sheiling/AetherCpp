/**
 * @file schema_initializer.cpp
 * @brief SchemaInitializer SQL 脚本解析与执行
 */

#include "repository/orm/schema_initializer.h"

#include "repository/orm/db_connection.h"
#include "repository/orm/sql_paths.h"

#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

/** @brief SchemaInitializer SQL 脚本解析与幂等建表实现 */
namespace lighten {
namespace {

/** 去掉 -- 行注释，避免按分号分段时误跳过 INSERT 等语句 */
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

bool SchemaInitializer::executeSqlScript(const QString &filePath, QString *errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = QStringLiteral("Cannot open schema file: %1").arg(filePath);
        if (errorString)
            *errorString = msg;
        qWarning() << "[LightenSchema]" << msg;
        return false;
    }

    const QString script = stripLineComments(QString::fromUtf8(file.readAll()));
    file.close();

    QSqlDatabase db = DbConnection::database();
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
            qWarning() << "[LightenSchema]" << msg;
            return false;
        }
    }

    return true;
}

bool SchemaInitializer::apply(StorageBackend backend, QString *errorString)
{
    if (backend != StorageBackend::Sqlite && backend != StorageBackend::MySql) {
        if (errorString)
            *errorString = QStringLiteral("Schema initialization only applies to SQLite/MySQL.");
        return false;
    }

    const QStringList schemaFiles = {
        QStringLiteral("004_lighten_inspection_history_schema.sql"),
        QStringLiteral("005_lighten_production_kpi_schema.sql"),
    };

    for (const QString &fileName : schemaFiles) {
        const QString path = inspectionSqlFilePath(backend, fileName);
        if (!QFile::exists(path)) {
            if (errorString)
                *errorString = QStringLiteral("Schema file not found: %1").arg(path);
            return false;
        }
        if (!executeSqlScript(path, errorString))
            return false;
    }

    qDebug() << "[LightenSchema] Schema ready for" << storageBackendName(backend);
    return true;
}

} // namespace lighten
