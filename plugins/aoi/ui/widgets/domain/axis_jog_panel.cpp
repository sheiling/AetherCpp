/**
 * @file axis_jog_panel.cpp
 * @brief AxisJogPanel 布局构建、紧凑样式与多语言刷新
 */

#include "domain/axis_jog_panel.h"

#include "component/controls/button/themed_icon_button.h"
#include "component/font/fa_icon_button.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/theme/theme_font.h"

#include "uibase/languagemanager.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

constexpr double kDefaultSpeedRatio = 0.1;

/** 标签 + 值控件水平组，objectName 为 recipeAxisFieldGroup */
QWidget *makeAxisFieldGroup(const QString &labelKey, QWidget *valueWidget, QWidget *parent)
{
    auto *group = new QWidget(parent);
    group->setObjectName(QStringLiteral("recipeAxisFieldGroup"));
    auto *hl    = new QHBoxLayout(group);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(3);
    hl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hl->addWidget(ui::makeFormLabel(labelKey, group), 0, Qt::AlignVCenter);
    hl->addWidget(valueWidget, 0, Qt::AlignVCenter);
    return group;
}

/** 区块小标题；trKey 属性供 retranslatePanel 重译 */
QLabel *makeDebugSectionTitle(const char *key, QWidget *parent)
{
    auto *lbl = new QLabel(TR(key), parent);
    lbl->setObjectName(QStringLiteral("recipeDebugSectionTitle"));
    lbl->setProperty("trKey", QString::fromUtf8(key));
    return lbl;
}

ui::ThemedComboBox *makeSpeedRatioCombo(QWidget *parent)
{
    auto *combo = ui::makeComboBox(parent);
    combo->setObjectName(QStringLiteral("recipeAxisSpeedCombo"));
    const QList<double> ratios = {0.5, 0.1, 0.001};
    for (double ratio : ratios) {
        combo->addItem(QString::number(ratio, 'f', 3), ratio);
    }
    const int defaultIndex = combo->findData(kDefaultSpeedRatio);
    combo->setCurrentIndex(defaultIndex >= 0 ? defaultIndex : 1);
    combo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return combo;
}

char logicalAxisLetter(const QString &axisKey)
{
    const QString upper = axisKey.toUpper();
    if (upper.contains(QLatin1Char('X')))
        return 'X';
    if (upper.contains(QLatin1Char('Y')))
        return 'Y';
    if (upper.contains(QLatin1Char('Z')))
        return 'Z';
    return '\0';
}

} // namespace

