/**
 * @file themed_datetime_picker_popup.cpp
 * @brief ThemedDateTimePickerPopup 实现
 */

#include "controls/datetime/themed_datetime_picker_popup.h"

#include "controls/datetime/datetime_edit_style.h"
#include "theme/theme_refresh.h"
#include "uibase/languagemanager.h"

#include <QEvent>

#include <QButtonGroup>
#include <QCalendarWidget>
#include <QEventLoop>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QStyleFactory>
#include <QTextCharFormat>
#include <QVBoxLayout>

namespace ui {
namespace {

constexpr int kCalendarWidth   = 308;
constexpr int kCalendarHeight  = 232;
constexpr int kHourColumnWidth = 56;
constexpr int kHeaderHeight    = 40;
constexpr int kFooterHeight    = 48;

QLocale pickerLocale()
{
    LanguageManager *langMgr = LanguageManager::instance();
    return langMgr && langMgr->currentLanguage() == LanguageManager::Language::English
               ? QLocale(QLocale::English, QLocale::UnitedStates)
               : QLocale(QLocale::Chinese, QLocale::China);
}

QPushButton *makeNavButton(const QString &text, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setFlat(false);
    btn->setFixedSize(28, 28);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

} // namespace

ThemedDateTimePickerPopup::ThemedDateTimePickerPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("themedDateTimePickerPopup"));

    bindThemeRefresh(this, [this]() { refreshTheme(); });

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_header = new QWidget(this);
    m_header->setFixedHeight(kHeaderHeight);
    auto *headerLay = new QHBoxLayout(m_header);
    headerLay->setContentsMargins(8, 4, 8, 4);
    headerLay->setSpacing(2);

    auto *btnPrevYear  = makeNavButton(QStringLiteral("«"), m_header);
    auto *btnPrevMonth = makeNavButton(QStringLiteral("‹"), m_header);
    m_headerLabel      = new QLabel(m_header);
    m_headerLabel->setAlignment(Qt::AlignCenter);
    auto *btnNextMonth = makeNavButton(QStringLiteral("›"), m_header);
    auto *btnNextYear  = makeNavButton(QStringLiteral("»"), m_header);

    headerLay->addWidget(btnPrevYear);
    headerLay->addWidget(btnPrevMonth);
    headerLay->addWidget(m_headerLabel, 1);
    headerLay->addWidget(btnNextMonth);
    headerLay->addWidget(btnNextYear);

    auto *body = new QWidget(this);
    auto *bodyLay = new QHBoxLayout(body);
    bodyLay->setContentsMargins(4, 4, 4, 4);
    bodyLay->setSpacing(0);

    m_calendar = new QCalendarWidget(body);
    m_calendar->setLocale(pickerLocale());
    m_calendar->setFirstDayOfWeek(Qt::Monday);
    m_calendar->setNavigationBarVisible(false);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        m_calendar->setStyle(fusion);
    m_calendar->setFixedSize(kCalendarWidth, kCalendarHeight);

    m_timePanel = new QWidget(body);
    m_timePanel->setObjectName(QStringLiteral("themedDateTimePanel"));
    m_timePanel->setFixedWidth(kHourColumnWidth + 8);
    auto *timeOuter = new QVBoxLayout(m_timePanel);
    timeOuter->setContentsMargins(4, 0, 4, 0);
    timeOuter->setSpacing(4);

    m_hourCaption = new QLabel(m_timePanel);
    m_hourCaption->setAlignment(Qt::AlignCenter);

    m_hourScroll = new QScrollArea(m_timePanel);
    m_hourScroll->setObjectName(QStringLiteral("themedDateTimeHourScroll"));
    m_hourScroll->setWidgetResizable(true);
    m_hourScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_hourScroll->setFrameShape(QFrame::NoFrame);
    m_hourScroll->setFixedHeight(kCalendarHeight - 22);

    m_hourContent = new QWidget(m_hourScroll);
    m_hourContent->setMinimumWidth(kHourColumnWidth - 8);
    auto *hourLay = new QVBoxLayout(m_hourContent);
    hourLay->setContentsMargins(0, 0, 0, 0);
    hourLay->setSpacing(0);
    m_hourScroll->setWidget(m_hourContent);

    timeOuter->addWidget(m_hourCaption);
    timeOuter->addWidget(m_hourScroll, 1);

    auto *divider = new QFrame(body);
    divider->setFrameShape(QFrame::VLine);
    divider->setFixedWidth(1);

    bodyLay->addWidget(m_calendar, 0, Qt::AlignTop);
    bodyLay->addWidget(divider, 0, Qt::AlignTop);
    bodyLay->addWidget(m_timePanel, 0, Qt::AlignTop);

