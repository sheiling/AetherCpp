/**
 * @file    Repository.h
 * @brief   泛型 Repository 模板 —— 对 QxOrm DAO 层的统一封装
 *
 * Repository 模式（参考 Spring Data / Eloquent / GORM 的设计思路）：
 *   - 将数据访问逻辑集中在 Repository 中，业务层不直接调用 qx::dao::
 *   - 提供标准 CRUD + 查询接口，支持关联关系预加载
 *   - 所有操作均返回 QSqlError，调用方自行判断是否有误
 *
 * 用法：
 * @code
 *   // 1. 定义模型（见 OrmBase.h 注册宏）
 *   struct User { long id; QString name; QString email; };
 *
 *   // 2. 直接使用 Repository
 *   Repository<User> repo;
 *
 *   User u; u.name = "Alice"; u.email = "alice@example.com";
 *   QSqlError err = repo.insert(u);           // INSERT
 *
 *   QList<std::shared_ptr<User>> all;
 *   err = repo.fetchAll(all);                 // SELECT *
 *
 *   QxSqlQuery q; q.where("name").isEqualTo("Alice");
 *   err = repo.fetchByQuery(q, all);          // SELECT WHERE name='Alice'
 * @endcode
 *
 * @note  T 必须已通过 QX_REGISTER_HPP / QX_REGISTER_CPP 注册到 QxOrm 上下文。
 */
#pragma once

#include "orm_global.h"

#include <QxOrm.h>

#include <QtSql/QSqlError>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include <memory>

namespace orm {

/**
 * @brief  泛型 Repository，封装 QxOrm 全套 DAO 操作
 * @tparam T  QxOrm 已注册的数据模型类
 * @tparam ID 主键类型，默认 long
 */
template <typename T, typename ID = long>
class Repository
{
public:
    using EntityPtr     = std::shared_ptr<T>;
    using EntityList    = QList<EntityPtr>;
    using EntityPtrList = qx::QxCollection<ID, EntityPtr>;

    // ── CRUD ─────────────────────────────────────────────────────────────────

    /**
     * @brief 插入一条记录（主键自动回填）
     */
    QSqlError insert(T &entity, QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::insert(entity, db);
        if (err.isValid())
            qWarning() << "[Repository] insert error:" << err.text();
        return err;
    }

    /**
     * @brief 更新一条记录
     * @param columns 指定只更新哪些列，空表示全部
     */
    QSqlError update(T &entity,
                     const QStringList &columns = {},
                     QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::update(entity, db, columns);
        if (err.isValid())
            qWarning() << "[Repository] update error:" << err.text();
        return err;
    }

    /**
     * @brief 插入或更新（根据主键是否存在自动选择）
     */
    QSqlError save(T &entity, QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::save(entity, db);
        if (err.isValid())
            qWarning() << "[Repository] save error:" << err.text();
        return err;
    }

    /**
     * @brief 按主键删除（支持软删除）
     */
    QSqlError removeById(T &entity, QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::delete_by_id(entity, db);
        if (err.isValid())
            qWarning() << "[Repository] removeById error:" << err.text();
        return err;
    }

    /**
     * @brief 按条件批量删除
     */
    QSqlError removeByQuery(const qx::QxSqlQuery &query,
                            QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::delete_by_query<T>(query, db);
        if (err.isValid())
            qWarning() << "[Repository] removeByQuery error:" << err.text();
        return err;
    }

    /**
     * @brief 删除全部（慎用）
     */
    QSqlError removeAll(QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::delete_all<T>(db);
        if (err.isValid())
            qWarning() << "[Repository] removeAll error:" << err.text();
        return err;
    }

    // ── 查询 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 按主键查询单条（entity 须先设置主键）
     * @param columns 只获取指定列，空表示全部
     */
    QSqlError findById(T &entity,
                       const QStringList &columns = {},
                       QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_by_id(entity, db, columns);
        if (err.isValid())
            qWarning() << "[Repository] findById error:" << err.text();
        return err;
    }