AxisJogPanel::AxisJogPanel(const QStringList &axisNames, QWidget *parent)
    : QWidget(parent)
    , m_axisNames(axisNames)
{
    setObjectName(QStringLiteral("recipeAxisJogPanel"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(4);

    root->addWidget(makeDebugSectionTitle("aoi.recipe.debug_title", this));

    m_axisWrap = new QWidget(this);
    m_axisWrap->setObjectName(QStringLiteral("recipeAxisSettings"));
    m_axisWrap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto *axisLayout = new QVBoxLayout(m_axisWrap);
    axisLayout->setContentsMargins(4, 0, 0, 4);
    axisLayout->setSpacing(6);

    root->addWidget(m_axisWrap, 0, Qt::AlignTop);

    rebuildAxisRows();
}

ui::TriangleDirection AxisJogPanel::inferNegDirection(const QString &axisKey) const
{
    switch (logicalAxisLetter(axisKey)) {
    case 'X':
        return ui::TriangleDirection::Left;
    case 'Y':
        return ui::TriangleDirection::Down;
    default:
        return ui::TriangleDirection::Down;
    }
}

ui::TriangleDirection AxisJogPanel::inferPosDirection(const QString &axisKey) const
{
    switch (logicalAxisLetter(axisKey)) {
    case 'X':
        return ui::TriangleDirection::Right;
    case 'Y':
        return ui::TriangleDirection::Up;
    default:
        return ui::TriangleDirection::Up;
    }
}

void AxisJogPanel::rebuildAxisRows()
{
    m_rows.clear();
    if (!m_axisWrap)
        return;

    if (auto *layout = m_axisWrap->layout()) {
        while (layout->count() > 0) {
            if (auto *item = layout->takeAt(0)) {
                if (auto *widget = item->widget())
                    widget->deleteLater();
                delete item;
            }
        }
    }

    for (const QString &axisKey : m_axisNames) {
        if (auto *layout = m_axisWrap->layout())
            layout->addWidget(buildAxisRow(axisKey));
    }

    updateVisibility();
}

void AxisJogPanel::updateVisibility()
{
    const bool hasAxes = !m_axisNames.isEmpty();
    setVisible(hasAxes);
    if (auto *wrap = parentWidget())
        wrap->setVisible(hasAxes);
}

/** 单行：轴名 | 速度比例 | ±点动 | 目标+绝对/相对 | 指令位 | 反馈位 */
QWidget *AxisJogPanel::buildAxisRow(const QString &axisKey)
{
    auto *row = new QFrame(m_axisWrap);
    row->setObjectName(QStringLiteral("recipeAxisRow"));
    row->setAttribute(Qt::WA_StyledBackground, true);
    row->setProperty("axis", axisKey);

    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(6, 7, 6, 8);
    hl->setSpacing(0);
    hl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    constexpr int kGroupGap = 14;

    auto *axisName = new QLabel(axisKey, row);
    axisName->setObjectName(QStringLiteral("recipeAxisName"));
    axisName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hl->addWidget(axisName, 0, Qt::AlignVCenter);
    hl->addSpacing(8);

    auto *speedCombo = makeSpeedRatioCombo(row);
    hl->addWidget(makeAxisFieldGroup(QStringLiteral("aoi.recipe.axis.speed_ratio"), speedCombo, row),
                  0, Qt::AlignVCenter);
    hl->addSpacing(kGroupGap);

    auto *motionWrap = new QWidget(row);
    motionWrap->setObjectName(QStringLiteral("recipeMotionBtns"));
    auto *motionRow  = new QHBoxLayout(motionWrap);
    motionRow->setContentsMargins(0, 0, 0, 0);
    motionRow->setSpacing(4);
    motionRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    const QString axisUpper = axisKey.toUpper();
    auto *negBtn = ui::makeFaIconButton(inferNegDirection(axisKey), motionWrap);
    negBtn->setObjectName(QStringLiteral("axisBtn"));
    negBtn->setToolTip(TR("aoi.recipe.axis.jog_neg").arg(axisUpper));
    auto *posBtn = ui::makeFaIconButton(inferPosDirection(axisKey), motionWrap);
    posBtn->setObjectName(QStringLiteral("axisBtn"));
    posBtn->setToolTip(TR("aoi.recipe.axis.jog_pos").arg(axisUpper));

    motionRow->addWidget(negBtn, 0, Qt::AlignVCenter);
    motionRow->addWidget(posBtn, 0, Qt::AlignVCenter);
    hl->addWidget(motionWrap, 0, Qt::AlignVCenter);
    hl->addSpacing(kGroupGap);

    auto *moveWrap = new QWidget(row);
    moveWrap->setObjectName(QStringLiteral("recipeAxisMoveGroup"));
    auto *moveRow  = new QHBoxLayout(moveWrap);
    moveRow->setContentsMargins(0, 0, 0, 0);
    moveRow->setSpacing(4);
    moveRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *targetSpin = makeDoubleSpinBox(moveWrap, QStringLiteral("recipeAxisTargetSpin"));
    targetSpin->setDecimals(3);
    targetSpin->setRange(-999999.0, 999999.0);
    targetSpin->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto makeMoveBtn = [moveWrap](const char *trKey) {
        auto *btn = makePushButton(TR(trKey), PushButtonVariant::Secondary, moveWrap);
        btn->setObjectName(QStringLiteral("recipeAxisMoveBtn"));
        btn->setProperty("trKey", QString::fromUtf8(trKey));
        return btn;
    };
    auto *absBtn = makeMoveBtn("aoi.recipe.axis.absolute");
    auto *relBtn = makeMoveBtn("aoi.recipe.axis.relative");

    moveRow->addWidget(targetSpin, 0, Qt::AlignVCenter);
    moveRow->addWidget(absBtn, 0, Qt::AlignVCenter);
    moveRow->addWidget(relBtn, 0, Qt::AlignVCenter);
    hl->addWidget(moveWrap, 0, Qt::AlignVCenter);
    hl->addSpacing(kGroupGap);

    auto *cmdLbl = new QLabel(QStringLiteral("0.000"), row);
    cmdLbl->setObjectName(QStringLiteral("recipeAxisPosValue"));
    cmdLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hl->addWidget(makeAxisFieldGroup(QStringLiteral("aoi.recipe.axis.cmd_pos"), cmdLbl, row), 0,
                  Qt::AlignVCenter);
    hl->addSpacing(kGroupGap);

    auto *fbLbl = new QLabel(QStringLiteral("0.000"), row);
    fbLbl->setObjectName(QStringLiteral("recipeAxisPosValue"));
    fbLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hl->addWidget(makeAxisFieldGroup(QStringLiteral("aoi.recipe.axis.fb_pos"), fbLbl, row), 0,
                  Qt::AlignVCenter);
    hl->addStretch(1);

    const auto emitJogPressed = [this, axisKey, speedCombo](int direction) {
        emit jogPressed(axisKey, direction, speedCombo->currentData().toDouble());
    };
    connect(negBtn, &QAbstractButton::pressed, this, [emitJogPressed]() { emitJogPressed(0); });
    connect(posBtn, &QAbstractButton::pressed, this, [emitJogPressed]() { emitJogPressed(1); });
    connect(negBtn, &QAbstractButton::released, this, [this, axisKey]() { emit jogReleased(axisKey); });
    connect(posBtn, &QAbstractButton::released, this, [this, axisKey]() { emit jogReleased(axisKey); });
    connect(absBtn, &QPushButton::clicked, this, [this, axisKey, targetSpin]() {
        emit absoluteMoveRequested(axisKey, targetSpin->value());
    });
    connect(relBtn, &QPushButton::clicked, this, [this, axisKey, targetSpin]() {
        emit relativeMoveRequested(axisKey, targetSpin->value());
    });

    AxisRowWidgets widgets;
    widgets.speedRatio = speedCombo;
    widgets.negBtn     = negBtn;
    widgets.posBtn     = posBtn;
    widgets.targetPos  = targetSpin;
    widgets.absMoveBtn = absBtn;
    widgets.relMoveBtn = relBtn;
    widgets.cmdPos     = cmdLbl;
    widgets.fbPos      = fbLbl;
    m_rows.insert(axisKey, widgets);

    return row;
}

void AxisJogPanel::setAxisNames(const QStringList &axisNames)
{
    if (m_axisNames == axisNames) {
        updateVisibility();
        return;
    }
    m_axisNames = axisNames;
    rebuildAxisRows();
}

AxisRowWidgets AxisJogPanel::rowWidgets(const QString &axisKey) const
{
    return m_rows.value(axisKey);
}

double AxisJogPanel::speedRatio(const QString &axisKey) const
{
    const AxisRowWidgets w = m_rows.value(axisKey);
    if (!w.speedRatio)
        return kDefaultSpeedRatio;
    return w.speedRatio->currentData().toDouble();
}

double AxisJogPanel::feedbackPosition(const QString &axisKey) const
{
    const AxisRowWidgets w = m_rows.value(axisKey);
    return w.fbPos ? w.fbPos->text().toDouble() : 0.0;
}

double AxisJogPanel::targetPosition(const QString &axisKey) const
{
    const AxisRowWidgets w = m_rows.value(axisKey);
    return w.targetPos ? w.targetPos->value() : 0.0;
}

void AxisJogPanel::setAxisPositions(const QString &axisKey, double cmdPos, double fbPos)
{
    const AxisRowWidgets w = m_rows.value(axisKey);
    if (w.cmdPos)
        w.cmdPos->setText(QString::number(cmdPos, 'f', 3));
    if (w.fbPos)
        w.fbPos->setText(QString::number(fbPos, 'f', 3));
}

void AxisJogPanel::resetForm()
{
    for (auto it = m_rows.constBegin(); it != m_rows.constEnd(); ++it) {
        const AxisRowWidgets &w = it.value();
        if (w.speedRatio) {
            const int idx = w.speedRatio->findData(kDefaultSpeedRatio);
            w.speedRatio->setCurrentIndex(idx >= 0 ? idx : 1);
        }
        if (w.cmdPos)
            w.cmdPos->setText(QStringLiteral("0.000"));
        if (w.fbPos)
            w.fbPos->setText(QStringLiteral("0.000"));
        if (w.targetPos)
            w.targetPos->setValue(0.0);
    }
}

/** 语言切换：表单标签 + 带 trKey 的调试标题 */
void AxisJogPanel::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);
    for (auto *lbl : findChildren<QLabel *>(QStringLiteral("recipeDebugSectionTitle"))) {
        const QVariant key = lbl->property("trKey");
        if (key.isValid())
            lbl->setText(TR(key.toString().toUtf8().constData()));
    }
    for (auto *btn : findChildren<ThemedPushButton *>(QStringLiteral("recipeAxisMoveBtn"))) {
        const QVariant key = btn->property("trKey");
        if (key.isValid())
            btn->setText(TR(key.toString().toUtf8().constData()));
    }
}

