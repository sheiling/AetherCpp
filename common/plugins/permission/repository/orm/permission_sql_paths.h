#pragma once

#include "data/domain/domain_types.h"

#include <QString>

namespace permission {

/// 权限 SQL 根目录：Debug 优先项目根 config/sql；Release 优先 {exe}/config/sql
QString permissionSqlRootPath();

/// config/sql/{sqlite|mysql}/001_schema.sql 等
QString permissionSqlFilePath(StorageBackend backend, const QString &fileName);

} // namespace permission
