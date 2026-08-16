/**
 * @file themed_button_group.cpp
 * @brief ThemedButtonGroup 实现：按钮重建、互斥选中、主题与语言刷新
 */

#include "controls/button/themed_button_group.h"

#include "controls/button/button_group_style.h"
#include "theme/theme_refresh.h"

#include "uibase/languagemanager.h"

#include <QButtonGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalBlocker>

namespace ui {

ThemedButtonGroup::ThemedButtonGroup(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("themedButtonGroup"));
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    connect(m_buttonGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (m_currentIndex == id)
            return;
        m_currentIndex = id;
        emit currentIndexChanged(id);
    });

    if (LM) {
        connect(LM, &LanguageManager::languageChanged, this,
                [this](LanguageManager::Language) { retranslateButtons(); });
    }

    bindTheme();
    refreshStyle();
}

void ThemedButtonGroup::setItemKeys(const QStringList &translationKeys)
{
    m_itemKeys = translationKeys;
    rebuildButtons();
}

void ThemedButtonGroup::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_buttons.size())
        return;
    if (auto *btn = m_buttons.at(index)) {
        QSignalBlocker blocker(m_buttonGroup);
        btn->setChecked(true);
        m_currentIndex = index;
    }
}

int ThemedButtonGroup::currentIndex() const
{
    if (!m_buttonGroup)
        return -1;
    return m_buttonGroup->checkedId();
}

void ThemedButtonGroup::setStretchFactors(bool equalWidth)
{
    m_equalWidth = equalWidth;
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (auto *btn = m_buttons.at(i))
            m_layout->setStretch(m_layout->indexOf(btn), equalWidth ? 1 : 0);
    }
}

void ThemedButtonGroup::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedButtonGroup::refreshStyle()
{
    setStyleSheet(buildButtonGroupStylesheet());
}

void ThemedButtonGroup::rebuildButtons()
{
    if (!m_layout || !m_buttonGroup)
        return;

    for (auto *btn : m_buttons) {
        m_buttonGroup->removeButton(btn);
        m_layout->removeWidget(btn);
        btn->deleteLater();
    }
    m_buttons.clear();
    m_currentIndex = -1;

    for (int i = 0; i < m_itemKeys.size(); ++i) {
        auto *btn = new QPushButton(this);
        btn->setObjectName(QStringLiteral("themedButtonGroupBtn"));
        btn->setCheckable(true);
        btn->setMinimumWidth(0);
        btn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        btn->setText(TR(m_itemKeys.at(i).toUtf8().constData()));
        m_buttonGroup->addButton(btn, i);
        m_buttons.append(btn);
        m_layout->addWidget(btn, m_equalWidth ? 1 : 0);
    }

    if (!m_buttons.isEmpty()) {
        m_currentIndex = 0;
        if (auto *first = m_buttons.first())
            first->setChecked(true);
    }

    refreshStyle();
}

void ThemedButtonGroup::retranslateButtons()
{
    for (int i = 0; i < m_buttons.size() && i < m_itemKeys.size(); ++i) {
        if (auto *btn = m_buttons.at(i))
            btn->setText(TR(m_itemKeys.at(i).toUtf8().constData()));
    }
}

void ThemedButtonGroup::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateButtons();
}

} // namespace ui
