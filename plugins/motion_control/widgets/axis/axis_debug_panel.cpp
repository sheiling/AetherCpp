/**
 * @file axis_debug_panel.cpp
 * @brief 轴调试面板：8 路状态灯 + 位置显示 + 运动/使能按钮
 *
 * 内容区三行：① 指示灯 | 竖分割线 | 命令/反馈位置  ② 目标位置 + 速度比例  ③ 操作按钮。
 * 点动按钮使用 pressed/released 配对 startJog/stopJog；绝对/相对移动使用 m_targetPos。
 */

#include "axis_debug_panel.h"

#include "motion_control_types.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/controls/radio/themed_radio_button.h"
#include "component/controls/indicator/themed_glass_indicator_dot.h"
#include "component/feedback/cursor_toast.h"
#include "component/group/themed_section_panel.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"
#include "uibase/thememanager.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <functional>

namespace motion_control_plugin {

namespace {

using namespace ui;

constexpr int kIndGridCols           = 4;   ///< 指示灯 4 列 × 2 行
constexpr int kIndCellWidth          = 76;
constexpr int kIndCellMinHeight      = 52;
constexpr int kContentPadding        = 12;
constexpr int kSectionSpacing        = 10;
constexpr int kPosEditMinWidth       = 120;
constexpr int kControlHeight         = 32;
constexpr int kStatusSectionMinHeight  = 112;
constexpr int kTargetSpeedRowMinHeight = 40;
constexpr int kBtnMinHeight          = 32;

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

QString formatAxisPosition(double value, int unitType)
{
    const char *fmtKey = "motion.axis.pos_mm";
    switch (unitType) {
    case 0:
        fmtKey = "motion.axis.pos_pulse";
        break;
    case 2:
        fmtKey = "motion.axis.pos_um";
        break;
    case 3:
        fmtKey = "motion.axis.pos_deg";
        break;
    default:
        break;
    }
    const int decimals = (unitType == 0) ? 0 : 3;
    return trKey(fmtKey).arg(QString::number(value, 'f', decimals));
}

QWidget *makeIndicatorCell(const QString &dotKey, const char *captionKey, QWidget *parent,
                           QHash<QString, ThemedGlassIndicatorDot *> &dots)
{
    auto *cell = new QWidget(parent);
    cell->setFixedWidth(kIndCellWidth);
    cell->setMinimumHeight(kIndCellMinHeight);
    auto *vl   = new QVBoxLayout(cell);
    vl->setContentsMargins(0, 2, 0, 2);
    vl->setSpacing(6);
    vl->setAlignment(Qt::AlignHCenter);

    auto *dot = new ThemedGlassIndicatorDot(cell);
    dot->setDiameter(12);
    dots.insert(dotKey, dot);
    vl->addWidget(dot, 0, Qt::AlignHCenter);

    auto *cap = new ThemedFormLabel(trKey(captionKey), cell);
    cap->setProperty("trKey", QString::fromLatin1(captionKey));
    cap->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    cap->setWordWrap(true);
    cap->setPixelSize(10);
    vl->addWidget(cap, 0, Qt::AlignHCenter);
    return cell;
}

ThemedPushButton *makeActionButton(const char *textKey, QWidget *parent)
{
    auto *btn = makePushButton(trKey(textKey), PushButtonVariant::Secondary, parent);
    btn->setProperty("trKey", QString::fromLatin1(textKey));
    btn->setMinimumHeight(kBtnMinHeight);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return btn;
}

ThemedLineEdit *makeReadOnlyPosEdit(QWidget *parent)
{
    auto *edit = makeLineEdit(parent);
    edit->setReadOnly(true);
    edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    edit->setMinimumWidth(kPosEditMinWidth);
    edit->setFixedHeight(kControlHeight);
    return edit;
}

ThemedFormLabel *makeFieldLabel(const char *captionKey, QWidget *parent)
{
    auto *cap = new ThemedFormLabel(trKey(captionKey), parent);
    cap->setProperty("trKey", QString::fromLatin1(captionKey));
    cap->setPixelSize(11);
    cap->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    return cap;
}

void addPosFieldRow(QVBoxLayout *vl, const char *captionKey, ThemedLineEdit **outEdit, QWidget *parent)
{
    auto *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);
    row->addWidget(makeFieldLabel(captionKey, parent), 0, Qt::AlignVCenter);
    *outEdit = makeReadOnlyPosEdit(parent);
    (*outEdit)->setText(formatAxisPosition(0.0, 1));
    row->addWidget(*outEdit, 1);
    vl->addLayout(row);
}

QFrame *makeVerticalDivider(QWidget *parent)
{
    auto *divider = new QFrame(parent);
    divider->setObjectName(QStringLiteral("motionAxisDebugDivider"));
    divider->setFrameShape(QFrame::VLine);
    divider->setFrameShadow(QFrame::Plain);
    divider->setFixedWidth(1);
    divider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    return divider;
}

/** @brief 与 UI 展示相关的运行时字段是否一致（位置用微小阈值） */
bool sameDisplayState(const AxisRuntimeState &a, const AxisRuntimeState &b)
{
    return a.enabled == b.enabled && a.alarm == b.alarm && a.inPosition == b.inPosition
           && a.origin == b.origin && a.hardPositiveLimit == b.hardPositiveLimit
           && a.hardNegativeLimit == b.hardNegativeLimit && a.softPositiveLimit == b.softPositiveLimit
           && a.softNegativeLimit == b.softNegativeLimit
           && qAbs(a.cmdPos - b.cmdPos) < 1e-6 && qAbs(a.fbPos - b.fbPos) < 1e-6;
}

} // namespace

