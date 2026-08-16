#pragma once

/**
 * @file themed_datetime_edit.h
 * @brief 主题日期时间输入：左侧编辑框 + 右侧日历按钮；样式与弹层来自 ThemeTokens
 */

#include "component_global.h"
#include "controls/datetime/datetime_resolution.h"
#include "controls/datetime/datetime_edit_style.h"
#include "controls/datetime/themed_datetime_picker_popup.h"
#include "component_global.h"

#include <QDateTime>
#include <QWidget>

class QFrame;
class QLineEdit;
class QPushButton;
class QLabel;

namespace ui {

class UI_COMPONENT_EXPORT ThemedDateTimeEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(DateTimeResolution resolution READ resolution WRITE setResolution)
    Q_PROPERTY(bool nullable READ nullable WRITE setNullable)
    Q_PROPERTY(bool endOfRange READ endOfRange WRITE setEndOfRange)

public:
    explicit ThemedDateTimeEdit(QWidget *parent = nullptr);

    DateTimeResolution resolution() const { return m_resolution; }
    void               setResolution(DateTimeResolution resolution);

    bool nullable() const { return m_nullable; }
    void setNullable(bool nullable);

    bool endOfRange() const { return m_endOfRange; }
    void setEndOfRange(bool endOfRange);

    QDateTime dateTime() const { return m_dateTime; }
    bool      isEmpty() const { return !m_dateTime.isValid(); }

    void setDateTime(const QDateTime &value);
    void clearDateTime();

    QDateTime effectiveDateTime() const;

    void setPlaceholderWhenEmpty(const QString &text);
    void setCaption(const QString &caption);
    QString caption() const;

    void setPickerTexts(const DateTimePickerTexts &texts);
    DateTimePickerTexts pickerTexts() const { return m_texts; }

    void setToolbarHeight(int height);

    QLineEdit *lineEdit() const { return m_edit; }

    void refreshTheme();

public slots:
    void openPicker();

signals:
    void dateTimeChanged(const QDateTime &value);
    void editingFinished();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void refreshDisplayText();
    void updateFieldStyle();
    int  preferredWidth() const;
    void paintCalendarButton();

    QLabel                 *m_caption    = nullptr;
    QFrame                 *m_fieldShell = nullptr;
    QLineEdit              *m_edit       = nullptr;
    QPushButton            *m_btnPicker  = nullptr;
    DateTimeResolution      m_resolution = DateTimeResolution::Hour;
    bool                    m_nullable   = false;
    bool                    m_endOfRange = false;
    bool                    m_focused    = false;
    QDateTime               m_dateTime;
    QString                 m_emptyPlaceholder;
    DateTimePickerTexts     m_texts;
    DateTimeEditStyleMetrics m_metrics;
};

} // namespace ui
