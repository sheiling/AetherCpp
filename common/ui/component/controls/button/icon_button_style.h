#pragma once

/**
 * @file icon_button_style.h
 * @brief ThemedIconButton 动态 QSS（颜色来自 ThemeTokens）
 *
 * 无变体枚举；ThemeTokens 映射：
 * - 默认：inputBackground, textPrimary, inputBorder
 * - hover：iconButtonBgHover, inputBorderHover
 * - pressed：surface, inputBorder
 * - focus：focus
 * - disabled：textSecondary, surface, inputBorder
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 方形图标按钮尺寸（边长、圆角、glyph 字号） */
struct IconButtonStyleMetrics
{
    int size         = 28;
    int borderRadius = 4;
    int fontSize     = 14;
};

/** 生成 ThemedIconButton 实例级 QSS */
inline QString buildIconButtonStylesheet(const IconButtonStyleMetrics &m = {})
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "QPushButton {"
        "  min-width: %1px; max-width: %1px;"
        "  min-height: %1px; max-height: %1px;"
        "  padding: 0px;"
        "  border-radius: %2px;"
        "  font-family: %3; font-size: %4px;"
        "  background-color: %5; color: %6; border: 1px solid %7;"
        "}"
        "QPushButton:hover:enabled { background-color: %8; border: 1px solid %9; }"
        "QPushButton:pressed:enabled { background-color: %10; border: 1px solid %11; }"
        "QPushButton:focus { border: 1px solid %12; }"
        "QPushButton:disabled { color: %13; background-color: %14; border: 1px solid %15; }"
    ).arg(m.size)
        .arg(m.borderRadius)
        .arg(fontFamily)
        .arg(m.fontSize)
        .arg(t.inputBackground.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(t.iconButtonBgHover.name())
        .arg(t.inputBorderHover.name())
        .arg(t.surface.name())
        .arg(t.inputBorder.name())
        .arg(t.focus.name())
        .arg(t.textSecondary.name())
        .arg(t.surface.name())
        .arg(t.inputBorder.name());
}

} // namespace ui
