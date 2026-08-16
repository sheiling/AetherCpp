#pragma once

/**
 * @file themed_slider.h
 * @brief 主题 QSlider：水平/垂直方向，groove/handle 色来自 ThemeTokens
 *
 * 样式见 slider_style.h（当前 QSS 定义 horizontal 子控件）；makeHorizontalSlider 为常用工厂。
 */

#include <QSlider>
#include "component_global.h"

class QEvent;

namespace ui {

/** 随 ThemeTokens 刷新的 QSlider */
class UI_COMPONENT_EXPORT ThemedSlider : public QSlider
{
    Q_OBJECT

public:
    explicit ThemedSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
};

/** 工厂：创建水平 ThemedSlider */
UI_COMPONENT_EXPORT ThemedSlider *makeHorizontalSlider(QWidget *parent = nullptr);

} // namespace ui
