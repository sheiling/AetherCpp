#pragma once

/**
 * @file themed_datetime_picker_popup.h
 * @brief 主题日期时间选择弹层：左日历 + 右小时列；样式来自 ThemeTokens
 */

#include "component_global.h"
#include "controls/datetime/datetime_resolution.h"

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QWidget>

class QEvent;

class QCalendarWidget;
class QLabel;
class QPushButton;
class QScrollArea;

namespace ui {

/** 弹层文案（由业务插件在 retranslate 时注入，避免 common 依赖各插件 ts） */
struct DateTimePickerTexts
{
    QString hourLabel;
    QString hourRangeHint;
    QString hourTooltipPattern; ///< 含 %1 小时
    QString monthHeaderPattern; ///< 含 %1 年 %2 月
    QString nowLabel;
    QString clearLabel;
    QString confirmLabel;
};

/** 弹层默认文案（业务未注入时兜底，避免标题/按钮空白） */
inline DateTimePickerTexts defaultDateTimePickerTexts()
{
    DateTimePickerTexts t;
    t.hourLabel            = QStringLiteral("时");
    t.hourRangeHint        = QStringLiteral("0–23");
    t.hourTooltipPattern   = QStringLiteral("%1:00");
    t.monthHeaderPattern   = QStringLiteral("%1年%2月");
    t.nowLabel             = QStringLiteral("此刻");
    t.clearLabel           = QStringLiteral("清空");
    t.confirmLabel         = QStringLiteral("确定");
    return t;
}

/** 业务文案覆盖默认项（空字段保留默认） */
inline DateTimePickerTexts mergeDateTimePickerTexts(const DateTimePickerTexts &custom)
{
    DateTimePickerTexts merged = defaultDateTimePickerTexts();
    if (!custom.hourLabel.isEmpty())
        merged.hourLabel = custom.hourLabel;
    if (!custom.hourRangeHint.isEmpty())
        merged.hourRangeHint = custom.hourRangeHint;
    if (!custom.hourTooltipPattern.isEmpty())
        merged.hourTooltipPattern = custom.hourTooltipPattern;
    if (!custom.monthHeaderPattern.isEmpty())
        merged.monthHeaderPattern = custom.monthHeaderPattern;
    if (!custom.nowLabel.isEmpty())
        merged.nowLabel = custom.nowLabel;
    if (!custom.clearLabel.isEmpty())
        merged.clearLabel = custom.clearLabel;
    if (!custom.confirmLabel.isEmpty())
        merged.confirmLabel = custom.confirmLabel;
    return merged;
}

class UI_COMPONENT_EXPORT ThemedDateTimePickerPopup : public QWidget
{
    Q_OBJECT

public:
    explicit ThemedDateTimePickerPopup(QWidget *parent = nullptr);

    void setPickerTexts(const DateTimePickerTexts &texts);
    DateTimePickerTexts pickerTexts() const { return m_texts; }

    void setInitialDateTime(const QDateTime &value);
    void setResolution(DateTimeResolution resolution);
    void setNullable(bool nullable);

    QDateTime dateTime() const;
    bool      wasCleared() const { return m_cleared; }

    static bool pickAt(QWidget *anchor, const QDateTime &initial, DateTimeResolution resolution,
                       bool nullable, const DateTimePickerTexts &texts, QDateTime *result,
                       bool *cleared);

    void refreshTheme();

signals:
    void accepted();
    void rejected();

private slots:
    void onPrevYear();
    void onPrevMonth();
    void onNextMonth();
    void onNextYear();
    void onCalendarSelectionChanged();
    void onHourButtonClicked();
    void onNowClicked();
    void onClearClicked();
    void onConfirmClicked();

private:
    void applyTexts();
    void rebuildHourButtons();
    void updatePopupGeometry();
    void setSelectedHour(int hour);
    void syncHourButtonsToDraft();
    void syncHeaderLabel();
    void applyDraftFromUi();
    QDateTime buildDateTimeFromUi() const;
    void      scrollHourIntoView(int hour);

    bool eventFilter(QObject *watched, QEvent *event) override;

    QCalendarWidget       *m_calendar    = nullptr;
    QWidget               *m_timePanel   = nullptr;
    QScrollArea           *m_hourScroll  = nullptr;
    QWidget               *m_hourContent = nullptr;
    QVector<QPushButton *> m_hourButtons;
    QLabel                *m_headerLabel = nullptr;
    QLabel                *m_hourCaption = nullptr;
    QPushButton           *m_btnNow      = nullptr;
    QPushButton           *m_btnClear    = nullptr;
    QPushButton           *m_btnConfirm  = nullptr;
    QWidget               *m_header      = nullptr;
    QWidget               *m_footer      = nullptr;

    QDateTime           m_draft;
    int                 m_selectedHour = 0;
    DateTimeResolution  m_resolution = DateTimeResolution::Hour;
    bool                m_nullable   = false;
    bool                m_cleared    = false;
    DateTimePickerTexts m_texts;
};

} // namespace ui
