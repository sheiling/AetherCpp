#pragma once

/**
 * @file themed_combo_box.h
 * @brief 主题 QComboBox：动态 QSS、弹出列表样式与宽度适配
 *
 * 本体样式见 combo_box_style.h；下拉箭头在 paintEvent 自绘（ThemeTokens 箭头色）；
 * showPopup 时为独立弹出窗口单独设置 list view QSS 与最小宽度。
 */

#include <QComboBox>
#include "component_global.h"

class QEvent;
class QPaintEvent;

namespace ui {

/** 随 ThemeTokens 刷新的 QComboBox，minimumSizeHint 受 maximumWidth 约束 */
class UI_COMPONENT_EXPORT ThemedComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ThemedComboBox(QWidget *parent = nullptr);

    /** 避免最长下拉项撑宽父布局；尊重 maximumWidth 与当前 width */
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    /** 弹出后应用 list view QSS 并按内容/控件宽度调整 popup 宽度 */
    void  showPopup() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
};

/** 工厂：创建 ThemedComboBox */
UI_COMPONENT_EXPORT ThemedComboBox *makeComboBox(QWidget *parent = nullptr);

} // namespace ui
