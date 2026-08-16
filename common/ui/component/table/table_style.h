#pragma once

/**
 * @file table_style.h
 * @brief ThemedTableWidget 动态 QSS（颜色来自 ThemeTokens）
 *
 * 表头背景由 ThemedTableHeaderView 自绘；QSS 仅保留透明 section 与角区渐变。
 */

#include "theme/theme_color_math.h"
#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** buildTableWidgetStylesheet 可调 padding / 圆角 token */
struct TableStyleMetrics
{
    int headerPaddingV = 8;  ///< QHeaderView::section 上下内边距
    int headerPaddingH = 8;  ///< 表头左右内边距
    int cellPaddingV   = 4;  ///< 单元格上下内边距
    int cellPaddingH   = 8;  ///< 单元格左右内边距
    int borderRadius   = 4;  ///< 表格外框圆角
};

/** 表头渐变三色（与 ThemedTableHeaderView 自绘一致，供角按钮 QSS） */
inline void tableHeaderGradientColors(const ThemeTokens &t, bool lightTheme, QColor *top,
                                      QColor *mid, QColor *bottom)
{
    const QColor glassBase = themeBlend(t.headerBg, t.surfaceElevated, 0.28);
    if (top)
        *top = themeBlend(glassBase, t.accentHover, lightTheme ? 0.06 : 0.10);
    if (mid)
        *mid = themeBlend(t.headerBg, t.surfaceElevated, lightTheme ? 0.18 : 0.22);
    if (bottom)
        *bottom = themeBlend(t.headerBg, t.inputBackground, lightTheme ? 0.12 : 0.16);
}

inline bool tableThemeIsLight(const ThemeTokens &t)
{
    if (!t.surfaceElevated.isValid())
        return false;
    const QColor &c = t.surfaceElevated;
    const qreal   lum =
        0.2126 * c.redF() + 0.7152 * c.greenF() + 0.0722 * c.blueF();
    return lum > 0.45;
}

/**
 * 生成 QTableWidget + QHeaderView + 角按钮完整 QSS
 * Token 映射：inputBackground(单元格底)、textPrimary(表格默认字色)、inputBorder(边框/网格)、
 * divider(网格线)、accent(选中项)；单元格字色优先取 QTableWidgetItem::foreground，便于按列着色
 */
inline QString buildTableWidgetStylesheet(const TableStyleMetrics &m = {})
{
    const ThemeTokens t            = ThemeTokens::resolve();
    const QString     fontFamily   = themeFontFamilyCss();
    const bool        lightTheme   = tableThemeIsLight(t);
    QColor            headerTop;
    QColor            headerMid;
    QColor            headerBottom;
    tableHeaderGradientColors(t, lightTheme, &headerTop, &headerMid, &headerBottom);

    return QStringLiteral(
        "QTableWidget {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: %4px;"
        "  gridline-color: %5;"
        "  font-family: %6; font-size: 12px;"
        "  outline: none;"
        "}"
        "QTableWidget::item {"
        "  padding: %7px %8px; border: none;"
        "  background-color: %1;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: %9;"
        "}"
        "QHeaderView { background: transparent; }"
        "QHeaderView::section {"
        "  background: transparent; color: transparent;"
        "  padding: %10px %11px;"
        "  border: none;"
        "  font-family: %6; font-size: 12px; font-weight: bold;"
        "}"
        "QTableCornerButton::section {"
        "  border: none;"
        "  border-bottom: 1px solid %3;"
        "  background-color: qlineargradient("
        "    spread:pad, x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 %12, stop:0.42 %13, stop:0.78 %14, stop:1 %14);"
        "}"
    ).arg(t.inputBackground.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(m.borderRadius)
        .arg(t.divider.name())
        .arg(fontFamily)
        .arg(m.cellPaddingV)
        .arg(m.cellPaddingH)
        .arg(t.accent.name())
        .arg(m.headerPaddingV)
        .arg(m.headerPaddingH)
        .arg(headerTop.name())
        .arg(headerMid.name())
        .arg(headerBottom.name());
}

} // namespace ui
