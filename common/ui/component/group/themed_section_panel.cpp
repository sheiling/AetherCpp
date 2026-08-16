/**
 * @file themed_section_panel.cpp
 * @brief ThemedSectionPanel 卡片布局、ThemeTokens 样式与标题几何
 */

#include "group/themed_section_panel.h"

#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace ui {

namespace {

QString verticalTitleText(const QString &horizontal)
{
    QString vertical;
    vertical.reserve(horizontal.size() * 2);
    for (int i = 0; i < horizontal.size(); ++i) {
        if (i > 0)
            vertical += QChar(u'\n');
        vertical += horizontal.at(i);
    }
    return vertical;
}

} // namespace

ThemedSectionPanel::ThemedSectionPanel(const QString &titleKey,
                                       QWidget *parent,
                                       int titleBarHeight,
                                       SectionTitlePlacement titlePlacement)
    : QFrame(parent)
    , m_titleKey(titleKey)
    , m_titleBarHeight(titleBarHeight)
    , m_titlePlacement(titlePlacement)
{
    setObjectName(QStringLiteral("themedSectionPanel"));
    setAttribute(Qt::WA_StyledBackground, true);
    buildLayout();
    bindTheme();
    retranslatePanel();
    refreshStyle();
}

void ThemedSectionPanel::buildLayout()
{
    if (!m_titleLabel) {
        m_titleLabel = new QLabel(this);
        m_titleLabel->setObjectName(QStringLiteral("themedSectionPanelTitle"));
        QFont titleFont = m_titleLabel->font();
        titleFont.setBold(true);
        m_titleLabel->setFont(titleFont);
    }

    if (!m_contentHost) {
        m_contentHost = new QWidget(this);
        m_contentHost->setObjectName(QStringLiteral("themedSectionPanelContent"));
        m_contentHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_contentLayout = new QVBoxLayout(m_contentHost);
        m_contentLayout->setContentsMargins(0, 0, 0, 0);
        m_contentLayout->setSpacing(4);
        m_contentLayout->setAlignment(Qt::AlignTop);
    }

    updateTitleGeometry();

    if (m_titlePlacement == SectionTitlePlacement::LeftVertical) {
        auto *h = new QHBoxLayout(this);
        h->setContentsMargins(10, 8, 10, 8);
        h->setSpacing(6);
        h->addWidget(m_titleLabel, 0);
        h->addWidget(m_contentHost, 1);
        m_rootLayout = h;
    } else {
        auto *v = new QVBoxLayout(this);
        v->setContentsMargins(10, 8, 10, 8);
        v->setSpacing(4);
        v->addWidget(m_titleLabel, 0);
        v->addWidget(m_contentHost, 1);
        m_rootLayout = v;
    }
}

void ThemedSectionPanel::setTitlePlacement(SectionTitlePlacement placement)
{
    if (m_titlePlacement == placement)
        return;
    m_titlePlacement = placement;

    delete m_rootLayout;
    m_rootLayout = nullptr;
    buildLayout();
    retranslatePanel();
    refreshStyle();
}

void ThemedSectionPanel::setTitleKey(const QString &key)
{
    m_titleKey = key;
    retranslatePanel();
}

void ThemedSectionPanel::setTitleBarHeight(int height)
{
    m_titleBarHeight = qMax(1, height);
    updateTitleGeometry();
}

void ThemedSectionPanel::updateTitleGeometry()
{
    if (!m_titleLabel)
        return;

    if (m_titlePlacement == SectionTitlePlacement::LeftVertical) {
        m_titleLabel->setFixedWidth(m_titleBarHeight);
        m_titleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_titleLabel->setWordWrap(true);
    } else {
        m_titleLabel->setFixedHeight(m_titleBarHeight);
        m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_titleLabel->setWordWrap(false);
    }
}

void ThemedSectionPanel::setContentWidget(QWidget *widget, int stretch)
{
    if (!m_contentLayout)
        return;

    while (QLayoutItem *item = m_contentLayout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->setParent(nullptr);
        delete item;
    }

    if (widget)
        m_contentLayout->addWidget(widget, stretch);
}

void ThemedSectionPanel::setContentMargins(int left, int top, int right, int bottom)
{
    if (m_rootLayout)
        m_rootLayout->setContentsMargins(left, top, right, bottom);
}

void ThemedSectionPanel::setContentSpacing(int spacing)
{
    if (m_rootLayout)
        m_rootLayout->setSpacing(spacing);
    if (m_contentLayout)
        m_contentLayout->setSpacing(spacing);
}

void ThemedSectionPanel::retranslatePanel()
{
    if (!m_titleLabel || m_titleKey.isEmpty())
        return;

    const QString horizontal = TR(m_titleKey.toUtf8().constData());
    if (m_titlePlacement == SectionTitlePlacement::LeftVertical)
        m_titleLabel->setText(verticalTitleText(horizontal));
    else
        m_titleLabel->setText(horizontal);
}

void ThemedSectionPanel::changeEvent(QEvent *event)
{
    QFrame::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslatePanel();
}

void ThemedSectionPanel::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

/** ThemeTokens → 面板背景/边框/标题色 QSS */
void ThemedSectionPanel::refreshStyle()
{
    const ThemeTokens t = ThemeTokens::resolve();
    setStyleSheet(QStringLiteral(
        "QFrame#themedSectionPanel {"
        "  background-color: %1; border: 1px solid %2; border-radius: 8px;"
        "}"
        "QLabel#themedSectionPanelTitle {"
        "  color: %3; background: transparent;"
        "}"
        "QWidget#themedSectionPanelContent {"
        "  background: transparent;"
        "}"
        "QWidget#themedSectionPanelContent > QWidget {"
        "  background: transparent;"
        "}")
        .arg(t.surface.name(), t.surfaceBorder.name(), t.textPrimary.name()));
}

} // namespace ui
