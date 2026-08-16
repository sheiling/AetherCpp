/**
 * @file themed_datetime_edit.cpp
 * @brief ThemedDateTimeEdit 实现
 */

#include "controls/datetime/themed_datetime_edit.h"

#include "controls/datetime/datetime_edit_style.h"
#include "theme/theme_refresh.h"

#include <QEvent>
#include <QFocusEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QStyleFactory>
#include <QVBoxLayout>

namespace ui {

ThemedDateTimeEdit::ThemedDateTimeEdit(QWidget *parent)
    : QWidget(parent)
{
    bindThemeRefresh(this, [this]() { refreshTheme(); });

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    m_caption = new QLabel(this);
    m_caption->setVisible(false);

    m_fieldShell = new QFrame(this);
    m_fieldShell->setObjectName(QStringLiteral("themedDateTimeField"));
    m_fieldShell->setAttribute(Qt::WA_StyledBackground, true);
    m_fieldShell->setFixedHeight(m_metrics.toolbarHeight);
    QStyle *fusionStyle = QStyleFactory::create(QStringLiteral("Fusion"));
    if (fusionStyle)
        m_fieldShell->setStyle(fusionStyle);

    auto *fieldLay = new QHBoxLayout(m_fieldShell);
    fieldLay->setContentsMargins(m_metrics.fieldInset, m_metrics.fieldInset,
                                 m_metrics.fieldInset, m_metrics.fieldInset);
    fieldLay->setSpacing(0);
    fieldLay->setAlignment(Qt::AlignVCenter);

    m_edit = new QLineEdit(m_fieldShell);
    m_edit->setReadOnly(true);
    m_edit->setFocusPolicy(Qt::NoFocus);
    m_edit->setCursor(Qt::PointingHandCursor);
    m_edit->setFrame(false);
    m_edit->setFixedHeight(dateTimeInnerCtrlHeight(m_metrics));
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_edit->installEventFilter(this);
    if (fusionStyle)
        m_edit->setStyle(fusionStyle);

    m_btnPicker = new QPushButton(m_fieldShell);
    m_btnPicker->setObjectName(QStringLiteral("themedDateTimePickerBtn"));
    m_btnPicker->setFixedSize(m_metrics.pickerBtnWidth, dateTimeInnerCtrlHeight(m_metrics));
    m_btnPicker->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_btnPicker->setCursor(Qt::PointingHandCursor);
    m_btnPicker->setFocusPolicy(Qt::NoFocus);
    m_btnPicker->setText(QString());
    if (fusionStyle)
        m_btnPicker->setStyle(fusionStyle);

    fieldLay->addWidget(m_edit, 1, Qt::AlignVCenter);
    fieldLay->addWidget(m_btnPicker, 0, Qt::AlignVCenter);

    lay->addWidget(m_caption);
    lay->addWidget(m_fieldShell);

    connect(m_btnPicker, &QPushButton::clicked, this, &ThemedDateTimeEdit::openPicker);

    m_fieldShell->installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    m_texts = defaultDateTimePickerTexts();

    setResolution(m_resolution);
    refreshTheme();
    refreshDisplayText();
    setFixedWidth(preferredWidth());
}

void ThemedDateTimeEdit::setToolbarHeight(int height)
{
    m_metrics.toolbarHeight = height;
    m_fieldShell->setFixedHeight(height);
    const int inner = dateTimeInnerCtrlHeight(m_metrics);
    m_edit->setFixedHeight(inner);
    m_btnPicker->setFixedSize(m_metrics.pickerBtnWidth, inner);
    refreshTheme();
}

void ThemedDateTimeEdit::setPickerTexts(const DateTimePickerTexts &texts)
{
    m_texts = texts;
}

void ThemedDateTimeEdit::refreshTheme()
{
    if (m_caption)
        m_caption->setStyleSheet(buildDateTimeCaptionStylesheet());
    updateFieldStyle();
}

void ThemedDateTimeEdit::setResolution(DateTimeResolution resolution)
{
    m_resolution = resolution;
    m_edit->setPlaceholderText(dateTimeResolutionDisplayFormat(resolution).toUpper());
    if (m_dateTime.isValid())
        setDateTime(m_dateTime);
    setFixedWidth(preferredWidth());
    refreshDisplayText();
}

void ThemedDateTimeEdit::setNullable(bool nullable) { m_nullable = nullable; }

void ThemedDateTimeEdit::setEndOfRange(bool endOfRange) { m_endOfRange = endOfRange; }

void ThemedDateTimeEdit::setDateTime(const QDateTime &value)
{
    const QDateTime truncated =
        value.isValid() ? dateTimeTruncate(value, m_resolution) : QDateTime();
    if (truncated == m_dateTime)
        return;
    m_dateTime = truncated;
    refreshDisplayText();
    emit dateTimeChanged(m_dateTime);
}

void ThemedDateTimeEdit::clearDateTime()
{
    if (!m_nullable)
        return;
    if (!m_dateTime.isValid())
        return;
    m_dateTime = QDateTime();
    refreshDisplayText();
    emit dateTimeChanged(m_dateTime);
}

QDateTime ThemedDateTimeEdit::effectiveDateTime() const
{
    if (!m_dateTime.isValid())
        return {};
    return dateTimeFilterBound(m_dateTime, m_resolution, m_endOfRange);
}

void ThemedDateTimeEdit::setPlaceholderWhenEmpty(const QString &text)
{
    m_emptyPlaceholder = text;
    refreshDisplayText();
}

void ThemedDateTimeEdit::setCaption(const QString &caption)
{
    m_caption->setText(caption);
    m_caption->setVisible(!caption.isEmpty());
}

QString ThemedDateTimeEdit::caption() const { return m_caption->text(); }

void ThemedDateTimeEdit::openPicker()
{
    QDateTime result;
    bool      cleared = false;
    if (!ThemedDateTimePickerPopup::pickAt(m_fieldShell,
                                           m_dateTime.isValid() ? m_dateTime
                                                                : QDateTime::currentDateTime(),
                                           m_resolution, m_nullable, m_texts, &result, &cleared))
        return;

    if (cleared)
        clearDateTime();
    else
        setDateTime(result);
    emit editingFinished();
}

bool ThemedDateTimeEdit::eventFilter(QObject *watched, QEvent *event)
{
    const bool clickTarget = watched == m_edit || watched == m_fieldShell;
    if (clickTarget && event->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            openPicker();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ThemedDateTimeEdit::focusInEvent(QFocusEvent *event)
{
    m_focused = true;
    updateFieldStyle();
    QWidget::focusInEvent(event);
}

void ThemedDateTimeEdit::focusOutEvent(QFocusEvent *event)
{
    m_focused = false;
    updateFieldStyle();
    QWidget::focusOutEvent(event);
}

void ThemedDateTimeEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Down) {
        openPicker();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ThemedDateTimeEdit::refreshDisplayText()
{
    if (!m_dateTime.isValid()) {
        m_edit->clear();
        m_edit->setPlaceholderText(m_emptyPlaceholder);
        return;
    }
    m_edit->setPlaceholderText(dateTimeResolutionDisplayFormat(m_resolution).toUpper());
    m_edit->setText(dateTimeFormatDisplayText(m_dateTime, m_resolution, m_endOfRange));
    m_edit->setToolTip(QString());
}

void ThemedDateTimeEdit::updateFieldStyle()
{
    if (m_fieldShell)
        m_fieldShell->setStyleSheet(buildDateTimeFieldShellStylesheet(m_focused, m_metrics));
    paintCalendarButton();
}

void ThemedDateTimeEdit::paintCalendarButton()
{
    const ThemeTokens t = ThemeTokens::resolve();
    const int           iconW = 14;
    const int           iconH = 13;
    QPixmap             pix(iconW, iconH);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor stroke = m_focused ? t.textPrimary : t.textSecondary;
    QPen         pen(stroke, 1.2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(1, 3, iconW - 2, iconH - 4, 2, 2);
    p.drawLine(1, 6, iconW - 1, 6);
    p.setBrush(stroke);
    p.drawRect(4, 1, 2, 3);
    p.drawRect(iconW - 6, 1, 2, 3);
    p.end();
    m_btnPicker->setIcon(QIcon(pix));
    m_btnPicker->setIconSize(QSize(iconW, iconH));
}

int ThemedDateTimeEdit::preferredWidth() const
{
    switch (m_resolution) {
    case DateTimeResolution::Day:
        return 118;
    case DateTimeResolution::Hour:
        return 158;
    case DateTimeResolution::Minute:
        return 158;
    case DateTimeResolution::Second:
    default:
        return 178;
    }
}

} // namespace ui
