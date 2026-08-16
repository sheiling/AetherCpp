/**
 * @file home_process_flow_widget.cpp
 * @brief 首页工艺流程条实现：三步骤 label + 主题色 active 高亮
 */

#include "home/home_process_flow_widget.h"

#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QLabel>

namespace aoi {

using namespace ui;

namespace {

/** 创建统一样式的流程步骤 label（objectName=homeProcessStep） */
QLabel *makeStepLabel(QWidget *parent)
{
    auto *lbl = new QLabel(parent);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setMinimumHeight(32);
    lbl->setMinimumWidth(72);
    lbl->setObjectName(QStringLiteral("homeProcessStep"));
    return lbl;
}

} // namespace

// ── 构造 / 布局 ──

HomeProcessFlowWidget::HomeProcessFlowWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("homeProcessFlow"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    layout->addStretch();

    m_feedLbl = makeStepLabel(this);
    auto *arrow1 = new QLabel(QStringLiteral("\u2192"), this);
    arrow1->setObjectName(QStringLiteral("homeProcessArrow"));
    m_inspectLbl = makeStepLabel(this);
    auto *arrow2 = new QLabel(QStringLiteral("\u2192"), this);
    arrow2->setObjectName(QStringLiteral("homeProcessArrow"));
    m_dischargeLbl = makeStepLabel(this);

    layout->addWidget(m_feedLbl);
    layout->addWidget(arrow1);
    layout->addWidget(m_inspectLbl);
    layout->addWidget(arrow2);
    layout->addWidget(m_dischargeLbl);
    layout->addStretch();

    bindTheme();
    retranslateUi();
    updateStepStyles();
}

// ── 状态 / 国际化 ──

void HomeProcessFlowWidget::setActiveStep(HomeProcessStep step)
{
    m_active = step;
    updateStepStyles();
}

void HomeProcessFlowWidget::retranslateUi()
{
    if (m_feedLbl)
        m_feedLbl->setText(TR("aoi.home.process.feed"));
    if (m_inspectLbl)
        m_inspectLbl->setText(TR("aoi.home.process.inspect"));
    if (m_dischargeLbl)
        m_dischargeLbl->setText(TR("aoi.home.process.discharge"));
}

// ── 主题 ──

void HomeProcessFlowWidget::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        refreshStyle();
        updateStepStyles();
    });
}

void HomeProcessFlowWidget::refreshStyle()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    setStyleSheet(QStringLiteral(
                      "QLabel#homeProcessArrow { color: %1; background: transparent; }"
                      "QLabel#homeProcessStep { border-radius: 4px; padding: 4px 12px; }")
                      .arg(t.textSecondary.name()));
}

void HomeProcessFlowWidget::updateStepStyles()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    const auto styleFor = [&](QLabel *lbl, bool active) {
        if (!lbl)
            return;
        if (active) {
            lbl->setStyleSheet(QStringLiteral(
                "QLabel#homeProcessStep {"
                "  background-color: %1; color: %2; border: 1px solid %3;"
                "  border-radius: 4px; padding: 4px 12px; font-weight: 600;"
                "}")
                                   .arg(t.accent.name(), t.highlightText.name(), t.accentBorder.name()));
        } else {
            lbl->setStyleSheet(QStringLiteral(
                "QLabel#homeProcessStep {"
                "  background-color: %1; color: %2; border: 1px solid %3;"
                "  border-radius: 4px; padding: 4px 12px;"
                "}")
                                   .arg(t.surface.name(), t.textSecondary.name(), t.surfaceBorder.name()));
        }
    };

    styleFor(m_feedLbl, m_active == HomeProcessStep::Feed);
    styleFor(m_inspectLbl, m_active == HomeProcessStep::Inspect);
    styleFor(m_dischargeLbl, m_active == HomeProcessStep::Discharge);
}

} // namespace aoi