AxisDebugPanel::AxisDebugPanel(IMotionControlService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    setObjectName(QStringLiteral("axisDebugPanel"));
    buildUi();
    m_homeToast = new CursorToast(this);
    m_homeToast->hide();
    retranslateUi();
    refreshContentStyle();
    bindService();

    bindThemeRefresh(this, [this]() { refreshContentStyle(); });
    connect(TM, &ThemeManager::themeChanged, this, [this]() { refreshContentStyle(); });
}

void AxisDebugPanel::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_panel = new ui::ThemedSectionPanel(QStringLiteral("motion.axis.debug_title"), this);
    m_panel->setContentMargins(12, 8, 12, 10);
    m_panel->setContentSpacing(0);
    root->addWidget(m_panel);

    m_contentBg = new QFrame(m_panel->contentHost());
    m_contentBg->setObjectName(QStringLiteral("motionAxisDebugContentBg"));
    m_contentBg->setAttribute(Qt::WA_StyledBackground, true);

    auto *mainLay = new QVBoxLayout(m_contentBg);
    mainLay->setContentsMargins(kContentPadding, kContentPadding, kContentPadding, kContentPadding);
    mainLay->setSpacing(kSectionSpacing);

    m_axisNameLbl = new ui::ThemedFormLabel(QStringLiteral("—"), m_contentBg);
    m_axisNameLbl->setPixelSize(12);
    mainLay->addWidget(m_axisNameLbl);

    // 第一行：指示灯 | 竖分割线 | 命令/反馈位置（垂直）
    auto *statusRow = new QHBoxLayout();
    statusRow->setContentsMargins(0, 0, 0, 0);
    statusRow->setSpacing(12);
    statusRow->addWidget(buildIndicatorSection(), 0, Qt::AlignTop);
    statusRow->addWidget(makeVerticalDivider(m_contentBg), 0, Qt::AlignTop);
    statusRow->addWidget(buildPosFeedbackSection(), 1, Qt::AlignVCenter);
    mainLay->addLayout(statusRow);

    mainLay->addWidget(buildTargetSpeedSection());
    mainLay->addWidget(buildButtonSection());

    m_panel->contentLayout()->addWidget(m_contentBg);
    ensureThemedSpinStyles(m_contentBg);
}

