#pragma once

/**
 * @file storageconfig.h
 * @brief 检测历史数据存储配置（Mock / SQLite / MySQL）
 */

#if COMMON_ORM_ENABLED
#include <DbConfig.h>
#endif

#include <QString>

namespace lighten {

/** 检测历史数据存储后端类型 */
enum class StorageBackend {
    Mock   = 0, ///< 内存 Mock，无持久化数据
    Sqlite = 1, ///< 本地 SQLite 文件
    MySql  = 2, ///< MySQL / MariaDB
};

/** 将存储后端枚举转为可读字符串（日志用） */
inline QString storageBackendName(StorageBackend b)
{
    switch (b) {
    case StorageBackend::Mock:   return QStringLiteral("mock");
    case StorageBackend::Sqlite: return QStringLiteral("sqlite");
    case StorageBackend::MySql:  return QStringLiteral("mysql");
    }
    return QStringLiteral("unknown");
}

/**
 * @brief 检测历史 DataStore 配置
 *
 * 由 LightenPlugin 启动时注入，决定 RepositoryFactory 创建何种实现。
 */
struct StorageConfig
{
    StorageBackend backend = StorageBackend::Mock;

    QString sqliteFilePath = QStringLiteral("data/lighten_inspection.db"); ///< SQLite 库文件路径
    bool    createSchemaOnOpen = true;  ///< open() 时执行 004/005 schema（历史宽表 + 生产 KPI）
    bool    autoCreateDatabase = false; ///< MySQL 库不存在时自动 CREATE DATABASE
#if COMMON_ORM_ENABLED
    orm::DbConfig dbConfig;             ///< ORM 连接参数（Sqlite/MySql 生效）
#endif

    /** 构造 Mock 配置 */
    static StorageConfig mock()
    {
        StorageConfig c;
        c.backend = StorageBackend::Mock;
        return c;
    }

    /** 构造 SQLite 配置 */
    static StorageConfig sqlite(const QString &filePath, bool traceSql = false)
    {
        StorageConfig c;
        c.backend            = StorageBackend::Sqlite;
        c.sqliteFilePath     = filePath;
#if COMMON_ORM_ENABLED
        c.dbConfig           = orm::DbConfig::sqlite(filePath, traceSql);
#endif
        c.createSchemaOnOpen = true;
        return c;
    }

    /** 构造 MySQL 配置 */
    static StorageConfig mysql(const QString &database,
                               const QString &host = QStringLiteral("127.0.0.1"),
                               int port = 3306,
                               const QString &username = QStringLiteral("root"),
                               const QString &password = QString(),
                               bool traceSql = false)
    {
        StorageConfig c;
        c.backend              = StorageBackend::MySql;
#if COMMON_ORM_ENABLED
        c.dbConfig             = orm::DbConfig::mysql(database, host, port, username, password, traceSql);
#endif
        c.autoCreateDatabase   = true;
        c.createSchemaOnOpen   = true;
        return c;
    }
};

} // namespace lighten