    m_footer = new QWidget(this);
    m_footer->setFixedHeight(kFooterHeight);
    auto *footerLay = new QHBoxLayout(m_footer);
    footerLay->setContentsMargins(10, 8, 10, 8);
    footerLay->setSpacing(8);

    m_btnNow = new QPushButton(m_footer);
    m_btnNow->setObjectName(QStringLiteral("themedDateTimeBtnLink"));
    m_btnNow->setCursor(Qt::PointingHandCursor);

    m_btnClear = new QPushButton(m_footer);
    m_btnClear->setObjectName(QStringLiteral("themedDateTimeBtnLink"));
    m_btnClear->setCursor(Qt::PointingHandCursor);
    m_btnClear->setVisible(false);

    m_btnConfirm = new QPushButton(m_footer);
    m_btnConfirm->setObjectName(QStringLiteral("themedDateTimeConfirmBtn"));
    m_btnConfirm->setCursor(Qt::PointingHandCursor);
    m_btnConfirm->setDefault(true);
    m_btnConfirm->setAutoDefault(true);

    footerLay->addWidget(m_btnNow);
    footerLay->addWidget(m_btnClear);
    footerLay->addStretch();
    footerLay->addWidget(m_btnConfirm);

    root->addWidget(m_header);
    root->addWidget(body);
    root->addWidget(m_footer);

    connect(btnPrevYear, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onPrevYear);
    connect(btnPrevMonth, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onPrevMonth);
    connect(btnNextMonth, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onNextMonth);
    connect(btnNextYear, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onNextYear);
    connect(m_calendar, &QCalendarWidget::selectionChanged, this,
            &ThemedDateTimePickerPopup::onCalendarSelectionChanged);
    connect(m_btnNow, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onNowClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onClearClicked);
    connect(m_btnConfirm, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onConfirmClicked);

    auto *shortcutConfirm = new QShortcut(QKeySequence(Qt::Key_Return), this);
    shortcutConfirm->setContext(Qt::WindowShortcut);
    connect(shortcutConfirm, &QShortcut::activated, this, &ThemedDateTimePickerPopup::onConfirmClicked);
    auto *shortcutConfirm2 = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    shortcutConfirm2->setContext(Qt::WindowShortcut);
    connect(shortcutConfirm2, &QShortcut::activated, this, &ThemedDateTimePickerPopup::onConfirmClicked);
    auto *shortcutCancel = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    shortcutCancel->setContext(Qt::WindowShortcut);
    connect(shortcutCancel, &QShortcut::activated, this, &QWidget::close);

    m_texts = defaultDateTimePickerTexts();
    rebuildHourButtons();
    refreshTheme();
    applyTexts();
    updatePopupGeometry();
}

void ThemedDateTimePickerPopup::setPickerTexts(const DateTimePickerTexts &texts)
{
    m_texts = mergeDateTimePickerTexts(texts);
    applyTexts();
}

void ThemedDateTimePickerPopup::refreshTheme()
{
    setStyleSheet(buildDateTimePickerPopupStylesheet());
    if (m_header)
        m_header->setStyleSheet(buildDateTimePickerPanelStylesheet());
    if (m_footer)
        m_footer->setStyleSheet(buildDateTimePickerFooterStylesheet());
    if (m_headerLabel)
        m_headerLabel->setStyleSheet(buildDateTimePickerHeaderLabelStylesheet());
    if (m_hourCaption)
        m_hourCaption->setStyleSheet(buildDateTimePickerHourCaptionStylesheet());
    if (m_calendar)
        m_calendar->setStyleSheet(buildDateTimePickerCalendarStylesheet());
    if (m_btnNow)
        m_btnNow->setStyleSheet(buildDateTimePickerLinkButtonStylesheet());
    if (m_btnClear)
        m_btnClear->setStyleSheet(buildDateTimePickerLinkButtonStylesheet());
    if (m_btnConfirm)
        m_btnConfirm->setStyleSheet(buildDateTimePickerConfirmButtonStylesheet());

    const ThemeTokens t = ThemeTokens::resolve();
    if (m_calendar) {
        QTextCharFormat weekendFmt;
        weekendFmt.setForeground(t.dangerText);
        m_calendar->setWeekdayTextFormat(Qt::Saturday, weekendFmt);
        m_calendar->setWeekdayTextFormat(Qt::Sunday, weekendFmt);
    }

    for (QPushButton *btn : m_hourButtons) {
        if (btn)
            btn->setStyleSheet(buildDateTimePickerHourButtonStylesheet());
    }

    for (QPushButton *navBtn : findChildren<QPushButton *>()) {
        if (!navBtn || navBtn == m_btnNow || navBtn == m_btnClear || navBtn == m_btnConfirm)
            continue;
        if (navBtn->width() == 28 && navBtn->height() == 28)
            navBtn->setStyleSheet(buildDateTimePickerNavButtonStylesheet());
    }
}

