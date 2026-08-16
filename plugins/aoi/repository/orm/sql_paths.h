#pragma once

/**
 * @file sql_paths.h
 * @brief config/sql 目录定位（开发态工程根 / 发布态 exe 旁）
 */

#include <QString>

namespace aoi::repository {

/** 返回 config/sql 根目录绝对路径 */
QString aoiSqlRootPath();

/** config/sql/sqlite/{fileName} 绝对路径 */
QString aoiSqliteFilePath(const QString &fileName);

} // namespace aoi::repository
