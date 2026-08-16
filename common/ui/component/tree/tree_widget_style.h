#pragma once

/**
 * @file tree_widget_style.h
 * @brief ThemedTreeWidget 动态 QSS（颜色来自 ThemeTokens）
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** ThemedTreeWidget 内嵌 QTreeWidget 的样式参数 */
struct TreeWidgetStyleMetrics
{
    int borderRadius = 8;
    int itemPaddingV = 4;
    int fontSizePt   = 13;
};

/** 按当前主题生成树控件 QSS（背景/文字/选中/hover 均走 ThemeTokens） */
inline QString buildTreeWidgetStylesheet(const TreeWidgetStyleMetrics &m = {})
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();

    return QStringLiteral(
               "#themedTreeWidget { background: transparent; }"
               "#themedTree {"
               "  background-color: %1; color: %2;"
               "  border: 1px solid %3; border-radius: %4px;"
               "  padding: 6px; outline: none;"
               "  font-family: %5; font-size: %6px;"
               "}"
               "#themedTree::item {"
               "  padding: %7px 2px; border-radius: 4px;"
               "  background-color: transparent;"
               "}"
               "#themedTree::item:selected { color: %8; background-color: %9; }"
               "#themedTree::item:hover { background-color: %10; }"
               "#themedTree::branch { background: transparent; }")
        .arg(t.surface.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(m.borderRadius)
        .arg(fontFamily)
        .arg(m.fontSizePt)
        .arg(m.itemPaddingV)
        .arg(t.highlightText.name())
        .arg(t.accentPressed.name())
        .arg(t.iconButtonBgHover.name());
}

} // namespace ui
