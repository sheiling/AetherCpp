/**
 * @file themed_slider.cpp
 * @brief ThemedSlider 实现：slider_style QSS 与主题绑定
 */

#include "controls/slider/themed_slider.h"

#include "controls/slider/slider_style.h"

#include "theme/theme_refresh.h"

#include <QEvent>

namespace ui {

ThemedSlider::ThemedSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
    bindTheme();
    refreshStyle();
}

void ThemedSlider::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedSlider::refreshStyle()
{
    setStyleSheet(buildSliderStylesheet());
}

void ThemedSlider::changeEvent(QEvent *event)
{
    QSlider::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
}

ThemedSlider *makeHorizontalSlider(QWidget *parent)
{
    return new ThemedSlider(Qt::Horizontal, parent);
}

} // namespace ui
