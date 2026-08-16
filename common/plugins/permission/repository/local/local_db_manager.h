/**
 * @file    local_db_manager.h
 * @brief   数据库连接管理器（单例封装 QSqlDatabase，无 QxOrm 依赖）
 *
 * 职责：
 *  1. 根据 DbConfig 初始化 QSqlDatabase 全局连接
 *  2. 提供 init() / isConnected() / database() 等便捷接口
 *  3. 隐藏 QSqlDatabase 连接名称细节
 *
 * 设计原则：
 *  - 主线程调用 init() 一次，使用固定连接名 "permission_connection"
 *  - 轻量设计，仅提供 Permission 插件所需的最小 API 子集
 *
 * 使用示例：
 * @code
 *   // main() 入口处
 *   DbManager &db = DbManager::instance();
 *   db.init(DbConfig::sqlite("app.db", true));
 *   if (!db.isConnected()) { qCritical() << db.lastError(); }
 *
 *   // 获取数据库对象
 *   QSqlDatabase d = db.database();
 * @endcode
 */
#pragma once

#include "local_db_config.h"

#include <QString>
#include <memory>

// 前向声明，避免头文件污染
class QSqlDatabase;

namespace orm {

class DbManager
{
public:
    /**
     * @brief 获取全局单例
     */
    static DbManager &instance();

    /**
     * @brief 根据配置初始化数据库连接
     *
     * 使用 QSqlDatabase::addDatabase() 注册一个固定名为
     * "permission_connection" 的连接，并做一次 open() 验证。
     *
     * @param config  数据库配置
     * @return true  初始化并连接成功
     * @return false 连接失败，可通过 lastError() 查看详情
     */
    bool init(const DbConfig &config);

    /**
     * @brief 当前连接是否已初始化且处于打开状态
     */
    bool isConnected() const;

    /**
     * @brief 最近一次操作的错误信息
     */
    QString lastError() const;

    /**
     * @brief 返回当前使用的配置副本
     */
    DbConfig config() const;

    /**
     * @brief 返回 "permission_connection" 名称的 QSqlDatabase 对象
     */
    QSqlDatabase database();

    /**
     * @brief 关闭并清理数据库连接
     *
     * 通常在程序退出前调用。
     */
    void closeAll();

private:
    DbManager();
    ~DbManager();
    DbManager(const DbManager &)            = delete;
    DbManager &operator=(const DbManager &) = delete;

    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace orm
