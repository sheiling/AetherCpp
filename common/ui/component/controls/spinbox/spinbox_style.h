#pragma once

/**
 * @file spinbox_style.h
 * @brief ThemedSpinBox / ThemedDoubleSpinBox 动态 QSS（颜色来自 ThemeTokens）
 *
 * 全局 ACSS 将 QSpinBox 文字设为 primaryColor，暗色主题下与背景对比不足；
 * 本控件在实例级 QSS 中改用 primaryTextColor（textPrimary）。
 *
 * ThemeTokens 映射（QSpinBox / QDoubleSpinBox 共用）：
 * - 默认：inputBackground, textPrimary, inputBorder
 * - hover 边框：inputBorderHover
 * - focus：focus
 * - disabled：textSecondary
 * - 选区：accent
 * - 自绘箭头背景（paintThemedSpinArrows）：spinButtonBg；箭头色：arrowActive / arrowDisabled
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

class QAbstractSpinBox;

namespace ui {

/** QSS 与布局尺寸（含右侧步进按钮区宽度） */
struct SpinBoxStyleMetrics
{
    int minHeight    = 28;
    int borderRadius = 4;
    int paddingH     = 8;
    int paddingV     = 2;
    int buttonWidth  = 20;
};

/** 生成 SpinBox 本体 QSS（隐藏原生箭头，由 spinbox_paint_utils 自绘） */
inline QString buildSpinBoxStylesheet(const SpinBoxStyleMetrics &m = {})
{
    const ThemeTokens t            = ThemeTokens::resolve();
    const QString     fontFamily   = themeFontFamilyCss();
    const int         paddingRight = m.paddingH + m.buttonWidth;

    return QStringLiteral(
        "QSpinBox, QDoubleSpinBox {"
        "  min-height: %1px; padding: %2px %3px %2px %4px;"
        "  background-color: %5; color: %6;"
        "  border: 1px solid %7; border-radius: %8px;"
        "  font-family: %9; font-size: 12px;"
        "  selection-background-color: %10; selection-color: %6;"
        "}"
        "QSpinBox:hover:enabled, QDoubleSpinBox:hover:enabled {"
        "  border: 1px solid %11;"
        "}"
        "QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border: 1px solid %12;"
        "}"
        "QSpinBox:disabled, QDoubleSpinBox:disabled {"
        "  color: %13; background-color: %5; border: 1px solid %7;"
        "}"
        "QSpinBox::up-button, QDoubleSpinBox::up-button,"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  subcontrol-origin: border;"
        "  width: %14px; border: none; background: transparent;"
        "}"
        "QSpinBox::up-button, QDoubleSpinBox::up-button {"
        "  subcontrol-position: top right;"
        "}"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  subcontrol-position: bottom right;"
        "}"
        "QSpinBox::up-arrow, QDoubleSpinBox::up-arrow,"
        "QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {"
        "  width: 0; height: 0; image: none;"
        "}"
    ).arg(m.minHeight)
        .arg(m.paddingV)
        .arg(paddingRight)
        .arg(m.paddingH)
        .arg(t.inputBackground.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(m.borderRadius)
        .arg(fontFamily)
        .arg(t.accent.name())
        .arg(t.inputBorderHover.name())
        .arg(t.focus.name())
        .arg(t.textSecondary.name())
        .arg(m.buttonWidth);
}

/** 对 QAbstractSpinBox 实例应用 buildSpinBoxStylesheet */
inline void applyThemedSpinBoxStyle(QAbstractSpinBox *spin)
{
    if (!spin)
        return;
    spin->setStyleSheet(buildSpinBoxStylesheet());
}

} // namespace ui
