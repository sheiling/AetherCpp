#pragma once

/**
 * @file status_indicator_row_style.h
 * @brief StatusIndicatorRow 动态 QSS（颜色来自 ThemeTokens）
 */

#include "theme/theme_color_math.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 生成 #statusIndicatorRow 分层背景与子 QLabel 样式 */
inline QString buildStatusIndicatorRowStylesheet(const ThemeTokens &t)
{
    const QColor rowBg     = themeBlend(t.surface, t.accent, 0.08);
    const QColor rowBorder = themeBlend(t.inputBorder, t.accent, 0.18);

    return QStringLiteral(
        "QWidget#statusIndicatorRow {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"
        "QWidget#statusIndicatorRow QLabel#statusIndicatorName {"
        "  color: %3; font-size: 13px; font-weight: 500; background: transparent;"
        "}"
        "QWidget#statusIndicatorRow QLabel#statusIndicatorSub {"
        "  color: %4; font-size: 11px; background: transparent;"
        "}"
        "QWidget#statusIndicatorRow QLabel#statusIndicatorText {"
        "  color: %5; font-size: 12px; background: transparent;"
        "}"
    )
        .arg(rowBg.name())
        .arg(rowBorder.name())
        .arg(t.textPrimary.name())
        .arg(t.textSecondary.name())
        .arg(t.mutedText.name());
}

} // namespace ui