    /**
     * @brief 按主键查询，同时预加载指定关联
     * @param relations 关联键列表，传 {"*"} 加载全部关联
     */
    QSqlError findByIdWithRelation(T &entity,
                                   const QStringList &relations,
                                   QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_by_id_with_relation(relations, entity, db);
        if (err.isValid())
            qWarning() << "[Repository] findByIdWithRelation error:" << err.text();
        return err;
    }

    /**
     * @brief 查询全部记录到 QxCollection（id → entity）
     */
    QSqlError fetchAll(EntityPtrList &result,
                       const QStringList &columns = {},
                       QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_all(result, db, columns);
        if (err.isValid())
            qWarning() << "[Repository] fetchAll error:" << err.text();
        return err;
    }

    /**
     * @brief 按条件查询，结果写入 QxCollection
     */
    QSqlError fetchByQuery(const qx::QxSqlQuery &query,
                           EntityPtrList &result,
                           const QStringList &columns = {},
                           QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_by_query(query, result, db, columns);
        if (err.isValid())
            qWarning() << "[Repository] fetchByQuery error:" << err.text();
        return err;
    }

    /**
     * @brief 查询全部，同时预加载指定关联
     */
    QSqlError fetchAllWithRelation(const QStringList &relations,
                                   EntityPtrList &result,
                                   QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_all_with_relation(relations, result, db);
        if (err.isValid())
            qWarning() << "[Repository] fetchAllWithRelation error:" << err.text();
        return err;
    }

    /**
     * @brief 按条件查询，同时预加载指定关联
     */
    QSqlError fetchByQueryWithRelation(const QStringList &relations,
                                       const qx::QxSqlQuery &query,
                                       EntityPtrList &result,
                                       QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::fetch_by_query_with_relation(
                            relations, query, result, db);
        if (err.isValid())
            qWarning() << "[Repository] fetchByQueryWithRelation error:" << err.text();
        return err;
    }

    // ── 统计 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 统计行数
     */
    long count(const qx::QxSqlQuery &query = {},
               QSqlDatabase *db = nullptr) const
    {
        return qx::dao::count<T>(query, db);
    }

    /**
     * @brief 判断记录是否存在（entity 须先设置主键）
     */
    bool exists(T &entity, QSqlDatabase *db = nullptr) const
    {
        return qx::dao::exist(entity, db);
    }

    // ── 表结构 ────────────────────────────────────────────────────────────────

    /**
     * @brief 创建对应数据库表（仅 SQLite 原型用）
     */
    QSqlError createTable(QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::create_table<T>(db);
        if (err.isValid())
            qWarning() << "[Repository] createTable error:" << err.text();
        return err;
    }

    // ── 批量操作 ──────────────────────────────────────────────────────────────

    /**
     * @brief 批量插入列表（内部开启事务）
     * @param list  std::vector / QList 等容器均可
     */
    template <typename Container>
    QSqlError batchInsert(Container &list, QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::insert(list, db);
        if (err.isValid())
            qWarning() << "[Repository] batchInsert error:" << err.text();
        return err;
    }

    /**
     * @brief 批量保存（insert-or-update）
     */
    template <typename Container>
    QSqlError batchSave(Container &list, QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::save(list, db);
        if (err.isValid())
            qWarning() << "[Repository] batchSave error:" << err.text();
        return err;
    }

    // ── 原生 SQL ──────────────────────────────────────────────────────────────

    /**
     * @brief 执行自定义 SQL，将可映射的列自动填充到 result
     *
     * @code
     *   QxCollection<long, std::shared_ptr<User>> result;
     *   qx::QxSqlQuery q("SELECT * FROM user WHERE age > :age");
     *   q.bind(":age", 18);
     *   repo.executeQuery(q, result);
     * @endcode
     */
    QSqlError executeQuery(qx::QxSqlQuery &query,
                           EntityPtrList &result,
                           QSqlDatabase *db = nullptr) const
    {
        QSqlError err = qx::dao::execute_query(query, result, db);
        if (err.isValid())
            qWarning() << "[Repository] executeQuery error:" << err.text();
        return err;
    }
};

} // namespace orm
