#include "productviewmodel.h"
#include <QDebug>

ProductViewModel::ProductViewModel(QObject *parent)
    : ViewModel(parent)
{
    // ── 注册属性 ─────────────────────────────────────────────────────────────
    registerProperty(PROP_NAME,        QString());
    registerProperty(PROP_PRICE,       0.0);
    registerProperty(PROP_STOCK,       0);
    registerProperty(PROP_CATEGORY,    QString());
    registerProperty(PROP_DESCRIPTION, QString());
    registerProperty(PROP_STATUS,      QString::fromUtf8("\xe5\xb0\xb1\xe7\xbb\xaa"));  // 就绪
    registerProperty(PROP_COUNT,       0);

    // ── 创建属性中间件（around 模式，按注册顺序由外到内包裹）────────────────
    //
    // 执行路径：
    //   [属性日志] → 进入
    //     [审计日志] → ...
    //       ★ 属性写入（最深处）
    //     [审计日志] ← 记录
    //   [属性日志] ← 完成
    //
    m_logMW   = std::make_shared<LoggingMiddleware>();
    m_auditMW = std::make_shared<AuditLogMiddleware>();

    addPropertyMiddleware(m_logMW);    // 第 1 层（最外层）：前置/后置日志
    addPropertyMiddleware(m_auditMW);  // 第 2 层（内层）：审计记录

    // ── 创建命令 ─────────────────────────────────────────────────────────────
    m_addCommand = new FunctionalCommand(
        [this]() { addProduct(); },
        [this]() { return canAdd(); },
        this
    );
    m_addCommand->setName("addProduct");
    m_addCommand->setViewModel(this);

    m_clearCommand = new FunctionalCommand(
        [this]() { clearForm(); },
        nullptr,
        this
    );
    m_clearCommand->setName("clearForm");
    m_clearCommand->setViewModel(this);

    m_loadCommand = new FunctionalCommand(
        [this]() { loadProducts(); },
        nullptr,
        this
    );
    m_loadCommand->setName("loadProducts");
    m_loadCommand->setViewModel(this);

    // ── 创建命令中间件（around 模式，按注册顺序由外到内包裹）────────────────
    //
    // 完整执行路径（addCommand）：
    //
    //   [计时] → 开始: addProduct
    //     [AUTH] → 验证: addProduct
    //       [命令日志] → 进入: addProduct
    //         ★ execute(): addProduct（最深处）
    //       [命令日志] ← 完成: addProduct ✓
    //     [AUTH] ← 通过: addProduct ✓
    //   [计时] ← 结束: addProduct  耗时 N ms ✓
    //
    m_timingMW = std::make_shared<TimingCommandMiddleware>();
    m_authMW   = std::make_shared<AuthCommandMiddleware>();
    m_cmdLogMW = std::make_shared<LoggingCommandMiddleware>();

    // addCommand：完整三层管道
    m_addCommand->addMiddleware(m_timingMW);  // 第 1 层（最外层）：计时
    m_addCommand->addMiddleware(m_authMW);    // 第 2 层：权限校验（可取消）
    m_addCommand->addMiddleware(m_cmdLogMW);  // 第 3 层（最内层）：命令日志

    // clearCommand / loadCommand：计时 + 日志（无权限限制）
    m_clearCommand->addMiddleware(m_timingMW);
    m_clearCommand->addMiddleware(m_cmdLogMW);

    m_loadCommand->addMiddleware(m_timingMW);
    m_loadCommand->addMiddleware(m_cmdLogMW);

    // ── 监听属性变更，动态更新 addCommand 可用性 ─────────────────────────────
    connect(this, &ViewModel::propertyChanged, this, [this]() {
        m_addCommand->updateCanExecute();
    });

    // ── 初始加载 ─────────────────────────────────────────────────────────────
    loadProducts();
}

void ProductViewModel::addProduct()
{
    Product p;
    p.name        = name();
    p.price       = price();
    p.stock       = stock();
    p.category    = category();
    p.description = description();

    if (ProductRepository::addProduct(p)) {
        updateStatus(QString::fromUtf8("\xe4\xba\xa7\xe5\x93\x81 '%1' \xe6\xb7\xbb\xe5\x8a\xa0\xe6\x88\x90\xe5\x8a\x9f\xef\xbc\x81").arg(p.name));
        clearForm();
        loadProducts();
    } else {
        updateStatus(QString::fromUtf8("\xe6\xb7\xbb\xe5\x8a\xa0\xe5\xa4\xb1\xe8\xb4\xa5\xef\xbc\x9a\xe8\xaf\xb7\xe6\xa3\x80\xe6\x9f\xa5\xe4\xba\xa7\xe5\x93\x81\xe4\xbf\xa1\xe6\x81\xaf\xe6\x98\xaf\xe5\x90\xa6\xe5\xae\x8c\xe6\x95\xb4\xe3\x80\x82"));
    }
}

void ProductViewModel::clearForm()
{
    beginBatchUpdate();
    setName(QString());
    setPrice(0.0);
    setStock(0);
    setCategory(QString());
    setDescription(QString());
    endBatchUpdate();
    updateStatus(QString::fromUtf8("\xe8\xa1\xa8\xe5\x8d\x95\xe5\xb7\xb2\xe6\xb8\x85\xe7\xa9\xba\xe3\x80\x82"));
}

void ProductViewModel::loadProducts()
{
    const QList<Product> products = ProductRepository::getAllProducts();
    setCount(products.size());
    updateStatus(QString::fromUtf8("\xe5\xb7\xb2\xe5\x8a\xa0\xe8\xbd\xbd %1 \xe4\xbb\xb6\xe4\xba\xa7\xe5\x93\x81\xe3\x80\x82").arg(products.size()));
}

bool ProductViewModel::canAdd() const
{
    return !name().isEmpty() && price() >= 0.0 && stock() >= 0;
}

void ProductViewModel::updateStatus(const QString &msg)
{
    setStatus(msg);
}
