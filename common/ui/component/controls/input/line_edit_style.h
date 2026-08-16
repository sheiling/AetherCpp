#pragma once

/**
 * @file line_edit_style.h
 * @brief ThemedLineEdit / PathLineEdit 动态 QSS（颜色来自 ThemeTokens）
 *
 * buildLineEditStylesheet ThemeTokens 映射：
 * - 默认：inputBackground, textPrimary, inputBorder
 * - hover：inputBorderHover
 * - focus：focus
 * - disabled / read-only：textSecondary, inputBackground
 * - 选区：accent
 *
 * buildPathLineEditStylesheet：在 LineEdit 基础上增大 paddingH（≥10），其余 token 相同
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QtGlobal>

#include <QString>

namespace ui {

/** 单行输入框 QSS 尺寸参数 */
struct LineEditStyleMetrics
{
    int minHeight     = 28;
    int borderRadius  = 4;
    int paddingH      = 8;
    int paddingV      = 2;
};

/** 标准单行输入框样式（setStyleSheet 作用于 ThemedLineEdit 自身） */
inline QString buildLineEditStylesheet(const LineEditStyleMetrics &m = {})
{
    const ThemeTokens t = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
        "QLineEdit {"
        "  min-height: %1px; padding: %2px %3px;"
        "  background-color: %4; color: %5;"
        "  border: 1px solid %6; border-radius: %7px;"
        "  font-family: %8; font-size: 12px;"
        "  selection-background-color: %9; selection-color: %5;"
        "}"
        "QLineEdit:hover:enabled { border: 1px solid %10; }"
        "QLineEdit:focus { border: 1px solid %11; }"
        "QLineEdit:disabled { color: %12; border: 1px solid %6; background-color: %4; }"
        "QLineEdit:read-only:enabled { color: %5; background-color: %4; }"
    ).arg(m.minHeight)
        .arg(m.paddingV)
        .arg(m.paddingH)
        .arg(t.inputBackground.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(m.borderRadius)
        .arg(fontFamily)
        .arg(t.accent.name())
        .arg(t.inputBorderHover.name())
        .arg(t.focus.name())
        .arg(t.textSecondary.name());
}

/** 路径输入框：略增水平内边距，便于长路径阅读 */
inline QString buildPathLineEditStylesheet(const LineEditStyleMetrics &m = {})
{
    LineEditStyleMetrics pathMetrics = m;
    pathMetrics.paddingH             = qMax(m.paddingH, 10);
    return buildLineEditStylesheet(pathMetrics);
}

} // namespace ui
