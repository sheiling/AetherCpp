/**
 * @file    DbManager.h
 * @brief   数据库连接管理器（单例封装 QxOrm QxSqlDatabase）
 *
 * 职责：
 *  1. 根据 DbConfig 初始化 QxSqlDatabase 全局连接参数
 *  2. 提供 init() / isConnected() / currentConnection() 等便捷接口
 *  3. 封装 QxOrm 的 createTable / session 等常用入口，隐藏 QxOrm 细节
 *
 * 设计原则：
 *  - 主线程调用 init() 一次，QxOrm 内部按线程维护连接池
 *  - 多数 DAO 操作直接用 qx::dao::xxx 或 Repository<T>，无需持有连接对象
 *
 * 使用示例：
 * @code
 *   // main() 入口处
 *   DbManager &db = DbManager::instance();
 *   db.init(DbConfig::sqlite("app.db", true));
 *   if (!db.isConnected()) { qCritical() << db.lastError(); }
 *
 *   // 建表（开发/测试阶段）
 *   db.createTable<User>();
 * @endcode
 */
#pragma once

#include "orm_global.h"
#include "DbConfig.h"

#include <QString>
#include <QSqlError>
#include <functional>
#include <memory>

// 前向声明，避免头文件污染
class QSqlDatabase;
namespace qx { class QxSession; }

namespace orm {

class ORM_API DbManager
{
public:
    /**
     * @brief 获取全局单例
     */
    static DbManager &instance();

    /**
     * @brief 根据配置初始化数据库连接参数
     *
     * 该函数仅设置 QxSqlDatabase 的连接参数，并做一次 test-open
     * 以验证配置是否有效。真正的连接在首次 DAO 操作时按线程建立。
     *
     * @param config  数据库配置
     * @return true  配置并试连接成功
     * @return false 试连接失败，可通过 lastError() 查看详情
     */
    bool init(const DbConfig &config);

    /**
     * @brief 当前配置是否已初始化且成功连接过
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
     * @brief 为当前线程返回一个有效的 QSqlDatabase 连接
     *
     * 底层调用 qx::QxSqlDatabase::getDatabase()，
     * 封装后屏蔽 QxOrm 内部细节。
     */
    QSqlDatabase database();

    /**
     * @brief 关闭并清理所有线程的数据库连接
     *
     * 通常在程序退出前调用。
     */
    void closeAll();

    /**
     * @brief 便捷接口：为类型 T 创建对应的数据库表
     *
     * 等价于 qx::dao::create_table<T>()，
     * 仅适合 SQLite 原型开发；正式环境请使用迁移脚本。
     *
     * @tparam T  已在 QxOrm 中注册的数据模型类型
     * @return 空 QSqlError 表示成功
     */
    template <typename T>
    QSqlError createTable();

    /**
     * @brief 便捷接口：在事务 Session 中执行一组操作
     *
     * @param fn  事务体函数，参数为 qx::QxSession&，返回 bool
     *            （返回 false 则自动 rollback）
     * @return true 事务提交成功
     */
    bool transaction(const std::function<bool(qx::QxSession &)> &fn);

private:
    DbManager();
    ~DbManager();
    DbManager(const DbManager &)            = delete;
    DbManager &operator=(const DbManager &) = delete;

    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace orm

// ─────────────────────────────────────────────────────────────────────────────
// 模板实现（须放在头文件，或在 .inl 中引用）
// ─────────────────────────────────────────────────────────────────────────────
#include <QxOrm.h>

namespace orm {

template <typename T>
QSqlError DbManager::createTable()
{
    return qx::dao::create_table<T>();
}

} // namespace orm
