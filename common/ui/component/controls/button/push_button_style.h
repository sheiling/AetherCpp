#pragma once

/**
 * @file push_button_style.h
 * @brief ThemedPushButton 动态 QSS（颜色来自 ThemeTokens）
 *
 * PushButtonVariant 与 ThemeTokens 映射：
 * - Secondary — 正常：inputBackground, textPrimary, inputBorder；hover: iconButtonBgHover,
 *   inputBorderHover；pressed: surface；focus: focus；disabled: textSecondary, surface；
 *   checked（可勾选）：successBg / successText / successBorder
 * - Primary   — 强调：accentBg, highlightText, accentBorder；hover: accentBgHover；
 *   pressed: accentPressed；focus: focus；disabled: textSecondary, surface, inputBorder
 * - Danger    — 危险：dangerBg, dangerText, dangerBorder；hover: dangerBgHover；
 *   pressed: dangerBgPressed；focus: dangerBorder；disabled: textSecondary, surface, inputBorder
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 按钮语义变体，决定 buildPushButtonStylesheet 使用的 ThemeTokens 子集 */
enum class PushButtonVariant {
    Secondary, ///< 默认描边/浅底按钮
    Primary,   ///< 主题 accent 填充的强调按钮
    Danger     ///< 危险操作（danger* 色系）
};

/** QSS 尺寸参数，可在调用 buildPushButtonStylesheet 时覆盖默认值 */
struct PushButtonStyleMetrics
{
    int minHeight    = 28;
    int borderRadius = 4;
    int paddingH     = 12;
    int paddingV     = 4;
};

/**
 * 按变体生成 ThemedPushButton 实例级 QSS 字符串
 * @param variant Secondary / Primary / Danger
 * @param m       可选尺寸覆盖（minHeight、圆角、内边距）
 */
inline QString buildPushButtonStylesheet(PushButtonVariant variant,
                                         const PushButtonStyleMetrics &m = {})
{
    const ThemeTokens t            = ThemeTokens::resolve();
    const QString     fontFamily   = themeFontFamilyCss();
    const QString     baseTemplate = QStringLiteral(
        "QPushButton {"
        "  min-height: %1px; padding: %2px %3px;"
        "  border-radius: %4px; font-family: %5; font-size: 12px;"
        "  background-color: %6; color: %7; border: 1px solid %8;"
        "}"
        "QPushButton:hover:enabled { background-color: %9; border: 1px solid %10; }"
        "QPushButton:pressed:enabled { background-color: %11; border: 1px solid %12; }"
        "QPushButton:focus { border: 1px solid %13; }"
        "QPushButton:disabled { color: %14; background-color: %15; border: 1px solid %16; }"
    );

    if (variant == PushButtonVariant::Primary) {
        return baseTemplate.arg(m.minHeight)
            .arg(m.paddingV)
            .arg(m.paddingH)
            .arg(m.borderRadius)
            .arg(fontFamily)
            .arg(t.accentBg.name())
            .arg(t.highlightText.name())
            .arg(t.accentBorder.name())
            .arg(t.accentBgHover.name())
            .arg(t.accentBorder.name())
            .arg(t.accentPressed.name())
            .arg(t.accentBorder.name())
            .arg(t.focus.name())
            .arg(t.textSecondary.name())
            .arg(t.surface.name())
            .arg(t.inputBorder.name());
    }

    if (variant == PushButtonVariant::Danger) {
        return baseTemplate.arg(m.minHeight)
            .arg(m.paddingV)
            .arg(m.paddingH)
            .arg(m.borderRadius)
            .arg(fontFamily)
            .arg(t.dangerBg.name())
            .arg(t.dangerText.name())
            .arg(t.dangerBorder.name())
            .arg(t.dangerBgHover.name())
            .arg(t.dangerBorder.name())
            .arg(t.dangerBgPressed.name())
            .arg(t.dangerBorder.name())
            .arg(t.dangerBorder.name())
            .arg(t.textSecondary.name())
            .arg(t.surface.name())
            .arg(t.inputBorder.name());
    }

    return baseTemplate.arg(m.minHeight)
        .arg(m.paddingV)
        .arg(m.paddingH)
        .arg(m.borderRadius)
        .arg(fontFamily)
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
        .arg(t.inputBorder.name())
        + QStringLiteral(
              "QPushButton:checked {"
              "  background-color: %1; color: %2; border: 1px solid %3;"
              "}"
              "QPushButton:checked:hover:enabled {"
              "  background-color: %4; border: 1px solid %3;"
              "}"
              "QPushButton:checked:pressed:enabled {"
              "  background-color: %5; border: 1px solid %3;"
              "}")
              .arg(t.successBg.name(), t.successText.name(), t.successBorder.name(),
                   t.successBgHover.name(), t.successBgPressed.name());
}

} // namespace ui
