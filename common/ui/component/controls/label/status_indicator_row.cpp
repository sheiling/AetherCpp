/**
 * @file status_indicator_row.cpp
 * @brief StatusIndicatorRow：玻璃指示灯 + 分层背景 + 主/次级状态文案
 */

#include "controls/label/status_indicator_row.h"

#include "controls/label/status_indicator_row_style.h"
#include "controls/indicator/themed_glass_indicator_dot.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace ui {

StatusIndicatorRow::StatusIndicatorRow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("statusIndicatorRow"));
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumHeight(40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 10, 6);
    layout->setSpacing(8);

    m_dot = new ThemedGlassIndicatorDot(this);
    m_dot->setDiameter(10);

    m_nameLbl = new QLabel(this);
    m_nameLbl->setObjectName(QStringLiteral("statusIndicatorName"));

    m_subLbl = new QLabel(this);
    m_subLbl->setObjectName(QStringLiteral("statusIndicatorSub"));

    m_statusLbl = new QLabel(this);
    m_statusLbl->setObjectName(QStringLiteral("statusIndicatorText"));
    m_statusLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto *textCol = new QVBoxLayout();
    textCol->setContentsMargins(0, 0, 0, 0);
    textCol->setSpacing(1);
    textCol->addWidget(m_nameLbl);
    textCol->addWidget(m_subLbl);

    layout->addWidget(m_dot, 0, Qt::AlignVCenter);
    layout->addLayout(textCol, 1);
    layout->addWidget(m_statusLbl, 0, Qt::AlignVCenter);

    bindTheme();
    setIndicatorState(LinkIndicatorState::Unknown);
    refreshStyle();
}

void StatusIndicatorRow::setName(const QString &name)
{
    if (m_nameLbl)
        m_nameLbl->setText(name);
}

void StatusIndicatorRow::setStatusText(const QString &text)
{
    if (m_statusLbl)
        m_statusLbl->setText(text);
}

void StatusIndicatorRow::setSubStatusText(const QString &text)
{
    if (!m_subLbl)
        return;
    m_subLbl->setText(text);
    m_subLbl->setVisible(!text.isEmpty());
}

void StatusIndicatorRow::setIndicatorState(LinkIndicatorState state)
{
    m_state = state;
    if (m_dot)
        m_dot->setAccentColor(indicatorColorForState(state));
}

void StatusIndicatorRow::setIndicatorDiameter(int pixels)
{
    if (m_dot)
        m_dot->setDiameter(pixels);
}

void StatusIndicatorRow::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        setIndicatorState(m_state);
        refreshStyle();
    });
}

void StatusIndicatorRow::refreshStyle()
{
    setStyleSheet(buildStatusIndicatorRowStylesheet(ThemeTokens::resolve()));
}

QColor StatusIndicatorRow::indicatorColorForState(LinkIndicatorState state) const
{
    const ThemeTokens t = ThemeTokens::resolve();
    switch (state) {
    case LinkIndicatorState::Connected:
        return t.successText;
    case LinkIndicatorState::Standby:
        return t.warningText;
    case LinkIndicatorState::Disconnected:
        return t.dangerText;
    case LinkIndicatorState::Unknown:
    default:
        return t.textSecondary;
    }
}

} // namespace ui