QWidget *AxisDebugPanel::buildIndicatorSection()
{
    auto *host = new QWidget(m_contentBg);
    host->setMinimumHeight(kStatusSectionMinHeight);
    auto *indGrid = new QGridLayout(host);
    indGrid->setContentsMargins(0, 4, 0, 4);
    indGrid->setHorizontalSpacing(4);
    indGrid->setVerticalSpacing(10);

    const QList<QPair<QString, const char *>> indicators = {
        {QStringLiteral("alarm"), "motion.ind.alarm"},
        {QStringLiteral("elp"), "motion.ind.elp"},
        {QStringLiteral("eln"), "motion.ind.eln"},
        {QStringLiteral("org"), "motion.ind.org"},
        {QStringLiteral("slp"), "motion.ind.slp"},
        {QStringLiteral("sln"), "motion.ind.sln"},
        {QStringLiteral("inp"), "motion.ind.inp"},
        {QStringLiteral("en"), "motion.ind.en"},
    };
    for (int i = 0; i < indicators.size(); ++i) {
        const int row = i / kIndGridCols;
        const int col = i % kIndGridCols;
        indGrid->addWidget(makeIndicatorCell(indicators[i].first, indicators[i].second, host,
                                             m_indicatorDots),
                           row, col, Qt::AlignHCenter);
    }
    return host;
}

QWidget *AxisDebugPanel::buildPosFeedbackSection()
{
    auto *host = new QWidget(m_contentBg);
    host->setMinimumHeight(kStatusSectionMinHeight);
    auto *vl = new QVBoxLayout(host);
    vl->setContentsMargins(0, 8, 0, 8);
    vl->setSpacing(12);
    addPosFieldRow(vl, "motion.axis.cmd_pos", &m_cmdPosEdit, host);
    addPosFieldRow(vl, "motion.axis.fb_pos", &m_fbPosEdit, host);
    return host;
}

QWidget *AxisDebugPanel::buildTargetSpeedSection()
{
    auto *host = new QWidget(m_contentBg);
    host->setMinimumHeight(kTargetSpeedRowMinHeight);
    auto *row = new QHBoxLayout(host);
    row->setContentsMargins(0, 6, 0, 6);
    row->setSpacing(8);

    row->addWidget(makeFieldLabel("motion.axis.target_pos", host));
    m_targetPos = makeDoubleSpinBox(host, QStringLiteral("axisDebugTargetPos"));
    m_targetPos->setDecimals(3);
    m_targetPos->setRange(-1e6, 1e6);
    m_targetPos->setMinimumWidth(kPosEditMinWidth);
    m_targetPos->setFixedHeight(kControlHeight);
    row->addWidget(m_targetPos);

    row->addSpacing(12);
    row->addWidget(makeFieldLabel("motion.axis.speed_ratio", host));

    m_stepGroup = new QButtonGroup(host);
    const QList<double> ratios = {0.01, 0.1, 0.5};
    for (double ratio : ratios) {
        auto *rb = makeRadioButton(QString::number(ratio), host);
        rb->setProperty("step", ratio);
        m_stepGroup->addButton(rb);
        row->addWidget(rb);
        if (qAbs(ratio - 0.1) < 1e-9)
            rb->setChecked(true);
    }
    row->addStretch();
    return host;
}

