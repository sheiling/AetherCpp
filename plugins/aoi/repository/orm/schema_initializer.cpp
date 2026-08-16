/**
 * @file schema_initializer.cpp
 * @brief SchemaInitializer SQL 脚本解析与执行
 */

#include "repository/orm/schema_initializer.h"

#include "repository/orm/sql_paths.h"

#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace aoi::repository {
namespace {

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

bool SchemaInitializer::executeSqlScript(QSqlDatabase &db, const QString &filePath, QString *errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = QStringLiteral("Cannot open schema file: %1").arg(filePath);
        if (errorString)
            *errorString = msg;
        qWarning() << "[AoiSchema]" << msg;
        return false;
    }

    const QString script = stripLineComments(QString::fromUtf8(file.readAll()));
    file.close();

    if (!db.isValid() || !db.isOpen()) {
        const QString msg = QStringLiteral("Database is not open for schema initialization.");
        if (errorString)
            *errorString = msg;
        return false;
    }

    const QStringList rawParts = script.split(QLatin1Char(';'));
    QSqlQuery         query(db);

    for (const QString &part : rawParts) {
        const QString stmt = stripLineComments(part).trimmed();
        if (stmt.isEmpty())
            continue;

        if (!query.exec(stmt)) {
            const QString msg = QStringLiteral("Schema initialization failed: %1 | SQL: %2")
                                    .arg(query.lastError().text(), stmt.left(120));
            if (errorString)
                *errorString = msg;
            qWarning() << "[AoiSchema]" << msg;
            return false;
        }
    }

    return true;
}

bool SchemaInitializer::applyInspectionHistorySchema(QSqlDatabase &db, QString *errorString)
{
    const QString path =
        aoiSqliteFilePath(QStringLiteral("006_aoi_inspection_history_schema.sql"));
    if (!QFile::exists(path)) {
        const QString msg = QStringLiteral("Schema file not found: %1").arg(path);
        if (errorString)
            *errorString = msg;
        qWarning() << "[AoiSchema]" << msg;
        return false;
    }

    if (!executeSqlScript(db, path, errorString))
        return false;

    qDebug() << "[AoiSchema] aoi_inspection_history schema ready";
    return true;
}

} // namespace aoi::repository
