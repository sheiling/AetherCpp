#pragma once

/**
 * @file schema_initializer.h
 * @brief aoi_inspection_history schema 幂等初始化（config/sql/006）
 */

#include <QString>

class QSqlDatabase;

namespace aoi::repository {

/**
 * @brief 执行 config/sql/sqlite/006_aoi_inspection_history_schema.sql
 */
class SchemaInitializer
{
public:
    static bool applyInspectionHistorySchema(QSqlDatabase &db, QString *errorString = nullptr);

private:
    static bool executeSqlScript(QSqlDatabase &db, const QString &filePath, QString *errorString);
};

} // namespace aoi::repository