QWidget *AxisDebugPanel::buildButtonSection()
{
    auto *host = new QWidget(m_contentBg);
    auto *btnGrid = new QGridLayout(host);
    btnGrid->setContentsMargins(0, 0, 0, 0);
    btnGrid->setHorizontalSpacing(8);
    btnGrid->setVerticalSpacing(8);

    auto addBtn = [this](const char *textKey, const std::function<void()> &handler) {
        auto *b = makeActionButton(textKey, m_contentBg);
        connect(b, &QPushButton::clicked, this, [handler]() { handler(); });
        return b;
    };

    m_enableBtn = makeActionButton("motion.axis.btn.enable", m_contentBg);
    m_enableBtn->setCheckable(true);
    connect(m_enableBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (!m_service || m_axisName.isEmpty())
            return;
        m_service->setAxisEnabled(m_axisName, checked);
    });

    m_clearAlarmBtn = makeActionButton("motion.axis.btn.clear_alarm", m_contentBg);
    connect(m_clearAlarmBtn, &QPushButton::clicked, this, [this]() {
        if (m_service && !m_axisName.isEmpty())
            m_service->clearAxisAlarm(m_axisName);
    });

    const QList<ThemedPushButton *> row1Btns = {
        m_enableBtn,
        addBtn("motion.axis.btn.home", [this]() {
            if (!m_service || m_axisName.isEmpty())
                return;
            m_pendingHomeAxis = m_axisName;
            if (m_service->homeAxis(m_axisName, true))
                return;
            m_pendingHomeAxis.clear();
            const char *key = m_service->isReady() ? "motion.axis.toast.home_busy"
                                                     : "motion.axis.toast.home_unavailable";
            showHomeResultToast(m_axisName, false, trKey(key));
        }),
        m_clearAlarmBtn,
        addBtn("motion.axis.btn.stop", [this]() {
            if (m_service && !m_axisName.isEmpty())
                m_service->stopJog(m_axisName);
        }),
    };
    for (int c = 0; c < row1Btns.size(); ++c)
        btnGrid->addWidget(row1Btns[c], 0, c);

    auto *jogNeg = makeActionButton("motion.axis.btn.jog_neg", m_contentBg);
    auto *jogPos = makeActionButton("motion.axis.btn.jog_pos", m_contentBg);
    connect(jogNeg, &QAbstractButton::pressed, this, [this]() {
        if (m_service && !m_axisName.isEmpty())
            m_service->startJog(m_axisName, 0, stepSize());
    });
    connect(jogNeg, &QAbstractButton::released, this, [this]() {
        if (m_service && !m_axisName.isEmpty())
            m_service->stopJog(m_axisName);
    });
    connect(jogPos, &QAbstractButton::pressed, this, [this]() {
        if (m_service && !m_axisName.isEmpty())
            m_service->startJog(m_axisName, 1, stepSize());
    });
    connect(jogPos, &QAbstractButton::released, this, [this]() {
        if (m_service && !m_axisName.isEmpty())
            m_service->stopJog(m_axisName);
    });

    btnGrid->addWidget(
        addBtn("motion.axis.btn.move_abs", [this]() {
            if (m_service && !m_axisName.isEmpty())
                m_service->moveToPosition(m_axisName, m_targetPos->value(), true);
        }),
        1, 0);
    btnGrid->addWidget(
        addBtn("motion.axis.btn.move_rel", [this]() {
            if (m_service && !m_axisName.isEmpty())
                m_service->moveToPosition(m_axisName, m_targetPos->value(), false);
        }),
        1, 1);
    btnGrid->addWidget(jogNeg, 1, 2);
    btnGrid->addWidget(jogPos, 1, 3);

    for (int c = 0; c < 4; ++c)
        btnGrid->setColumnStretch(c, 1);

    return host;
}

void AxisDebugPanel::refreshContentStyle()
{
    if (!m_contentBg)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    m_contentBg->setStyleSheet(QStringLiteral(
        "QFrame#motionAxisDebugContentBg {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 4px;"
        "}"
        "QFrame#motionAxisDebugDivider {"
        "  background-color: %2;"
        "  border: none;"
        "  max-width: 1px;"
        "}")
                                   .arg(t.background.name(), t.inputBorder.name()));
    if (m_homeToast)
        m_homeToast->applyToastStyle();
}

void AxisDebugPanel::bindService()
{
    if (!m_service)
        return;

    // 状态刷新由父页面统一调度（MotionAxisDebugView::refreshDebugPanels /
    // MotionAxisSettingsView 连接 axisStatesUpdated），避免多面板重复刷新。
    connect(m_service, &IMotionControlService::homeAxisFinished, this,
            [this](const QString &axisName, bool success, const QString &errorMessage) {
                if (axisName != m_pendingHomeAxis)
                    return;
                m_pendingHomeAxis.clear();
                showHomeResultToast(axisName, success, errorMessage);
            });
}

