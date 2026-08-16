#pragma once

/**
 * @file spinbox_paint_utils.h
 * @brief SpinBox 箭头三角绘制（ThemedSpinBox 自绘步进箭头）
 *
 * 箭头颜色：ThemeTokens::arrowActive（启用）/ arrowDisabled（禁用）。
 * 步进按钮背景：ThemeTokens::spinButtonBg（paintThemedSpinArrows）。
 */

#include <QRect>

class QAbstractSpinBox;
class QPainter;
class QStyleOptionSpinBox;

namespace ui {

/** 三角箭头朝向，供 paintTriangleArrow 使用 */
enum class TriangleDirection {
    Up,    ///< 向上（SpinBox 增）
    Down,  ///< 向下（SpinBox 减）
    Left,  ///< 向左
    Right  ///< 向右
};

/** 在 rect 中心绘制实心三角箭头 */
void paintTriangleArrow(QPainter &p, const QRect &rect, TriangleDirection dir, bool enabled);

/** 在 SpinBox 上下按钮区填充 spinButtonBg 并绘制 Up/Down 箭头 */
void paintThemedSpinArrows(QAbstractSpinBox *spin, const QStyleOptionSpinBox &opt);

} // namespace ui
