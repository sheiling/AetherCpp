/**
 * @file    Product.h
 * @brief   产品实体模型（MySQL 示例）
 */
#pragma once

#include <QxOrm.h>

#include <QtCore/QString>
#include <memory>
#include <vector>

// 前向声明
struct Category;

// ─────────────────────────────────────────────────────────────────────────────
// 实体定义
// ─────────────────────────────────────────────────────────────────────────────
struct Product
{
    long    id       = 0;
    QString name;
    QString sku;        // 库存编号
    double  price    = 0.0;
    int     stock    = 0;
    bool    enabled  = true;

    // ManyToOne：产品属于某个分类
    std::shared_ptr<Category> category;

    Product() = default;
    Product(const QString &name_, const QString &sku_,
            double price_, int stock_)
        : name(name_), sku(sku_), price(price_), stock(stock_) {}
};

QX_REGISTER_HPP(Product, qx::trait::no_base_class_defined, 0)
QX_REGISTER_SERIALIZE_QDATASTREAM_HPP(, Product)

namespace qx {
template <> void register_class(QxClass<Product> &t);
}