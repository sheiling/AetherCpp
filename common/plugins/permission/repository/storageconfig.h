#pragma once

#include "data/domain/domain_types.h"

#include "repository/local/local_db_config.h"

#include <QString>

namespace permission {

/**
 * @brief 权限数据存储配置（由应用启动时注入，决定使用 Mock / SQLite / MySQL）
 */
struct StorageConfig
{
    StorageBackend backend = StorageBackend::Mock;

    /// SQLite：数据库文件路径（相对路径基于应用目录）
    QString sqliteFilePath = QStringLiteral("data/permission.db");

    /// 为 true 时，在 open() 阶段自动执行 data/sql/{sqlite|mysql}/001_schema.sql 建表脚本
    bool createSchemaOnOpen = true;

    /// 为 true 时，perm_role 为空则执行 002_seed_users.sql（admin/minimal）
    bool seedUsersOnEmpty = true;

    /// 为 true 时，perm_permission 为空则执行 003_seed_permissions.sql
    bool seedPermissionsOnEmpty = true;

    /// MySQL：库不存在时自动 CREATE DATABASE（仅 MySql 生效）
    bool autoCreateDatabase = false;

    /// MySQL / 通用：ORM 连接参数（backend=MySql 时生效）
    orm::DbConfig dbConfig;

    static StorageConfig mock()
    {
        StorageConfig c;
        c.backend = StorageBackend::Mock;
        return c;
    }

    static StorageConfig sqlite(const QString &filePath,
                              bool traceSql = false)
    {
        StorageConfig c;
        c.backend             = StorageBackend::Sqlite;
        c.sqliteFilePath      = filePath;
        c.dbConfig            = orm::DbConfig::sqlite(filePath, traceSql);
        c.createSchemaOnOpen  = true;
        c.seedUsersOnEmpty       = true;
        c.seedPermissionsOnEmpty = true;
        return c;
    }

    static StorageConfig mysql(const QString &database,
                               const QString &host = QStringLiteral("127.0.0.1"),
                               int port = 3306,
                               const QString &username = QStringLiteral("root"),
                               const QString &password = QString(),
                               bool traceSql = false)
    {
        StorageConfig c;
        c.backend              = StorageBackend::MySql;
        c.dbConfig             = orm::DbConfig::mysql(database, host, port, username, password, traceSql);
        c.autoCreateDatabase   = true;
        c.createSchemaOnOpen   = true;
        c.seedUsersOnEmpty     = true;
        c.seedPermissionsOnEmpty = true;
        return c;
    }
};

} // namespace permission
