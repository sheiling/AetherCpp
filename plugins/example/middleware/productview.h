#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H

#include "pagewidget.h"
#include "productviewmodel.h"
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QSplitter>

/**
 * @brief 产品管理视图（View 层）— Middleware 使用示例
 *
 * 布局（水平分割）：
 *   左侧：产品录入表单 + 操作按钮 + 产品计数
 *   右侧：中间件活动日志 + 审计变更历史
 *
 * 通过此页面可直观看到：
 *   1. LoggingMiddleware  — 每次属性变更的前置/后置日志
 *   2. AuditLogMiddleware — 成功变更后追加审计历史
 *   3. AuthCommandMiddleware — 勾选"禁用权限"后"添加产品"按钮被阻止
 */
class ProductView : public PageWidget
{
    Q_OBJECT

public:
    explicit ProductView(QWidget *parent = nullptr);
    ~ProductView() override = default;

protected:
    void onLoad()        override;
    void retranslateUi() override;
    void applyTheme()    override;

private:
    void setupUi();
    void setupBindings();

    // ── ViewModel ─────────────────────────────────────────────────────────
    ProductViewModel *m_viewModel = nullptr;

    // ── 左侧：产品表单 ────────────────────────────────────────────────────
    QGroupBox   *m_formGroup       = nullptr;
    QFormLayout *m_formLayout      = nullptr;
    QLineEdit   *m_nameEdit        = nullptr;
    QDoubleSpinBox *m_priceSpinBox = nullptr;
    QSpinBox    *m_stockSpinBox    = nullptr;
    QComboBox   *m_categoryCombo   = nullptr;
    QLineEdit   *m_descEdit        = nullptr;

    QPushButton *m_addButton       = nullptr;
    QPushButton *m_clearButton     = nullptr;
    QPushButton *m_loadButton      = nullptr;

    QLabel      *m_statusLabel     = nullptr;
    QLabel      *m_countLabel      = nullptr;

    // ── 权限模拟开关 ───────────────────────────────────────────────────────
    QCheckBox   *m_denyCheckBox    = nullptr;

    // ── 右侧：中间件日志面板 ───────────────────────────────────────────────
    QGroupBox   *m_logGroup        = nullptr;
    QTextEdit   *m_logEdit         = nullptr;   ///< 实时日志
    QPushButton *m_clearLogButton  = nullptr;

    QGroupBox   *m_auditGroup      = nullptr;
    QTextEdit   *m_auditEdit       = nullptr;   ///< 审计历史
    QPushButton *m_clearAuditButton = nullptr;

    // ── 标题 ───────────────────────────────────────────────────────────────
    QLabel      *m_titleLabel      = nullptr;
};

#endif // PRODUCTVIEW_H
