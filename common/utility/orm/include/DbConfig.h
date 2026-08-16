/**
 * @file    DbConfig.h
 * @brief   数据库连接配置
 *
 * 支持的驱动（Qt SQL Driver）：
 *   - QSQLITE  — SQLite 嵌入式数据库（无需服务器）
 *   - QMYSQL   — MySQL / MariaDB
 *   - QPSQL    — PostgreSQL
 *   - QODBC    — ODBC 通用接口
 *
 * 用法示例（SQLite）：
 * @code
 *   DbConfig cfg = DbConfig::sqlite("my_app.db");
 *   DbManager::instance().init(cfg);
 * @endcode
 *
 * 用法示例（MySQL）：
 * @code
 *   DbConfig cfg;
 *   cfg.driver   = "QMYSQL";
 *   cfg.host     = "127.0.0.1";
 *   cfg.port     = 3306;
 *   cfg.database = "ess_db";
 *   cfg.username = "root";
 *   cfg.password = "secret";
 *   DbManager::instance().init(cfg);
 * @endcode
 */
#pragma once

#include "orm_global.h"

#include <QString>

namespace orm {

/**
 * @brief 数据库驱动类型枚举（与 Qt SQL Driver 名称对应）
 */
enum class DbDriver {
    SQLite,    ///< QSQLITE
    MySQL,     ///< QMYSQL
    PostgreSQL,///< QPSQL
    ODBC,      ///< QODBC
    Custom     ///< 自定义（由 driverName 字段指定）
};

/**
 * @brief 数据库连接配置
 */
struct ORM_API DbConfig
{
    // ── 通用字段 ──────────────────────────────────────────────
    DbDriver driver   = DbDriver::SQLite; ///< 驱动类型（枚举快捷方式）
    QString  driverName;  ///< 驱动名（Custom时生效，或覆盖 driver 推导值）
    QString  database;    ///< 数据库名（SQLite为文件路径）
    QString  host     = "127.0.0.1"; ///< 主机名（仅服务器型数据库）
    int      port     = 0;           ///< 端口（0=使用驱动默认端口）
    QString  username;    ///< 用户名
    QString  password;    ///< 密码
    QString  connectOptions; ///< 驱动特定的连接选项字符串

    // ── 调试 / 性能 ──────────────────────────────────────────
    bool traceSql      = false; ///< 是否打印 SQL 语句（开发阶段建议 true）
    bool traceSqlError = true;  ///< 出错时打印 SQL 绑定参数

    // ── 工厂方法 ─────────────────────────────────────────────

    /**
     * @brief 快速创建 SQLite 配置
     * @param filePath  数据库文件路径（":memory:" 为内存模式）
     * @param trace     是否开启 SQL 追踪
     */
    static DbConfig sqlite(const QString &filePath, bool trace = false);

    /**
     * @brief 快速创建 MySQL 配置
     */
    static DbConfig mysql(const QString &database,
                          const QString &host     = "127.0.0.1",
                          int            port     = 3306,
                          const QString &username = "root",
                          const QString &password = "",
                          bool           trace    = false);

    /**
     * @brief 根据 driver 枚举推导 Qt 驱动名字符串
     */
    QString resolvedDriverName() const;

    /**
     * @brief 简要描述（用于日志输出）
     */
    QString toString() const;
};

} // namespace orm
