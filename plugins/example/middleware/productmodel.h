#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QString>
#include <QList>

/**
 * @brief 产品数据模型（Model 层）
 *
 * 纯数据类，不包含任何 UI 逻辑。
 */
struct Product
{
    QString name;
    double  price   = 0.0;
    int     stock   = 0;
    QString category;
    QString description;

    Product() = default;

    Product(const QString &n, double p, int s,
            const QString &cat, const QString &desc = QString())
        : name(n), price(p), stock(s), category(cat), description(desc)
    {}
};

/**
 * @brief 产品仓储（内存模拟数据访问层）
 */
class ProductRepository
{
public:
    static QList<Product> getAllProducts();
    static bool           addProduct(const Product &product);
    static void           clearAll();

private:
    static QList<Product> m_products;
};

#endif // PRODUCTMODEL_H