/** 按 recipeAxisRow 递归设置字体与控件固定尺寸（配方页紧凑模式） */
void AxisJogPanel::refreshCompactLayout()
{
    if (!m_axisWrap)
        return;

    QFont labelFont = themeUiFont(10);
    labelFont.setPixelSize(12);
    QFont comboFont = themeUiFont(10);
    comboFont.setPixelSize(12);

    constexpr int kControlH       = 28;
    constexpr int kSpeedComboW    = 96;
    constexpr int kTargetSpinW    = 100;
    constexpr int kMoveBtnMinW    = 68;

    for (auto *row : m_axisWrap->findChildren<QFrame *>(QStringLiteral("recipeAxisRow"))) {
        if (auto *name = row->findChild<QLabel *>(QStringLiteral("recipeAxisName")))
            name->setFont(labelFont);
        for (auto *lbl : row->findChildren<ui::ThemedFormLabel *>())
            lbl->setFont(labelFont);

        for (auto *combo : row->findChildren<ui::ThemedComboBox *>(QStringLiteral("recipeAxisSpeedCombo"))) {
            combo->setFont(comboFont);
            combo->setFixedSize(kSpeedComboW, kControlH);
        }

        for (auto *spin :
             row->findChildren<ui::ThemedDoubleSpinBox *>(QStringLiteral("recipeAxisTargetSpin"))) {
            spin->setFont(comboFont);
            spin->setFixedSize(kTargetSpinW, kControlH);
        }

        for (auto *btn : row->findChildren<ThemedPushButton *>(QStringLiteral("recipeAxisMoveBtn"))) {
            btn->setFont(comboFont);
            btn->setFixedHeight(kControlH);
            btn->setMinimumWidth(kMoveBtnMinW);
        }

        for (auto *btn : row->findChildren<QPushButton *>(QStringLiteral("axisBtn"))) {
            btn->setFont(comboFont);
            btn->setFixedSize(28, kControlH);
        }

        row->setMinimumHeight(kControlH + 24);
        row->setMaximumHeight(QWIDGETSIZE_MAX);
        row->updateGeometry();
    }

    m_axisWrap->adjustSize();
    m_axisWrap->setMinimumHeight(m_axisWrap->sizeHint().height() + 6);
}

void AxisJogPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslatePanel();
}

} // namespace aoi
