/**
 * @file    register.cpp
 * @brief   将 Category / Product 注册到 QxOrm 上下文（MySQL 示例）
 */
#include "Category.h"
#include "Product.h"

// ─────────────────────────────────────────────────────────────────────────────
// Category
// ─────────────────────────────────────────────────────────────────────────────
QX_REGISTER_CPP(Category)
QX_REGISTER_SERIALIZE_QDATASTREAM_CPP(Category)

namespace qx {
template <> void register_class(QxClass<Category> &t)
{
    t.id(&Category::id, "id");
    t.data(&Category::name,        "name");
    t.data(&Category::description, "description");
    t.data(&Category::sortOrder,   "sort_order");

    // OneToMany：sKey="products"，外键在 product 表的 category_id 列
    t.relationOneToMany(&Category::products, "products", "category_id");
}
} // namespace qx

// ─────────────────────────────────────────────────────────────────────────────
// Product
// ─────────────────────────────────────────────────────────────────────────────
QX_REGISTER_CPP(Product)
QX_REGISTER_SERIALIZE_QDATASTREAM_CPP(Product)

namespace qx {
template <> void register_class(QxClass<Product> &t)
{
    t.id(&Product::id, "id");
    t.data(&Product::name,    "name");
    t.data(&Product::sku,     "sku");
    t.data(&Product::price,   "price");
    t.data(&Product::stock,   "stock");
    t.data(&Product::enabled, "enabled");

    // ManyToOne：sKey 使用 category_id，与数据库外键列保持一致
    t.relationManyToOne(&Product::category, "category_id");
}
} // namespace qx
