#pragma once

#include "data/domain/domain_types.h"

#include <QString>

namespace permission {

/**
 * @brief 种子数据（002 用户角色、003 权限资源，见 config/sql）
 *
 * 按关键标记判断是否已灌种；已完整则跳过，有残留脏数据则报错，空库则在事务中执行脚本。
 */
class DataSeeder
{
public:
    static bool applyUsersAndRolesIfEmpty(StorageBackend backend, QString *errorString = nullptr);
    static bool applyPermissionsIfEmpty(StorageBackend backend, QString *errorString = nullptr);

    static qint64 tableRowCount(const QString &tableName, QString *errorString = nullptr);

private:
    static QString seedFilePath(StorageBackend backend, const QString &fileName);
};

} // namespace permission
