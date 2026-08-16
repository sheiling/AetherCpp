#pragma once

/**
 * @file combo_box_style.h
 * @brief ThemedComboBox 动态 QSS（颜色来自 ThemeTokens）
 *
 * buildComboBoxStylesheet（控件本体）ThemeTokens 映射：
 * - 默认：inputBackground, textPrimary, inputBorder
 * - hover：inputBorderHover
 * - on / focus：focus
 * - disabled：textSecondary, inputBackground, inputBorder
 *
 * 下拉箭头不在 QSS 中使用 SVG；ThemedComboBox::paintEvent 自绘三角（见 spinbox_paint_utils）。
 *
 * buildComboBoxListViewStylesheet（弹出 QAbstractItemView）ThemeTokens 映射：
 * - 列表背景/文字/边框：inputBackground, textPrimary, inputBorder
 * - 选中项：accent
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/**
 * 生成 QComboBox 本体 QSS
 * @param rootSelector 非空时作为选择器前缀（如 \"#myPanel\"），便于局部作用域
 */
inline QString buildComboBoxStylesheet(const QString &rootSelector = QString())
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();
    const QString     scope      = rootSelector.isEmpty() ? QString() : rootSelector;

    return QStringLiteral(
        "%1 QComboBox {"
        "  min-height: 0; min-width: 0; padding: 1px 28px 1px 8px;"
        "  background-color: %2; color: %3;"
        "  border: 1px solid %4; border-radius: 4px; font-family: %7; font-size: 12px;"
        "}"
        "%1 QComboBox:hover:enabled { border: 1px solid %5; }"
        "%1 QComboBox:on { border: 1px solid %6; }"
        "%1 QComboBox:disabled { color: %8; background-color: %2; border: 1px solid %4; }"
        "%1 QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  width: 24px; border: none; background: transparent;"
        "}"
        "%1 QComboBox::down-arrow {"
        "  width: 0; height: 0; border: none; image: none;"
        "}"
        "%1 QComboBox:focus { border: 1px solid %6; }"
    ).arg(scope,
          t.inputBackground.name(),
          t.textPrimary.name(),
          t.inputBorder.name(),
          t.inputBorderHover.name(),
          t.focus.name(),
          fontFamily,
          t.textSecondary.name());
}

/** 下拉列表（QComboBox 内部 view）专用样式：弹出层为独立窗口，需单独设置 */
inline QString buildComboBoxListViewStylesheet()
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "QAbstractItemView {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 4px;"
        "  selection-background-color: %4; selection-color: %2;"
        "  outline: none; padding: 4px 0;"
        "  font-family: %5; font-size: 12px;"
        "}"
        "QAbstractItemView::item {"
        "  height: 36px; min-height: 36px; padding: 8px 14px;"
        "}"
        "QAbstractItemView::item:selected {"
        "  background-color: %4; color: %2;"
        "}"
    ).arg(t.inputBackground.name(),
          t.textPrimary.name(),
          t.inputBorder.name(),
          t.accent.name(),
          fontFamily);
}

} // namespace ui
