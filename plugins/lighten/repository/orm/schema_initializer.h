#pragma once

/**
 * @file schema_initializer.h
 * @brief 检测历史表 schema 幂等初始化
 */

#include "repository/storageconfig.h"

#include <QString>

namespace lighten {

/**
 * @brief 执行 config/sql 下检测历史与生产 KPI schema（004 + 005）
 */
class SchemaInitializer
{
public:
    /** 按 backend 选择 sqlite/mysql 脚本并依次幂等建表 */
    static bool apply(StorageBackend backend, QString *errorString = nullptr);

private:
    static bool executeSqlScript(const QString &filePath, QString *errorString);
};

} // namespace lighten
