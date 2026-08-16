#pragma once

/**
 * @file kpi_metric_card.h
 * @brief KPI 指标卡片：标题 + 数值，数值色随 KpiValueRole 变化
 *
 * 样式见 kpi_metric_card_style.h；role 属性驱动 ok/ng/warning 语义色。
 */

#include <QFrame>
#include "component_global.h"

class QLabel;

namespace ui {

/** 数值语义色（映射到 QFrame role 属性与 ThemeTokens 成功/危险/警告色） */
enum class KpiValueRole {
    Normal,  ///< textPrimary
    Ok,      ///< successText
    Ng,      ///< dangerText
    Warning  ///< warningText
};

/** 固定最小高度的 KPI 展示卡片（#kpiMetricCard） */
class UI_COMPONENT_EXPORT KpiMetricCard : public QFrame
{
    Q_OBJECT

public:
    explicit KpiMetricCard(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setValue(const QString &value);
    /** 设置数值语义色并刷新 QSS role 选择器 */
    void setValueRole(KpiValueRole role);

private:
    void bindTheme();
    void refreshStyle();

    QLabel       *m_titleLbl = nullptr;
    QLabel       *m_valueLbl = nullptr;
    KpiValueRole  m_role     = KpiValueRole::Normal;
};

} // namespace ui
