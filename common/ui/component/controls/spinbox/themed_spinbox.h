#pragma once

/**
 * @file themed_spinbox.h
 * @brief 主题 SpinBox：ThemeTokens 动态 QSS 绘制本体文字/边框 + 自绘三角箭头
 *
 * QSS 见 spinbox_style.h；上下箭头由 spinbox_paint_utils 在 paintEvent 中覆盖绘制。
 * minimumSizeHint 按当前数值文本宽度计算，避免折叠面板展开时撑宽布局；
 * resizeEvent 中手动同步 lineEdit 宽度，避免 QSS 下编辑区不随外框拉宽。
 */

#include "component_global.h"
#include "controls/spinbox/spinbox_paint_utils.h"

#include <QDoubleSpinBox>
#include <QSpinBox>

class QEvent;
class QResizeEvent;
class QWidget;

namespace ui {

/** 整数 SpinBox，自绘箭头，Expanding 水平策略 */
class UI_COMPONENT_EXPORT ThemedSpinBox final : public QSpinBox
{
    Q_OBJECT
public:
    explicit ThemedSpinBox(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void syncLineEditGeometry();
};

/** 浮点 SpinBox，keyboardTracking=false，其余与 ThemedSpinBox 一致 */
class UI_COMPONENT_EXPORT ThemedDoubleSpinBox final : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit ThemedDoubleSpinBox(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void syncLineEditGeometry();
};

UI_COMPONENT_EXPORT ThemedSpinBox       *makeSpinBox(QWidget *parent, const QString &objectName = QString());
UI_COMPONENT_EXPORT ThemedDoubleSpinBox *makeDoubleSpinBox(QWidget *parent, const QString &objectName = QString());

UI_COMPONENT_EXPORT void ensureThemedSpinStyles(QWidget *root);

} // namespace ui
