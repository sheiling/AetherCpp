/**
 * @file device_control_panel.cpp
 */

#include "home/device_control_panel.h"

#include "component/controls/button/themed_cyber_pill_button.h"
#include "component/group/themed_section_panel.h"
#include "component/theme/theme_color_math.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

constexpr int kPrimaryBtnCount = 3;
constexpr int kAuxBtnCount     = 3;
constexpr int kCtrlBtnCount    = kPrimaryBtnCount + kAuxBtnCount;

QWidget *makeTransparentHost(QWidget *parent)
{
    auto *host = new QWidget(parent);
    host->setAttribute(Qt::WA_StyledBackground, true);
    host->setAutoFillBackground(false);
    host->setStyleSheet(QStringLiteral("background: transparent;"));
    return host;
}

} // namespace

DeviceControlPanel::DeviceControlPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("deviceControlPanel"));
    setAttribute(Qt::WA_StyledBackground, true);
    buildLayout();
    bindThemeRefresh(this, [this]() { applyPanelStyle(); });
    applyPanelStyle();
    retranslateUi();
}

void DeviceControlPanel::applyPanelStyle()
{
    setStyleSheet(QStringLiteral("QWidget#deviceControlPanel { background: transparent; }"));

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    if (m_primaryHost) {
        const QColor bg     = themeBlend(t.surface, t.accent, 0.10);
        const QColor border = themeBlend(t.inputBorder, t.accent, 0.35);
        m_primaryHost->setStyleSheet(QStringLiteral(
            "QWidget#deviceCtrlPrimary {"
            "  background-color: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 6px;"
            "}"
        ).arg(bg.name(), border.name()));
    }

    if (m_auxHost) {
        const QColor border = themeBlend(t.inputBorder, t.textSecondary, 0.18);
        m_auxHost->setStyleSheet(QStringLiteral(
            "QWidget#deviceCtrlAux {"
            "  background-color: transparent;"
            "  border: 1px solid %1;"
            "  border-radius: 6px;"
            "}"
        ).arg(border.name()));
    }

    if (m_ctrlBtns.size() >= kPrimaryBtnCount) {
        m_ctrlBtns[0]->setAccentColor(t.accent);
        m_ctrlBtns[1]->setAccentColor(t.warningText);
        m_ctrlBtns[2]->setAccentColor(t.accentHover);
    }
}

ui::ThemedCyberPillButton *DeviceControlPanel::addCtrlButton(QWidget *parent, int pillHeight,
                                                         int minWidth, int hPad)
{
    auto *btn = ui::makeCyberPillButton(QString(), parent);
    btn->setPillHeight(pillHeight);
    btn->setMinPillWidth(minWidth);
    btn->setHorizontalPadding(hPad);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_ctrlBtns.append(btn);
    return btn;
}

void DeviceControlPanel::buildLayout()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_ctrlPanel = new ui::ThemedSectionPanel(QStringLiteral("aoi.sidebar.device_control"), this);
    auto *ctrlHost = makeTransparentHost(m_ctrlPanel->contentHost());
    auto *ctrlVBox = new QVBoxLayout(ctrlHost);
    ctrlVBox->setContentsMargins(0, 0, 0, 0);
    ctrlVBox->setSpacing(10);

    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();

    m_primaryHost = makeTransparentHost(ctrlHost);
    m_primaryHost->setObjectName(QStringLiteral("deviceCtrlPrimary"));
    auto *primaryRow = new QHBoxLayout(m_primaryHost);
    primaryRow->setContentsMargins(6, 6, 6, 6);
    primaryRow->setSpacing(8);

    addCtrlButton(m_primaryHost, 40, 72, 10)->setAccentColor(tokens.accent);
    addCtrlButton(m_primaryHost, 40, 72, 10)->setAccentColor(tokens.warningText);
    addCtrlButton(m_primaryHost, 40, 72, 10)->setAccentColor(tokens.accentHover);

    primaryRow->addWidget(m_ctrlBtns[0]);
    primaryRow->addWidget(m_ctrlBtns[1]);
    primaryRow->addWidget(m_ctrlBtns[2]);

    m_auxHost = makeTransparentHost(ctrlHost);
    m_auxHost->setObjectName(QStringLiteral("deviceCtrlAux"));
    auto *auxRow = new QHBoxLayout(m_auxHost);
    auxRow->setContentsMargins(6, 6, 6, 6);
    auxRow->setSpacing(8);

    for (int i = 0; i < kAuxBtnCount; ++i)
        addCtrlButton(m_auxHost, 32, 88, 12);

    for (int i = kPrimaryBtnCount; i < kCtrlBtnCount; ++i)
        auxRow->addWidget(m_ctrlBtns[i]);

    ctrlVBox->addWidget(m_primaryHost);
    ctrlVBox->addWidget(m_auxHost);
    m_ctrlPanel->contentLayout()->addWidget(ctrlHost);
    layout->addWidget(m_ctrlPanel);
}

void DeviceControlPanel::retranslateUi()
{
    if (m_ctrlPanel)
        m_ctrlPanel->retranslatePanel();

    const char *ctrlKeys[] = {
        "aoi.sidebar.btn.start",
        "aoi.sidebar.btn.pause",
        "aoi.sidebar.btn.machine_home",
        "aoi.sidebar.btn.clear_alarm",
        "aoi.sidebar.btn.clear_material",
        "aoi.sidebar.btn.single_inspect",
    };
    for (int i = 0; i < m_ctrlBtns.size() && i < kCtrlBtnCount; ++i)
        m_ctrlBtns[i]->setText(TR(ctrlKeys[i]));
}

} // namespace aoi
