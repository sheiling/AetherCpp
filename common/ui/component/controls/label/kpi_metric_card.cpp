/**
 * @file kpi_metric_card.cpp
 * @brief KpiMetricCard 实现：标题/数值布局、role 属性与主题 QSS
 */

#include "controls/label/kpi_metric_card.h"

#include "controls/label/kpi_metric_card_style.h"
#include "theme/theme_refresh.h"

#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

namespace ui {

namespace {

QString roleProperty(KpiValueRole role)
{
    switch (role) {
    case KpiValueRole::Ok:
        return QStringLiteral("ok");
    case KpiValueRole::Ng:
        return QStringLiteral("ng");
    case KpiValueRole::Warning:
        return QStringLiteral("warning");
    case KpiValueRole::Normal:
    default:
        return QStringLiteral("normal");
    }
}

} // namespace

KpiMetricCard::KpiMetricCard(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("kpiMetricCard"));
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumHeight(58);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(2);

    m_titleLbl = new QLabel(this);
    m_titleLbl->setObjectName(QStringLiteral("kpiMetricTitle"));
    m_valueLbl = new QLabel(this);
    m_valueLbl->setObjectName(QStringLiteral("kpiMetricValue"));

    layout->addWidget(m_titleLbl);
    layout->addWidget(m_valueLbl);

    bindTheme();
    refreshStyle();
}

void KpiMetricCard::setTitle(const QString &title)
{
    if (m_titleLbl)
        m_titleLbl->setText(title);
}

void KpiMetricCard::setValue(const QString &value)
{
    if (m_valueLbl)
        m_valueLbl->setText(value);
}

void KpiMetricCard::setValueRole(KpiValueRole role)
{
    m_role = role;
    setProperty("role", roleProperty(role).toLatin1().constData());
    refreshStyle();
}

void KpiMetricCard::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void KpiMetricCard::refreshStyle()
{
    setStyleSheet(buildKpiMetricCardStylesheet(ThemeTokens::resolve()));
}

} // namespace ui
