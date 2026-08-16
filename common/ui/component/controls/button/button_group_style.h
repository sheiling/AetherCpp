#pragma once

/**
 * @file button_group_style.h
 * @brief ThemedButtonGroup 互斥按钮组 QSS（ThemeTokens）
 *
 * 单一样式（无变体枚举）；ThemeTokens 映射：
 * - 未选中：inputBackground, textPrimary, inputBorder
 * - 选中：accent, highlightText, accentBorder
 * - hover 未选中：iconButtonBgHover, inputBorderHover
 * - hover 选中：accentHover, accentBorder
 * - disabled：textSecondary, inputBackground, inputBorder
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 生成 #themedButtonGroup / #themedButtonGroupBtn 选择器 QSS */
inline QString buildButtonGroupStylesheet()
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "#themedButtonGroup { background: transparent; }"
        "#themedButtonGroup QPushButton#themedButtonGroupBtn {"
        "  min-height: 28px; padding: 2px 8px;"
        "  border-radius: 0px;"
        "  font-family: %1; font-size: 11px;"
        "  background-color: %2; color: %3; border: 1px solid %4;"
        "}"
        "#themedButtonGroup QPushButton#themedButtonGroupBtn:checked {"
        "  background-color: %5; color: %6; border: 1px solid %7;"
        "}"
        "#themedButtonGroup QPushButton#themedButtonGroupBtn:hover:enabled {"
        "  background-color: %8; border: 1px solid %9;"
        "}"
        "#themedButtonGroup QPushButton#themedButtonGroupBtn:checked:hover:enabled {"
        "  background-color: %10; border: 1px solid %7;"
        "}"
        "#themedButtonGroup QPushButton#themedButtonGroupBtn:disabled {"
        "  color: %11; background-color: %2; border: 1px solid %4;"
        "}"
    ).arg(fontFamily,
          t.inputBackground.name(),
          t.textPrimary.name(),
          t.inputBorder.name(),
          t.accent.name(),
          t.highlightText.name(),
          t.accentBorder.name(),
          t.iconButtonBgHover.name(),
          t.inputBorderHover.name(),
          t.accentHover.name(),
          t.textSecondary.name());
}

} // namespace ui
