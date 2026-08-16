#pragma once

/**
 * @file db_connection.h
 * @brief 检测历史独立 QSqlDatabase 连接管理（与 permission DbManager 隔离）
 */

#include "repository/storageconfig.h"

#include <QSqlDatabase>
#include <QString>

namespace lighten {

/**
 * @brief 点亮检测历史专用数据库连接
 *
 * 使用固定 connectionName，避免与权限插件共用 orm::DbManager 单例。
 */
class DbConnection
{
public:
    /** 按 StorageConfig 打开连接 */
    static bool open(const StorageConfig &config, QString *errorString = nullptr);
    /** 关闭并移除命名连接 */
    static void close();
    /** 连接是否已打开 */
    static bool isOpen();
    /** 当前线程可用的 QSqlDatabase */
    static QSqlDatabase database();

private:
    static QString connectionName();
};

} // namespace lighten
