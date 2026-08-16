#pragma once

/**
 * @file sql_paths.h
 * @brief config/sql 目录定位（开发态工程根 / 发布态 exe 旁）
 */

#include "repository/storageconfig.h"

#include <QString>

namespace lighten {

/** 返回 config/sql 根目录绝对路径 */
QString inspectionSqlRootPath();
/** 按 backend 拼接具体 SQL 文件路径 */
QString inspectionSqlFilePath(StorageBackend backend, const QString &fileName);

} // namespace lighten
