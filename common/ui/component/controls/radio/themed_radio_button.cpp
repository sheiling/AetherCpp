/**
 * @file themed_radio_button.cpp
 * @brief ThemedRadioButton 实现：radio_button_style QSS 与主题绑定
 */

#include "controls/radio/themed_radio_button.h"

#include "controls/radio/radio_button_style.h"

#include "theme/theme_refresh.h"

#include <QEvent>

namespace ui {

ThemedRadioButton::ThemedRadioButton(QWidget *parent)
    : QRadioButton(parent)
{
    bindTheme();
    refreshStyle();
}

ThemedRadioButton::ThemedRadioButton(const QString &text, QWidget *parent)
    : QRadioButton(text, parent)
{
    bindTheme();
    refreshStyle();
}

void ThemedRadioButton::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedRadioButton::refreshStyle()
{
    setStyleSheet(buildRadioButtonStylesheet());
}

void ThemedRadioButton::changeEvent(QEvent *event)
{
    QRadioButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
}

ThemedRadioButton *makeRadioButton(const QString &text, QWidget *parent)
{
    return new ThemedRadioButton(text, parent);
}

} // namespace ui
