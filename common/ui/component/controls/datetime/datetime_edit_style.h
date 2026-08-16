#pragma once

/**
 * @file datetime_edit_style.h
 * @brief ThemedDateTimeEdit / ThemedDateTimePickerPopup 动态 QSS（颜色来自 ThemeTokens）
 */

#include "theme/theme_color_math.h"
#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 工具栏日期字段外框高度（与权限筛选栏 30px 对齐） */
struct DateTimeEditStyleMetrics
{
    int toolbarHeight   = 30;
    int borderRadius    = 5;
    int fieldInset      = 3;
    int pickerBtnWidth  = 24;
    int fontSizePx      = 12;
};

inline int dateTimeInnerCtrlHeight(const DateTimeEditStyleMetrics &m = {})
{
    return m.toolbarHeight - 2 - 2 * m.fieldInset;
}

/** 日期输入外框 + 内嵌 QLineEdit / 日历按钮 */
inline QString buildDateTimeFieldShellStylesheet(bool focused,
                                                 const DateTimeEditStyleMetrics &metrics = {})
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     fontFamily = themeFontFamilyCss();
    const QString     border     = focused ? t.focus.name() : t.inputBorder.name();
    const QString     hoverBr =
        focused ? t.focus.name() : t.inputBorderHover.name();
    const QString innerH = QString::number(dateTimeInnerCtrlHeight(metrics));
    const QString btnW   = QString::number(metrics.pickerBtnWidth);
    const QString btnBg  = themeBlend(t.inputBackground, t.surfaceBorder, 0.35).name();
    const QString btnHover =
        themeBlend(t.inputBackground, t.accent, 0.12).name();
    const QString btnPressed =
        themeBlend(t.inputBackground, t.accent, 0.22).name();

    return QStringLiteral(
        "QFrame#themedDateTimeField{"
        "background-color:%1;border:1px solid %2;border-radius:%3px;}"
        "QFrame#themedDateTimeField:hover{border:1px solid %4;}"
        "QFrame#themedDateTimeField QLineEdit{"
        "background:transparent;border:none;color:%5;"
        "padding:0;font-size:%6px;font-family:%7;"
        "height:%8px;min-height:%8px;max-height:%8px;margin:0;"
        "selection-background-color:%9;selection-color:%5;}"
        "QFrame#themedDateTimeField QLineEdit::placeholder{color:%10;}"
        "QFrame#themedDateTimeField QPushButton#themedDateTimePickerBtn{"
        "background-color:%11;border:none;border-left:1px solid %2;"
        "border-top-right-radius:%12px;border-bottom-right-radius:%12px;"
        "height:%8px;min-height:%8px;max-height:%8px;"
        "min-width:%13px;max-width:%13px;padding:0;margin:0;}"
        "QFrame#themedDateTimeField QPushButton#themedDateTimePickerBtn:hover{"
        "background-color:%14;}"
        "QFrame#themedDateTimeField QPushButton#themedDateTimePickerBtn:pressed{"
        "background-color:%15;}")
        .arg(t.inputBackground.name(),
             border,
             QString::number(metrics.borderRadius),
             hoverBr,
             t.textPrimary.name(),
             QString::number(metrics.fontSizePx),
             fontFamily,
             innerH,
             t.accent.name(),
             t.textSecondary.name(),
             btnBg,
             QString::number(qMax(0, metrics.borderRadius - 2)),
             btnW,
             btnHover,
             btnPressed);
}

inline QString buildDateTimeCaptionStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("color:%1;font-size:11px;background:transparent;")
        .arg(t.textSecondary.name());
}

inline QString buildDateTimePickerPopupStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral(
        "#themedDateTimePickerPopup{background:%1;border:1px solid %2;border-radius:8px;}"
        "#themedDateTimePickerPopup QWidget#themedDateTimePanel{background:%1;}"
        "#themedDateTimePickerPopup QScrollArea#themedDateTimeHourScroll{"
        "background:%1;border:none;}"
        "#themedDateTimePickerPopup QScrollBar:vertical{width:6px;background:transparent;margin:2px 0;}"
        "#themedDateTimePickerPopup QScrollBar::handle:vertical{"
        "background:%3;border-radius:3px;min-height:28px;}"
        "#themedDateTimePickerPopup QScrollBar::add-line:vertical,"
        "#themedDateTimePickerPopup QScrollBar::sub-line:vertical{height:0;}"
        "#themedDateTimePickerPopup QScrollBar::add-page:vertical,"
        "#themedDateTimePickerPopup QScrollBar::sub-page:vertical{background:transparent;}")
        .arg(t.surface.name(), t.inputBorder.name(), t.accent.name());
}

inline QString buildDateTimePickerPanelStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("background:%1;border-bottom:1px solid %2;")
        .arg(t.surface.name(), t.inputBorder.name());
}

inline QString buildDateTimePickerBodyStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("background:%1;").arg(t.surface.name());
}

inline QString buildDateTimePickerDividerStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("background-color:%1;").arg(t.inputBorder.name());
}

inline QString buildDateTimePickerHeaderLabelStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("color:%1;font-size:14px;font-weight:600;background:transparent;")
        .arg(t.textPrimary.name());
}

inline QString buildDateTimePickerHourCaptionStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("color:%1;font-size:12px;font-weight:500;background:transparent;")
        .arg(t.textPrimary.name());
}

inline QString buildDateTimePickerFooterStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral("background:%1;border-top:1px solid %2;")
        .arg(t.surface.name(), t.inputBorder.name());
}

inline QString buildDateTimePickerHourButtonStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    const QString hoverBg = themeBlend(t.surface, t.accent, 0.14).name();
    const QString checkedBg = themeBlend(t.surface, t.accent, 0.28).name();
    return QStringLiteral(
        "QPushButton{background:transparent;color:%1;border:none;border-radius:4px;"
        "min-height:30px;max-height:30px;font-size:13px;padding:0;margin:0;}"
        "QPushButton:hover{background:%2;color:%1;}"
        "QPushButton:checked{background:%3;color:%1;font-weight:600;"
        "border:1px solid %4;}")
        .arg(t.textPrimary.name(), hoverBg, checkedBg, t.accent.name());
}

inline QString buildDateTimePickerLinkButtonStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral(
        "QPushButton{background:transparent;color:%1;border:none;font-size:12px;"
        "padding:0 8px;min-height:24px;max-height:24px;}"
        "QPushButton:hover{color:%2;}")
        .arg(t.accent.name(), t.accentHover.name());
}

inline QString buildDateTimePickerConfirmButtonStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    return QStringLiteral(
        "QPushButton{background-color:%1;color:%2;border:none;border-radius:4px;"
        "font-size:13px;font-weight:600;padding:0 20px;min-height:30px;max-height:30px;}"
        "QPushButton:hover{background-color:%3;}")
        .arg(t.accent.name(), t.highlightText.name(), t.accentHover.name());
}

inline QString buildDateTimePickerNavButtonStylesheet()
{
    const ThemeTokens t = ThemeTokens::resolve();
    const QString hoverBg = themeBlend(t.surface, t.surfaceBorder, 0.55).name();
    return QStringLiteral(
        "QPushButton{background:%1;color:%2;font-size:14px;"
        "border:1px solid %3;border-radius:4px;padding:0;}"
        "QPushButton:hover{color:%4;background:%5;border-color:%6;}")
        .arg(t.inputBackground.name(),
             t.textSecondary.name(),
             t.inputBorder.name(),
             t.textPrimary.name(),
             hoverBg,
             t.inputBorderHover.name());
}

inline QString buildDateTimePickerCalendarStylesheet()
{
    const ThemeTokens t          = ThemeTokens::resolve();
    const QString     spinBg     = themeBlend(t.inputBackground, t.surfaceBorder, 0.4).name();
    const QString     weekendCol = t.dangerText.name();
    Q_UNUSED(weekendCol);
    return QStringLiteral(
        "QCalendarWidget{background-color:%1;color:%2;}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar{background:%1;}"
        "QCalendarWidget QToolButton{color:%2;background:transparent;}"
        "QCalendarWidget QMenu{color:%2;background:%1;}"
        "QCalendarWidget QSpinBox{color:%2;background:%3;}"
        "QCalendarWidget QAbstractItemView:enabled{"
        "color:%2;selection-background-color:%4;selection-color:%5;"
        "background-color:%1;}"
        "QCalendarWidget QAbstractItemView:disabled{color:%6;}"
        "QCalendarWidget QWidget#qt_calendar_calendarview{border:none;}")
        .arg(t.surface.name(),
             t.textPrimary.name(),
             spinBg,
             t.accent.name(),
             t.highlightText.name(),
             t.textSecondary.name());
}

} // namespace ui
