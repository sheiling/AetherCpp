#ifndef PRODUCTVIEWMODEL_H
#define PRODUCTVIEWMODEL_H

#include "viewmodel.h"
#include "command.h"
#include "productmodel.h"
#include "loggingmiddleware.h"
#include "auditlogmiddleware.h"
#include "authcommandmiddleware.h"
#include <memory>

/**
 * @brief 产品管理 ViewModel（Middleware around 模式示例）
 *
 * 完整演示"洋葱"（around）执行路径：
 *
 * ─────── 属性中间件管道 ───────────────────────────────────────────────────
 *
 *   [属性日志] → 进入: price  [0] → [999]
 *     [审计日志] → ...
 *       ★ 属性写入: price = 999          ← 嵌在链条最深处
 *     [审计日志] ← 记录审计条目
 *   [属性日志] ← 完成: price = [999] ✓
 *
 *   注册顺序：LoggingMiddleware → AuditLogMiddleware → [写入属性]
 *
 * ─────── 命令中间件管道（addCommand）────────────────────────────────────
 *
 *   [计时] → 开始: addProduct
 *     [AUTH] → 验证: addProduct
 *       [命令日志] → 进入: addProduct
 *         ★ execute(): addProduct        ← 嵌在链条最深处
 *       [命令日志] ← 完成: addProduct ✓
 *     [AUTH] ← 通过: addProduct ✓
 *   [计时] ← 结束: addProduct  耗时 N ms ✓
 *
 *   注册顺序：TimingCommandMiddleware → AuthCommandMiddleware
 *             → LoggingCommandMiddleware → [execute()]
 *
 * ViewModel 暴露 middleware 访问器，供 View 设置回调 / 调整运行时配置。
 */
class ProductViewModel : public ViewModel
{
    Q_OBJECT

public:
    // ── 属性名常量 ──────────────────────────────────────────────────────────
    static constexpr const char *PROP_NAME        = "name";
    static constexpr const char *PROP_PRICE       = "price";
    static constexpr const char *PROP_STOCK       = "stock";
    static constexpr const char *PROP_CATEGORY    = "category";
    static constexpr const char *PROP_DESCRIPTION = "description";
    static constexpr const char *PROP_STATUS      = "status";
    static constexpr const char *PROP_COUNT       = "count";

    // ── 属性宏（自动生成 getter / setter）──────────────────────────────────
    VIEWMODEL_PROPERTY(QString, name,        Name)
    VIEWMODEL_PROPERTY(double,  price,       Price)
    VIEWMODEL_PROPERTY(int,     stock,       Stock)
    VIEWMODEL_PROPERTY(QString, category,    Category)
    VIEWMODEL_PROPERTY(QString, description, Description)
    VIEWMODEL_PROPERTY(QString, status,      Status)
    VIEWMODEL_PROPERTY(int,     count,       Count)

signals:
    void nameChanged();
    void priceChanged();
    void stockChanged();
    void categoryChanged();
    void descriptionChanged();
    void statusChanged();
    void countChanged();

public:
    explicit ProductViewModel(QObject *parent = nullptr);
    ~ProductViewModel() override = default;

    // ── Command 访问器 ──────────────────────────────────────────────────────
    Command *addCommand()   const { return m_addCommand;   }
    Command *clearCommand() const { return m_clearCommand; }
    Command *loadCommand()  const { return m_loadCommand;  }

    // ── Middleware 访问器（供 View 设置回调 / 调整运行时配置）──────────────
    std::shared_ptr<LoggingMiddleware>         loggingMiddleware()  const { return m_logMW;     }
    std::shared_ptr<AuditLogMiddleware>        auditMiddleware()    const { return m_auditMW;   }
    std::shared_ptr<AuthCommandMiddleware>     authMiddleware()     const { return m_authMW;    }
    std::shared_ptr<LoggingCommandMiddleware>  cmdLogMiddleware()   const { return m_cmdLogMW;  }
    std::shared_ptr<TimingCommandMiddleware>   timingMiddleware()   const { return m_timingMW;  }

public slots:
    void addProduct();
    void clearForm();
    void loadProducts();

private:
    bool canAdd() const;
    void updateStatus(const QString &msg);

private:
    FunctionalCommand *m_addCommand   = nullptr;
    FunctionalCommand *m_clearCommand = nullptr;
    FunctionalCommand *m_loadCommand  = nullptr;

    // 持有 middleware 实例（shared_ptr，与管道共享所有权）
    std::shared_ptr<LoggingMiddleware>         m_logMW;
    std::shared_ptr<AuditLogMiddleware>        m_auditMW;
    std::shared_ptr<AuthCommandMiddleware>     m_authMW;
    std::shared_ptr<LoggingCommandMiddleware>  m_cmdLogMW;
    std::shared_ptr<TimingCommandMiddleware>   m_timingMW;
};

#endif // PRODUCTVIEWMODEL_H