void ThemedDateTimePickerPopup::applyTexts()
{
    if (m_calendar)
        m_calendar->setLocale(pickerLocale());
    if (m_hourCaption) {
        m_hourCaption->setText(m_texts.hourLabel);
        m_hourCaption->setToolTip(m_texts.hourRangeHint);
    }
    if (m_btnNow)
        m_btnNow->setText(m_texts.nowLabel);
    if (m_btnClear)
        m_btnClear->setText(m_texts.clearLabel);
    if (m_btnConfirm)
        m_btnConfirm->setText(m_texts.confirmLabel);
    syncHeaderLabel();

    for (int h = 0; h < m_hourButtons.size(); ++h) {
        if (QPushButton *btn = m_hourButtons.at(h)) {
            const QString hh = QStringLiteral("%1").arg(h, 2, 10, QLatin1Char('0'));
            btn->setToolTip(m_texts.hourTooltipPattern.arg(hh));
        }
    }
}

bool ThemedDateTimePickerPopup::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick
        && m_hourButtons.contains(qobject_cast<QPushButton *>(watched))) {
        onHourButtonClicked();
        onConfirmClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void ThemedDateTimePickerPopup::rebuildHourButtons()
{
    if (!m_hourContent)
        return;

    for (auto *btn : m_hourButtons) {
        if (btn)
            btn->deleteLater();
    }
    m_hourButtons.clear();

    auto *hourLay = qobject_cast<QVBoxLayout *>(m_hourContent->layout());
    if (!hourLay)
        return;

    while (QLayoutItem *item = hourLay->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    auto *group = new QButtonGroup(m_hourContent);
    group->setExclusive(true);

    for (int h = 0; h <= 23; ++h) {
        auto *btn = new QPushButton(QStringLiteral("%1").arg(h, 2, 10, QLatin1Char('0')),
                                    m_hourContent);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(buildDateTimePickerHourButtonStylesheet());
        const QString hh = QStringLiteral("%1").arg(h, 2, 10, QLatin1Char('0'));
        btn->setToolTip(m_texts.hourTooltipPattern.arg(hh));
        group->addButton(btn, h);
        hourLay->addWidget(btn);
        m_hourButtons.append(btn);
        connect(btn, &QPushButton::clicked, this, &ThemedDateTimePickerPopup::onHourButtonClicked);
        btn->installEventFilter(this);
    }
    hourLay->addStretch();

    setSelectedHour(m_draft.isValid() ? m_draft.time().hour() : QTime::currentTime().hour());
}

void ThemedDateTimePickerPopup::setSelectedHour(int hour)
{
    m_selectedHour = qBound(0, hour, 23);
    for (int i = 0; i < m_hourButtons.size(); ++i) {
        if (m_hourButtons.at(i))
            m_hourButtons.at(i)->setChecked(i == m_selectedHour);
    }
    scrollHourIntoView(m_selectedHour);
}

void ThemedDateTimePickerPopup::updatePopupGeometry()
{
    const bool showTime = m_resolution != DateTimeResolution::Day;
    m_timePanel->setVisible(showTime);

    const int timePanelW = showTime ? kHourColumnWidth + 8 : 0;
    m_timePanel->setFixedWidth(timePanelW);

    const int bodyW = kCalendarWidth + (showTime ? 1 + timePanelW + 8 : 0);
    setFixedSize(bodyW + 8, kHeaderHeight + kCalendarHeight + 8 + kFooterHeight);
}

void ThemedDateTimePickerPopup::setInitialDateTime(const QDateTime &value)
{
    m_draft   = value.isValid() ? value : QDateTime::currentDateTime();
    m_cleared = false;
    m_calendar->setSelectedDate(m_draft.date());
    m_calendar->setCurrentPage(m_draft.date().year(), m_draft.date().month());
    syncHeaderLabel();
    setSelectedHour(m_draft.time().hour());
    updatePopupGeometry();
}

void ThemedDateTimePickerPopup::setResolution(DateTimeResolution resolution)
{
    m_resolution = resolution;
    updatePopupGeometry();
}

void ThemedDateTimePickerPopup::setNullable(bool nullable)
{
    m_nullable = nullable;
    if (m_btnClear)
        m_btnClear->setVisible(nullable);
}

QDateTime ThemedDateTimePickerPopup::dateTime() const
{
    return m_cleared ? QDateTime() : buildDateTimeFromUi();
}

void ThemedDateTimePickerPopup::syncHourButtonsToDraft()
{
    setSelectedHour(m_draft.time().hour());
}

void ThemedDateTimePickerPopup::syncHeaderLabel()
{
    if (!m_headerLabel || !m_calendar)
        return;
    const QDate d = m_calendar->selectedDate();
    m_headerLabel->setText(m_texts.monthHeaderPattern.arg(d.year()).arg(d.month()));
}

void ThemedDateTimePickerPopup::applyDraftFromUi()
{
    m_draft = buildDateTimeFromUi();
}

QDateTime ThemedDateTimePickerPopup::buildDateTimeFromUi() const
{
    QDate d = m_calendar->selectedDate();
    if (!d.isValid())
        d = QDate::currentDate();

    const int hour = m_resolution == DateTimeResolution::Day ? 0 : m_selectedHour;
    return dateTimeTruncate(QDateTime(d, QTime(hour, 0, 0)), DateTimeResolution::Hour);
}

void ThemedDateTimePickerPopup::scrollHourIntoView(int hour)
{
    if (hour < 0 || hour >= m_hourButtons.size())
        return;
    if (QPushButton *btn = m_hourButtons.at(hour))
        m_hourScroll->ensureWidgetVisible(btn, 0, 40);
}

void ThemedDateTimePickerPopup::onPrevYear()
{
    QDate d = m_calendar->selectedDate().addYears(-1);
    m_calendar->setCurrentPage(d.year(), d.month());
    m_calendar->setSelectedDate(d);
    syncHeaderLabel();
}

void ThemedDateTimePickerPopup::onPrevMonth()
{
    QDate d = m_calendar->selectedDate().addMonths(-1);
    m_calendar->setCurrentPage(d.year(), d.month());
    m_calendar->setSelectedDate(d);
    syncHeaderLabel();
}

void ThemedDateTimePickerPopup::onNextMonth()
{
    QDate d = m_calendar->selectedDate().addMonths(1);
    m_calendar->setCurrentPage(d.year(), d.month());
    m_calendar->setSelectedDate(d);
    syncHeaderLabel();
}

void ThemedDateTimePickerPopup::onNextYear()
{
    QDate d = m_calendar->selectedDate().addYears(1);
    m_calendar->setCurrentPage(d.year(), d.month());
    m_calendar->setSelectedDate(d);
    syncHeaderLabel();
}

void ThemedDateTimePickerPopup::onCalendarSelectionChanged()
{
    syncHeaderLabel();
    applyDraftFromUi();
}

void ThemedDateTimePickerPopup::onHourButtonClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    const int hour = m_hourButtons.indexOf(btn);
    if (hour >= 0)
        m_selectedHour = hour;
    applyDraftFromUi();
}

void ThemedDateTimePickerPopup::onNowClicked()
{
    setInitialDateTime(QDateTime::currentDateTime());
}

void ThemedDateTimePickerPopup::onClearClicked()
{
    m_cleared = true;
    emit accepted();
    close();
}

void ThemedDateTimePickerPopup::onConfirmClicked()
{
    m_cleared = false;
    applyDraftFromUi();
    emit accepted();
    close();
}

bool ThemedDateTimePickerPopup::pickAt(QWidget *anchor, const QDateTime &initial,
                                       DateTimeResolution resolution, bool nullable,
                                       const DateTimePickerTexts &texts, QDateTime *result,
                                       bool *cleared)
{
    if (!anchor || !result)
        return false;

    auto *popup = new ThemedDateTimePickerPopup(nullptr);
    popup->setPickerTexts(texts);
    popup->setResolution(resolution);
    popup->setNullable(nullable);
    popup->setInitialDateTime(initial);

    bool      accepted   = false;
    bool      clearFlag  = false;
    QDateTime pickedValue;

    QObject::connect(popup, &ThemedDateTimePickerPopup::accepted, popup, [&]() {
        accepted    = true;
        clearFlag   = popup->wasCleared();
        pickedValue = clearFlag ? QDateTime() : popup->dateTime();
    });

    const QPoint pos = anchor->mapToGlobal(QPoint(0, anchor->height()));
    popup->move(pos.x(), pos.y() + 4);
    popup->show();
    popup->raise();
    popup->activateWindow();
    if (auto *okBtn = popup->findChild<QPushButton *>(QStringLiteral("themedDateTimeConfirmBtn")))
        okBtn->setFocus();

    QEventLoop loop;
    QObject::connect(popup, &QObject::destroyed, &loop, &QEventLoop::quit);
    loop.exec();

    if (!accepted)
        return false;

    if (cleared)
        *cleared = clearFlag;
    *result = pickedValue;
    return true;
}

} // namespace ui
