#pragma once

/**
 * @file radio_button_style.h
 * @brief ThemedRadioButton 动态 QSS（颜色来自 ThemeTokens）
 *
 * 无变体枚举；ThemeTokens 映射：
 * - 标签文字：textPrimary；disabled：textSecondary
 * - 未选中指示器：inputBorder, inputBackground；hover 边框：inputBorderHover
 * - 选中指示器：accent
 * - 选中且 disabled：inputBorder, textSecondary
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 生成 ThemedRadioButton 实例级 QSS */
inline QString buildRadioButtonStylesheet()
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "QRadioButton {"
        "  spacing: 6px; color: %1; font-family: %2; font-size: 12px;"
        "}"
        "QRadioButton:disabled { color: %3; }"
        "QRadioButton::indicator {"
        "  width: 16px; height: 16px;"
        "}"
        "QRadioButton::indicator:unchecked {"
        "  border: 1px solid %4; border-radius: 8px; background: %5;"
        "}"
        "QRadioButton::indicator:unchecked:hover { border: 1px solid %6; }"
        "QRadioButton::indicator:checked {"
        "  border: 1px solid %7; border-radius: 8px; background: %7;"
        "}"
        "QRadioButton::indicator:checked:disabled {"
        "  border: 1px solid %4; background: %3;"
        "}"
    ).arg(t.textPrimary.name(),
          fontFamily,
          t.textSecondary.name(),
          t.inputBorder.name(),
          t.inputBackground.name(),
          t.inputBorderHover.name(),
          t.accent.name());
}

} // namespace ui