void AxisDebugPanel::retranslateUi()
{
    for (auto *lbl : findChildren<ThemedFormLabel *>()) {
        const QVariant key = lbl->property("trKey");
        if (key.isValid() && !key.toString().isEmpty())
            lbl->setText(trKey(key.toString().toLatin1().constData()));
    }
    for (auto *btn : findChildren<ThemedPushButton *>()) {
        const QVariant key = btn->property("trKey");
        if (key.isValid() && !key.toString().isEmpty())
            btn->setText(trKey(key.toString().toLatin1().constData()));
    }
    if (m_axisNameLbl)
        m_axisNameLbl->setText(m_axisName.isEmpty() ? QStringLiteral("—") : m_axisName);
    refreshState();
}

void AxisDebugPanel::showHomeResultToast(const QString &axisName, bool success,
                                         const QString &errorMessage)
{
    if (!m_homeToast)
        return;

    QString text;
    if (success) {
        text = trKey("motion.axis.toast.home_ok").arg(axisName);
    } else {
        const QString reason = errorMessage.isEmpty() ? trKey("motion.axis.toast.home_unknown")
                                                      : errorMessage;
        text = trKey("motion.axis.toast.home_fail").arg(axisName, reason);
    }
    m_homeToast->showMessage(text);
}

void AxisDebugPanel::setAxisName(const QString &axisName)
{
    m_axisName = axisName;
    m_hasLastState = false;
    if (m_service && !axisName.isEmpty())
        m_unitType = m_service->axisConfig(axisName).unitType;
    else
        m_unitType = 1;
    if (m_axisNameLbl)
        m_axisNameLbl->setText(axisName.isEmpty() ? QStringLiteral("—") : axisName);
    refreshState();
}

void AxisDebugPanel::setIndicator(ui::ThemedGlassIndicatorDot *dot, bool active, bool alarmTone,
                                  const ThemeTokens &t)
{
    if (!dot)
        return;

    if (!active) {
        dot->setAccentColor(t.textSecondary);
        return;
    }
    dot->setAccentColor(alarmTone ? t.dangerText : t.successText);
}

void AxisDebugPanel::syncEnableButton(bool enabled)
{
    if (!m_enableBtn)
        return;
    const QSignalBlocker blocker(m_enableBtn);
    m_enableBtn->setChecked(enabled);
}

void AxisDebugPanel::syncClearAlarmButton(bool alarm)
{
    if (!m_clearAlarmBtn)
        return;
    m_clearAlarmBtn->setVariant(alarm ? PushButtonVariant::Danger : PushButtonVariant::Secondary);
}

void AxisDebugPanel::updateIndicators(const AxisRuntimeState &state)
{
    if (m_hasLastState && sameDisplayState(m_lastRuntimeState, state))
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    setIndicator(m_indicatorDots.value(QStringLiteral("alarm")), state.alarm, true, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("elp")), state.hardPositiveLimit, true, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("eln")), state.hardNegativeLimit, true, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("org")), state.origin, false, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("slp")), state.softPositiveLimit, true, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("sln")), state.softNegativeLimit, true, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("inp")), state.inPosition, false, t);
    setIndicator(m_indicatorDots.value(QStringLiteral("en")), state.enabled, false, t);

    if (!m_hasLastState || m_lastRuntimeState.enabled != state.enabled)
        syncEnableButton(state.enabled);
    if (!m_hasLastState || m_lastRuntimeState.alarm != state.alarm)
        syncClearAlarmButton(state.alarm);

    if (m_cmdPosEdit)
        m_cmdPosEdit->setText(formatAxisPosition(state.cmdPos, m_unitType));
    if (m_fbPosEdit)
        m_fbPosEdit->setText(formatAxisPosition(state.fbPos, m_unitType));

    m_lastRuntimeState = state;
    m_hasLastState     = true;
}

void AxisDebugPanel::refreshState()
{
    if (!m_service || m_axisName.isEmpty()) {
        m_hasLastState = false;
        updateIndicators({});
        return;
    }
    updateIndicators(m_service->axisState(m_axisName));
}

double AxisDebugPanel::stepSize() const
{
    if (!m_stepGroup)
        return 0.1;
    if (auto *checked = m_stepGroup->checkedButton())
        return checked->property("step").toDouble();
    return 0.1;
}

void AxisDebugPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshState();
}

void AxisDebugPanel::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void AxisDebugPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace motion_control_plugin
