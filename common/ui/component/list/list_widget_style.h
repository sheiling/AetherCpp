#pragma once

/**
 * @file list_widget_style.h
 * @brief ThemedListWidget 动态 QSS（颜色来自 ThemeTokens）
 */

#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 列表行内容区视觉态（由 ThemedListWidget 根据选中/悬停判定） */
enum class ListRowVisualState {
    Normal,   ///< 默认透明背景
    Hovered,  ///< 半透明 accent 底色
    Selected, ///< accentPressed 实心背景
};

/** buildListWidgetStylesheet 可调尺寸 token */
struct ListWidgetStyleMetrics
{
    int borderRadius  = 6;  ///< 外框圆角 (px)
    int itemMinHeight = 36; ///< QListWidget::item min-height
    int listPadding   = 4;  ///< 列表内边距
    int itemPaddingV  = 8;  ///< 纯文本行上下 padding
    int itemPaddingH  = 12; ///< 纯文本行左右 padding
};

/** QColor → #AARRGGBB，用于半透明 hover 背景 */
inline QString colorHexArgb(const QColor &c)
{
    return c.name(QColor::HexArgb);
}

/** 纯文本 QListWidgetItem 的背景色（无 itemWidget 时使用） */
inline QColor listPlainItemBackground(ListRowVisualState state)
{
    const ThemeTokens t = ThemeTokens::resolve();
    switch (state) {
    case ListRowVisualState::Selected:
        return t.accentPressed;
    case ListRowVisualState::Hovered: {
        QColor hover = t.accent;
        hover.setAlpha(72);
        return hover;
    }
    case ListRowVisualState::Normal:
    default:
        return Qt::transparent;
    }
}

/** 纯文本 QListWidgetItem 的前景色（无 itemWidget 时使用） */
inline QColor listPlainItemForeground(ListRowVisualState state)
{
    const ThemeTokens t = ThemeTokens::resolve();
    switch (state) {
    case ListRowVisualState::Selected:
        return t.highlightText;
    case ListRowVisualState::Hovered:
        return t.textPrimary;
    case ListRowVisualState::Normal:
    default:
        return t.textPrimary;
    }
}

/** 行内容区背景（不含底部分隔线）；Selected→accentPressed，Hovered→accent@28% */
inline QString buildListRowContentStylesheet(ListRowVisualState state)
{
    const ThemeTokens t = ThemeTokens::resolve();

    switch (state) {
    case ListRowVisualState::Selected:
        return QStringLiteral(
            "background-color: %1; color: %2; border: none; margin: 0px; padding: 0px;")
            .arg(t.accentPressed.name(), t.highlightText.name());
    case ListRowVisualState::Hovered: {
        QColor hover = t.accent;
        hover.setAlpha(72);
        return QStringLiteral(
            "background-color: %1; color: %2; border: none; margin: 0px; padding: 0px;")
            .arg(colorHexArgb(hover), t.textPrimary.name());
    }
    case ListRowVisualState::Normal:
    default:
        return QStringLiteral(
            "background-color: transparent; color: %1; border: none; margin: 0px; padding: 0px;")
            .arg(t.textPrimary.name());
    }
}

/**
 * QListWidget 本体样式
 * @param itemWidgetRows true：::item 不着色（setItemWidget 行由 widget 自绘，避免与 ::item 双层）
 */
inline QString buildListWidgetStylesheet(const ListWidgetStyleMetrics &m = {},
                                         bool itemWidgetRows = false)
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();
    QColor            hoverBg    = t.accent;
    hoverBg.setAlpha(72);

    const QString itemSelectedBg =
        itemWidgetRows ? QStringLiteral("transparent") : t.accentPressed.name();
    const QString itemHoverBg =
        itemWidgetRows ? QStringLiteral("transparent") : colorHexArgb(hoverBg);
    const int itemPadV = itemWidgetRows ? 0 : m.itemPaddingV;
    const int itemPadH = itemWidgetRows ? 0 : m.itemPaddingH;
    return QStringLiteral(
        "QListWidget {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: %4px;"
        "  outline: none; padding: %5px;"
        "  font-family: %6; font-size: 12px;"
        "}"
        "QListWidget::item {"
        "  color: %2; background-color: transparent;"
        "  border: none; padding: %7px %8px; margin: 1px 0px;"
        "  min-height: %9px; border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: %10; color: %11;"
        "  border: none; border-radius: 4px;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: %12; color: %2;"
        "  border: none; border-radius: 4px;"
        "}"
        "QListWidget::item:selected:hover {"
        "  background-color: %10; color: %11;"
        "}"
        "QListWidget QWidget#recipeListRow { background: transparent; border: none; }"
        "QListWidget QWidget#recipeListRowContent { background: transparent; border: none; }"
        "QListWidget QFrame#recipeListRowSep {"
        "  background-color: %13; border: none; min-height: 1px; max-height: 1px;"
        "}"
        "QListWidget QLabel#recipeListName {"
        "  color: %2; font-size: 12px; background: transparent; border: none;"
        "}"
        "QListWidget QLabel#recipeListTag {"
        "  color: %11; font-size: 11px; font-weight: bold;"
        "  background-color: %10; border: none; border-radius: 3px;"
        "  padding: 1px 6px;"
        "}"
    ).arg(t.surface.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name())
        .arg(m.borderRadius)
        .arg(m.listPadding)
        .arg(fontFamily)
        .arg(itemPadV)
        .arg(itemPadH)
        .arg(m.itemMinHeight)
        .arg(itemSelectedBg)
        .arg(t.highlightText.name())
        .arg(itemHoverBg)
        .arg(t.divider.name());
}

} // namespace ui
