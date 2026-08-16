/**
 * @file    Category.h
 * @brief   产品分类实体模型（MySQL 示例）
 */
#pragma once

#include <QxOrm.h>

#include <QtCore/QString>
#include <memory>
#include <vector>

// 前向声明
struct Product;

// ─────────────────────────────────────────────────────────────────────────────
// 实体定义
// ─────────────────────────────────────────────────────────────────────────────
struct Category
{
    long    id          = 0;
    QString name;
    QString description;
    int     sortOrder   = 0;

    // OneToMany：一个分类包含多个产品
    std::vector<std::shared_ptr<Product>> products;

    Category() = default;
    explicit Category(const QString &name_,
                      const QString &desc_ = {},
                      int sort_ = 0)
        : name(name_), description(desc_), sortOrder(sort_) {}
};

QX_REGISTER_HPP(Category, qx::trait::no_base_class_defined, 0)
QX_REGISTER_SERIALIZE_QDATASTREAM_HPP(, Category)

namespace qx {
template <> void register_class(QxClass<Category> &t);
}