#pragma once

/**
 * @file slider_style.h
 * @brief ThemedSlider 动态 QSS（颜色来自 ThemeTokens）
 *
 * 无变体枚举；ThemeTokens 映射（水平滑条）：
 * - groove / add-page：inputBorder
 * - sub-page / handle 默认：accent, accentBorder
 * - handle hover：accentHover, accentBorder
 * - handle disabled：textSecondary, inputBorder
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 生成 ThemedSlider 实例级 QSS（当前仅定义 horizontal 子控件） */
inline QString buildSliderStylesheet()
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "QSlider::groove:horizontal {"
        "  height: 4px; background: %1; border-radius: 2px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background: %2; border-radius: 2px;"
        "}"
        "QSlider::add-page:horizontal {"
        "  background: %1; border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "  width: 14px; margin: -5px 0;"
        "  background: %2; border: 1px solid %3; border-radius: 7px;"
        "}"
        "QSlider::handle:horizontal:hover { background: %4; border: 1px solid %3; }"
        "QSlider::handle:horizontal:disabled { background: %5; border: 1px solid %1; }"
    ).arg(t.inputBorder.name(),
          t.accent.name(),
          t.accentBorder.name(),
          t.accentHover.name(),
          t.textSecondary.name());
}

} // namespace ui
