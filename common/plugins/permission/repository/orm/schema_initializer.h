#pragma once

#include "data/domain/domain_types.h"

#include <QString>

namespace permission {

/**
 * @brief 执行 config/sql/{sqlite|mysql}/001_schema.sql 建表脚本
 */
class SchemaInitializer
{
public:
    static bool apply(StorageBackend backend, QString *errorString = nullptr);

    /// 执行单个 SQL 脚本（供 DataSeeder 等复用）
    static bool executeScript(const QString &filePath, QString *errorString = nullptr);

private:
    static QString schemaFilePath(StorageBackend backend, int version);
    static bool executeSqlScript(const QString &filePath, QString *errorString);
};

} // namespace permission
