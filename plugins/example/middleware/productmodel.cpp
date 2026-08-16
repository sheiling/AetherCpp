#include "productmodel.h"

// 静态成员初始化（模拟数据库）
QList<Product> ProductRepository::m_products = {
    { "MacBook Pro",  12999.0, 50,  "电脑",    "苹果笔记本电脑，M3 芯片" },
    { "iPhone 15",     6999.0, 120, "手机",    "苹果旗舰智能手机" },
    { "机械键盘",        399.0, 200, "外设",    "Cherry MX 红轴，RGB 背光" },
};

QList<Product> ProductRepository::getAllProducts()
{
    return m_products;
}

bool ProductRepository::addProduct(const Product &product)
{
    if (product.name.isEmpty() || product.price < 0 || product.stock < 0)
        return false;

    m_products.append(product);
    return true;
}

void ProductRepository::clearAll()
{
    m_products.clear();
}
