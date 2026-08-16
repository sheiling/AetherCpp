/**
 * @file axis_param_form_widget.cpp
 * @brief 轴参数表单：固定单元格 + 视口宽度驱动的 ParamFormGridHost 自适应列布局
 */

#include "axis_param_form_widget.h"

#include "motion_action_keys.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/controls/switch/pill_switch.h"
#include "component/group/themed_section_panel.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"

#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"
#include "uibase/thememanager.h"

#include <QAbstractSpinBox>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

namespace motion_control_plugin {

namespace {

using namespace ui;

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

QList<QPair<int, QString>> axisTypeItems()
{
    return {{0, trKey("motion.axis.type.linear")}, {1, trKey("motion.axis.type.servo")},
            {2, trKey("motion.axis.type.stepper")}};
}

QList<QPair<int, QString>> unitTypeItems()
{
    return {{0, trKey("motion.axis.unit.pulse")}, {1, trKey("motion.axis.unit.mm")},
            {2, trKey("motion.axis.unit.um")}, {3, trKey("motion.axis.unit.deg")}};
}

QList<QPair<int, QString>> encoderTypeItems()
{
    return {{0, trKey("motion.axis.encoder.incremental")}, {1, trKey("motion.axis.encoder.absolute")}};
}

QList<QPair<int, QString>> driverTypeItems()
{
    return {{0, trKey("motion.axis.driver.default")}, {1, trKey("motion.axis.driver.panasonic")}};
}

QList<QPair<int, QString>> homeDirItems()
{
    return {{0, trKey("motion.axis.dir.negative")}, {1, trKey("motion.axis.dir.positive")}};
}

QList<QPair<int, QString>> homeVelModeItems()
{
    return {{0, trKey("motion.axis.home_vel.low")}, {1, trKey("motion.axis.home_vel.high")}};
}

QList<QPair<int, QString>> homePinLogicItems()
{
    return {{-1, trKey("motion.axis.logic.unset")}, {0, trKey("motion.io.low_active")},
            {1, trKey("motion.io.high_active")}};
}

QList<QPair<int, QString>> counterSourceItems()
{
    return {{0, trKey("motion.axis.counter.cmd")}, {1, trKey("motion.axis.counter.encoder")}};
}

QList<QPair<int, QString>> softLimitActionItems()
{
    return {{0, trKey("motion.axis.stop.immediate")}, {1, trKey("motion.axis.stop.decel")}};
}

void fillHomeModeCombo(ThemedComboBox *combo, const QString &currentMode = QString())
{
    if (!combo)
        return;

    // item data 为设备侧回零模式字符串（如 Origin_FindHome），与 AxisConfigData.homeMode 一致
    struct HomeModeItem {
        const char *textKey;
        const char *data;
    };
    static const HomeModeItem items[] = {
        {"motion.axis.home_mode.origin", "Origin_FindHome"},
        {"motion.axis.home_mode.z", "Z_FindHome"},
        {"motion.axis.home_mode.limit", "Limit_FindHome"},
        {"motion.axis.home_mode.origin_z", "Origin_Add_Z_FindHome"},
        {"motion.axis.home_mode.limit_z", "Limit_Add_Z_FindHome"},
        {"motion.axis.home_mode.current", "Current_Position"},
        {"motion.axis.home_mode.enable", "Enable"},
    };

    combo->clear();
    for (const auto &item : items)
        combo->addItem(trKey(item.textKey), QString::fromLatin1(item.data));

    if (!currentMode.isEmpty()) {
        const int idx = combo->findData(currentMode);
        combo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
}

// 每个字段单元：固定宽 kFieldCellWidth，高 = 标签(30) + 间距(2) + 控件(36)
constexpr int kFieldCellWidth      = 168;
constexpr int kFieldLabelPixelSize = 10;
constexpr int kFieldLabelHeight    = 30;
constexpr int kFieldControlHeight  = 36;
constexpr int kFieldCellSpacing    = 2;
constexpr int kFieldCellHeight     = kFieldLabelHeight + kFieldCellSpacing + kFieldControlHeight;
constexpr int kFieldGridHSpacing   = 12;
constexpr int kFieldGridVSpacing   = 8;

void tuneFieldLabel(ThemedFormLabel *lbl)
{
    if (!lbl)
        return;
    lbl->setPixelSize(kFieldLabelPixelSize);
    lbl->setWordWrap(false);
    lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbl->setFixedSize(kFieldCellWidth, kFieldLabelHeight);
}

void tuneFieldControl(QWidget *control)
{
    if (!control)
        return;
    control->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    control->setFixedSize(kFieldCellWidth, kFieldControlHeight);
}

void finalizeFieldCell(QWidget *cell)
{
    if (!cell)
        return;
    cell->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cell->setFixedSize(kFieldCellWidth, kFieldCellHeight);
    cell->updateGeometry();
}

void finalizeFieldCells(const QList<QWidget *> &cells, QWidget *gridHost)
{
    for (QWidget *cell : cells)
        finalizeFieldCell(cell);

    if (!gridHost)
        return;

    for (auto *w : gridHost->findChildren<QWidget *>()) {
        if (qobject_cast<QAbstractSpinBox *>(w) || qobject_cast<ThemedComboBox *>(w))
            tuneFieldControl(w);
    }
}

void fillCombo(ThemedComboBox *combo, const QList<QPair<int, QString>> &items, int current)
{
    if (!combo)
        return;
    combo->clear();
    for (const auto &item : items)
        combo->addItem(item.second, item.first);
    const int idx = combo->findData(current);
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
}

int comboData(const ThemedComboBox *combo)
{
    return combo ? combo->currentData().toInt() : 0;
}

ThemedDoubleSpinBox *makeMoveParamSpinBox(QWidget *parent)
{
    auto *sp = makeDoubleSpinBox(parent);
    sp->setDecimals(3);
    sp->setRange(0, 1e6);
    return sp;
}

void refillComboPreserving(ThemedComboBox *combo, const QList<QPair<int, QString>> &items)
{
    if (!combo)
        return;
    const int current = comboData(combo);
    fillCombo(combo, items, current);
}

QWidget *wrapField(const char *labelKey, QWidget *control, QWidget *parent)
{
    // 标签存 trKey 属性，retranslateUi 时按 key 重译
    auto *cell = new QWidget(parent);
    auto *vl   = new QVBoxLayout(cell);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(kFieldCellSpacing);
    vl->setSizeConstraint(QLayout::SetFixedSize);

    auto *lbl = new ThemedFormLabel(trKey(labelKey), cell);
    lbl->setProperty("trKey", QString::fromLatin1(labelKey));
    tuneFieldLabel(lbl);
    vl->addWidget(lbl, 0, Qt::AlignTop);

    tuneFieldControl(control);
    vl->addWidget(control, 0, Qt::AlignTop);

    finalizeFieldCell(cell);
    return cell;
}

QWidget *wrapSwitchField(const char *labelKey, PillSwitch *sw, QWidget *parent)
{
    auto *cell = new QWidget(parent);
    auto *vl   = new QVBoxLayout(cell);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(kFieldCellSpacing);
    vl->setSizeConstraint(QLayout::SetFixedSize);

    auto *lbl = new ThemedFormLabel(trKey(labelKey), cell);
    lbl->setProperty("trKey", QString::fromLatin1(labelKey));
    tuneFieldLabel(lbl);
    vl->addWidget(lbl, 0, Qt::AlignTop);

    auto *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(sw, 0, Qt::AlignLeft | Qt::AlignVCenter);
    row->addStretch();
    vl->addLayout(row);
    finalizeFieldCell(cell);
    return cell;
}

/**
 * 自适应列网格宿主。
 *
 * - 监听 scroll viewport 的 Resize，按 `available / (cellWidth + hSpacing)` 计算列数
 * - 行 stretch 恒为 0，minimumSize 等于内容行数；配合 widgetResizable 的 QScrollArea，
 *   内容能放下时不出现滚动条，视口更高时在网格下方留白
 */
class ParamFormGridHost final : public QWidget
{
public:
    explicit ParamFormGridHost(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        m_grid = new QGridLayout(this);
        m_grid->setContentsMargins(0, 0, 0, 0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    void setCells(const QList<QWidget *> &cells, int cellWidth, int hSpacing, int vSpacing)
    {
        m_cells     = cells;
        m_cellWidth = qMax(1, cellWidth);
        m_hSpacing  = qMax(0, hSpacing);
        m_vSpacing  = qMax(0, vSpacing);
        m_grid->setHorizontalSpacing(m_hSpacing);
        m_grid->setVerticalSpacing(m_vSpacing);
        reflow();
    }

    void setViewport(QWidget *viewport)
    {
        if (m_viewport)
            m_viewport->removeEventFilter(this);
        m_viewport = viewport;
        if (m_viewport)
            m_viewport->installEventFilter(this);
        reflow();
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_viewport && event->type() == QEvent::Resize) {
            reflow();
            return false;
        }
        return QWidget::eventFilter(watched, event);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        reflow();
    }

    QSize minimumSizeHint() const override
    {
        return {kFieldCellWidth, kFieldCellHeight};
    }

    QSize sizeHint() const override
    {
        if (m_cells.isEmpty())
            return minimumSizeHint();

        const int cols = qMax(1, m_lastColumns);
        const int rows = (m_cells.size() + cols - 1) / cols;
        const int w    = cols * m_cellWidth + qMax(0, cols - 1) * m_hSpacing;
        const int h    = rows * kFieldCellHeight + qMax(0, rows - 1) * m_vSpacing;
        return {w, h};
    }

private:
    void reflow()
    {
        // 清空 grid 但不销毁 cell widget，按新列数重新 addWidget
        while (QLayoutItem *item = m_grid->takeAt(0)) {
            if (item->widget())
                item->widget()->setParent(this);
            delete item;
        }

        if (m_cells.isEmpty())
            return;

        const int available = qMax(kFieldCellWidth, m_viewport ? m_viewport->width() : width());
        const int unit      = m_cellWidth + m_hSpacing;
        int       columns   = qMax(1, available / unit);
        columns             = qMin(columns, m_cells.size());

        int col = 0;
        int row = 0;
        int maxRow = 0;
        for (QWidget *cell : m_cells) {
            m_grid->addWidget(cell, row, col, Qt::AlignTop | Qt::AlignLeft);
            maxRow = qMax(maxRow, row);
            ++col;
            if (col >= columns) {
                col = 0;
                ++row;
            }
        }
        for (int c = 0; c < columns; ++c) {
            m_grid->setColumnStretch(c, 0);
            m_grid->setColumnMinimumWidth(c, m_cellWidth);
        }
        for (int c = columns; c < m_lastColumns; ++c)
            m_grid->setColumnMinimumWidth(c, 0);
        m_lastColumns = columns;
        for (int r = 0; r <= maxRow; ++r)
            m_grid->setRowStretch(r, 0);

        const int contentW = columns * m_cellWidth + qMax(0, columns - 1) * m_hSpacing;
        const int contentH = (maxRow + 1) * kFieldCellHeight + qMax(0, maxRow) * m_vSpacing;
        setMinimumSize(contentW, contentH);
        updateGeometry();
    }

    QList<QWidget *> m_cells;
    QGridLayout     *m_grid         = nullptr;
    QWidget         *m_viewport     = nullptr;
    int              m_cellWidth    = kFieldCellWidth;
    int              m_hSpacing     = kFieldGridHSpacing;
    int              m_vSpacing     = kFieldGridVSpacing;
    int              m_lastColumns  = 0;
};

} // namespace

AxisParamFormWidget::AxisParamFormWidget(IMotionControlService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    setObjectName(QStringLiteral("axisParamForm"));
    buildUi();
    retranslateUi();
    refreshContentStyle();
    applySavePermission();

    bindThemeRefresh(this, [this]() { refreshContentStyle(); });
    connect(TM, &ThemeManager::themeChanged, this, [this]() { refreshContentStyle(); });

    connect(PermService::instance(), &PermService::permissionsChanged, this,
            &AxisParamFormWidget::applySavePermission);
}

void AxisParamFormWidget::buildUi()
{
    // ThemedSectionPanel → m_contentBg(t.background) → QScrollArea(ParamFormGridHost) + 保存按钮
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_panel = new ui::ThemedSectionPanel(QStringLiteral("motion.axis.param_title"), this);
    m_panel->setContentMargins(12, 10, 12, 12);
    m_panel->setContentSpacing(10);
    root->addWidget(m_panel, 1);

    m_contentBg = new QFrame(m_panel->contentHost());
    m_contentBg->setObjectName(QStringLiteral("motionAxisParamContentBg"));
    m_contentBg->setAttribute(Qt::WA_StyledBackground, true);
    m_contentBg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentBg->setMinimumHeight(160);
    auto *contentBgLay = new QVBoxLayout(m_contentBg);
    contentBgLay->setContentsMargins(14, 12, 14, 14);
    contentBgLay->setSpacing(10);
    m_panel->contentLayout()->addWidget(m_contentBg, 1);

    auto *gridHost = new ParamFormGridHost(m_contentBg);

    m_enabled = makePillSwitch(true, gridHost);
    m_axisNo  = makeSpinBox(gridHost);
    m_axisNo->setRange(0, 128);

    m_axisType = makeComboBox(gridHost);
    fillCombo(m_axisType, axisTypeItems(), 1);

    m_unitType = makeComboBox(gridHost);
    fillCombo(m_unitType, unitTypeItems(), 1);

    m_encoderType = makeComboBox(gridHost);
    fillCombo(m_encoderType, encoderTypeItems(), 0);

    m_driverType = makeComboBox(gridHost);
    fillCombo(m_driverType, driverTypeItems(), 0);

    m_cardName = makeComboBox(gridHost);

    m_startVel = makeMoveParamSpinBox(gridHost);
    m_stopVel  = makeMoveParamSpinBox(gridHost);
    m_maxVel   = makeMoveParamSpinBox(gridHost);
    m_tacc     = makeMoveParamSpinBox(gridHost);
    m_tdec     = makeMoveParamSpinBox(gridHost);
    m_sPara    = makeMoveParamSpinBox(gridHost);

    m_homeDir = makeComboBox(gridHost);
    fillCombo(m_homeDir, homeDirItems(), 1);

    m_homeOffset = makeDoubleSpinBox(gridHost);
    m_homeOffset->setDecimals(3);
    m_homeOffset->setRange(-1e6, 1e6);

    m_homeVelMode = makeComboBox(gridHost);
    fillCombo(m_homeVelMode, homeVelModeItems(), 1);

    m_homePinLogic = makeComboBox(gridHost);
    fillCombo(m_homePinLogic, homePinLogicItems(), -1);

    m_homeSourceSel = makeComboBox(gridHost);
    fillCombo(m_homeSourceSel, counterSourceItems(), 1);

    m_homeMode = makeComboBox(gridHost);
    fillHomeModeCombo(m_homeMode, QStringLiteral("Origin_FindHome"));

    m_homeHighVel = makeDoubleSpinBox(gridHost);
    m_homeHighVel->setDecimals(2);
    m_homeHighVel->setRange(0, 1e5);
    m_homeTacc = makeDoubleSpinBox(gridHost);
    m_homeTacc->setDecimals(3);
    m_homeTacc->setRange(0, 1e3);
    m_homeLowVel = makeDoubleSpinBox(gridHost);
    m_homeLowVel->setDecimals(2);
    m_homeLowVel->setRange(0, 1e5);

    m_pulseEquivalent = makeDoubleSpinBox(gridHost);
    m_pulseEquivalent->setDecimals(4);
    m_pulseEquivalent->setRange(-1, 1e6);
    m_pulseUnit = makeDoubleSpinBox(gridHost);
    m_pulseUnit->setDecimals(4);
    m_pulseUnit->setRange(0, 1e6);

    m_negativeLimit = makeDoubleSpinBox(gridHost);
    m_negativeLimit->setDecimals(3);
    m_negativeLimit->setRange(-1e6, 1e6);
    m_positiveLimit = makeDoubleSpinBox(gridHost);
    m_positiveLimit->setDecimals(3);
    m_positiveLimit->setRange(-1e6, 1e6);

    m_errorBand = makeDoubleSpinBox(gridHost);
    m_errorBand->setDecimals(3);
    m_errorBand->setRange(0, 1e6);

    m_sourceSelLimit = makeComboBox(gridHost);
    fillCombo(m_sourceSelLimit, counterSourceItems(), 1);

    m_softLimitAction = makeComboBox(gridHost);
    fillCombo(m_softLimitAction, softLimitActionItems(), 0);

    const QList<QWidget *> cells = {
        wrapSwitchField("motion.axis.field.enabled", m_enabled, gridHost),
        wrapField("motion.axis.field.axis_no", m_axisNo, gridHost),
        wrapField("motion.axis.field.axis_type", m_axisType, gridHost),
        wrapField("motion.axis.field.unit_type", m_unitType, gridHost),
        wrapField("motion.axis.field.encoder_type", m_encoderType, gridHost),
        wrapField("motion.axis.field.driver_type", m_driverType, gridHost),
        wrapField("motion.axis.field.card_name", m_cardName, gridHost),
        wrapField("motion.axis.field.start_vel", m_startVel, gridHost),
        wrapField("motion.axis.field.stop_vel", m_stopVel, gridHost),
        wrapField("motion.axis.field.max_vel", m_maxVel, gridHost),
        wrapField("motion.axis.field.tacc", m_tacc, gridHost),
        wrapField("motion.axis.field.tdec", m_tdec, gridHost),
        wrapField("motion.axis.field.s_para", m_sPara, gridHost),
        wrapField("motion.axis.field.home_mode", m_homeMode, gridHost),
        wrapField("motion.axis.field.home_dir", m_homeDir, gridHost),
        wrapField("motion.axis.field.home_offset", m_homeOffset, gridHost),
        wrapField("motion.axis.field.home_vel_mode", m_homeVelMode, gridHost),
        wrapField("motion.axis.field.home_pin_logic", m_homePinLogic, gridHost),
        wrapField("motion.axis.field.home_source", m_homeSourceSel, gridHost),
        wrapField("motion.axis.field.home_high_vel", m_homeHighVel, gridHost),
        wrapField("motion.axis.field.home_low_vel", m_homeLowVel, gridHost),
        wrapField("motion.axis.field.home_tacc", m_homeTacc, gridHost),
        wrapField("motion.axis.field.pulse_equiv", m_pulseEquivalent, gridHost),
        wrapField("motion.axis.field.pulse_unit", m_pulseUnit, gridHost),
        wrapField("motion.axis.field.neg_limit", m_negativeLimit, gridHost),
        wrapField("motion.axis.field.pos_limit", m_positiveLimit, gridHost),
        wrapField("motion.axis.field.error_band", m_errorBand, gridHost),
        wrapField("motion.axis.field.soft_limit_src", m_sourceSelLimit, gridHost),
        wrapField("motion.axis.field.soft_limit_action", m_softLimitAction, gridHost),
    };

    gridHost->setCells(cells, kFieldCellWidth, kFieldGridHSpacing, kFieldGridVSpacing);

    auto *scroll = new QScrollArea(m_contentBg);
    scroll->setObjectName(QStringLiteral("axisParamFormScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll->setWidget(gridHost);
    gridHost->setViewport(scroll->viewport());
    contentBgLay->addWidget(scroll, 1);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    m_saveBtn = makePushButton(QString(), PushButtonVariant::Primary, m_contentBg);
    m_saveBtn->setProperty("trKey", QStringLiteral("motion.axis.save_btn"));
    connect(m_saveBtn, &QPushButton::clicked, this, &AxisParamFormWidget::onSaveClicked);
    btnRow->addWidget(m_saveBtn);
    contentBgLay->addLayout(btnRow);

    ensureThemedSpinStyles(gridHost);
    finalizeFieldCells(cells, gridHost);
}

void AxisParamFormWidget::refreshContentStyle()
{
    if (!m_contentBg)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    m_contentBg->setStyleSheet(QStringLiteral(
        "QFrame#motionAxisParamContentBg {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 4px;"
        "}")
                                   .arg(t.background.name()));
}

void AxisParamFormWidget::loadAxis(const QString &axisName)
{
    m_axisName = axisName;
    if (!m_service || axisName.isEmpty())
        return;

    const AxisConfigData cfg = m_service->axisConfig(axisName);
    bindCardCombo(m_service->motionCardNames(), cfg.cardName);
    bindFields(cfg);
}

void AxisParamFormWidget::bindCardCombo(const QStringList &cardNames, const QString &current)
{
    if (!m_cardName)
        return;

    m_cardName->clear();
    QStringList options = cardNames;
    if (!current.isEmpty() && !options.contains(current))
        options.prepend(current);
    options.removeDuplicates();
    for (const QString &name : options)
        m_cardName->addItem(name, name);

    const int idx = m_cardName->findText(current);
    m_cardName->setCurrentIndex(idx >= 0 ? idx : 0);
}

void AxisParamFormWidget::bindFields(const AxisConfigData &cfg)
{
    // 各 Combo 用 fillCombo/fillHomeModeCombo 按 int/string data 选中，与 collectConfig 对称
    if (m_enabled)
        m_enabled->setChecked(cfg.enabled);
    if (m_axisNo)
        m_axisNo->setValue(cfg.axisNo);
    if (m_axisType)
        fillCombo(m_axisType, axisTypeItems(), cfg.axisType);
    if (m_unitType)
        fillCombo(m_unitType, unitTypeItems(), cfg.unitType);
    if (m_encoderType)
        fillCombo(m_encoderType, encoderTypeItems(), cfg.encoderType);
    if (m_driverType)
        fillCombo(m_driverType, driverTypeItems(), cfg.driverType);
    if (m_cardName && m_cardName->findText(cfg.cardName) >= 0)
        m_cardName->setCurrentText(cfg.cardName);
    if (m_startVel)
        m_startVel->setValue(cfg.startVel);
    if (m_stopVel)
        m_stopVel->setValue(cfg.stopVel);
    if (m_maxVel)
        m_maxVel->setValue(cfg.maxVel);
    if (m_tacc)
        m_tacc->setValue(cfg.tacc);
    if (m_tdec)
        m_tdec->setValue(cfg.tdec);
    if (m_sPara)
        m_sPara->setValue(cfg.sPara);
    if (m_homeMode)
        fillHomeModeCombo(m_homeMode, cfg.homeMode);
    if (m_homeDir)
        fillCombo(m_homeDir, homeDirItems(), cfg.homeDir);
    if (m_homeOffset)
        m_homeOffset->setValue(cfg.homeOffset);
    if (m_homeVelMode)
        fillCombo(m_homeVelMode, homeVelModeItems(), cfg.homeVelMode);
    if (m_homePinLogic)
        fillCombo(m_homePinLogic, homePinLogicItems(), cfg.homePinLogic);
    if (m_homeSourceSel)
        fillCombo(m_homeSourceSel, counterSourceItems(), cfg.homeSourceSel);
    if (m_homeHighVel)
        m_homeHighVel->setValue(cfg.homeHighVel);
    if (m_homeLowVel)
        m_homeLowVel->setValue(cfg.homeLowVel);
    if (m_homeTacc)
        m_homeTacc->setValue(cfg.homeTacc);
    if (m_pulseEquivalent)
        m_pulseEquivalent->setValue(cfg.pulseEquivalent);
    if (m_pulseUnit)
        m_pulseUnit->setValue(cfg.pulseUnit);
    if (m_negativeLimit)
        m_negativeLimit->setValue(cfg.negativeLimit);
    if (m_positiveLimit)
        m_positiveLimit->setValue(cfg.positiveLimit);
    if (m_errorBand)
        m_errorBand->setValue(cfg.errorBandPluse);
    if (m_sourceSelLimit)
        fillCombo(m_sourceSelLimit, counterSourceItems(), cfg.sourceSelLimit);
    if (m_softLimitAction)
        fillCombo(m_softLimitAction, softLimitActionItems(), cfg.softwareLimitAction);
}

void AxisParamFormWidget::refillAllCombos()
{
    refillComboPreserving(m_axisType, axisTypeItems());
    refillComboPreserving(m_unitType, unitTypeItems());
    refillComboPreserving(m_encoderType, encoderTypeItems());
    refillComboPreserving(m_driverType, driverTypeItems());
    refillComboPreserving(m_homeDir, homeDirItems());
    refillComboPreserving(m_homeVelMode, homeVelModeItems());
    refillComboPreserving(m_homePinLogic, homePinLogicItems());
    refillComboPreserving(m_homeSourceSel, counterSourceItems());
    if (m_homeMode)
        fillHomeModeCombo(m_homeMode, m_homeMode->currentData().toString());
    refillComboPreserving(m_sourceSelLimit, counterSourceItems());
    refillComboPreserving(m_softLimitAction, softLimitActionItems());
}

void AxisParamFormWidget::retranslateUi()
{
    if (m_saveBtn) {
        const QVariant key = m_saveBtn->property("trKey");
        if (key.isValid() && !key.toString().isEmpty())
            m_saveBtn->setText(trKey(key.toString().toLatin1().constData()));
    }
    for (auto *lbl : findChildren<ThemedFormLabel *>()) {
        const QVariant key = lbl->property("trKey");
        if (key.isValid() && !key.toString().isEmpty())
            lbl->setText(trKey(key.toString().toLatin1().constData()));
    }
    refillAllCombos();
}

AxisConfigData AxisParamFormWidget::collectConfig() const
{
    AxisConfigData cfg;
    cfg.name                = m_axisName;
    cfg.enabled             = m_enabled ? m_enabled->isChecked() : true;
    cfg.axisNo              = m_axisNo ? m_axisNo->value() : 0;
    cfg.axisType            = comboData(m_axisType);
    cfg.unitType            = comboData(m_unitType);
    cfg.encoderType         = m_encoderType ? comboData(m_encoderType) : 0;
    cfg.driverType          = m_driverType ? comboData(m_driverType) : 0;
    cfg.cardName            = m_cardName ? m_cardName->currentText().trimmed() : QString();
    cfg.startVel            = m_startVel ? m_startVel->value() : 2.0;
    cfg.stopVel             = m_stopVel ? m_stopVel->value() : 2.0;
    cfg.maxVel              = m_maxVel ? m_maxVel->value() : 50.0;
    cfg.tacc                = m_tacc ? m_tacc->value() : 0.5;
    cfg.tdec                = m_tdec ? m_tdec->value() : 0.5;
    cfg.sPara               = m_sPara ? m_sPara->value() : 0.2;
    cfg.homeDir             = comboData(m_homeDir);
    cfg.homeOffset          = m_homeOffset ? m_homeOffset->value() : 0.0;
    cfg.homeVelMode         = comboData(m_homeVelMode);
    cfg.homePinLogic        = comboData(m_homePinLogic);
    cfg.homeSourceSel       = comboData(m_homeSourceSel);
    cfg.homeMode            = m_homeMode ? m_homeMode->currentData().toString() : QString();
    cfg.homeHighVel         = m_homeHighVel ? m_homeHighVel->value() : 0.0;
    cfg.homeTacc            = m_homeTacc ? m_homeTacc->value() : 0.0;
    cfg.homeLowVel          = m_homeLowVel ? m_homeLowVel->value() : 0.0;
    cfg.pulseEquivalent     = m_pulseEquivalent ? m_pulseEquivalent->value() : -1.0;
    cfg.pulseUnit           = m_pulseUnit ? m_pulseUnit->value() : 1.0;
    cfg.negativeLimit       = m_negativeLimit ? m_negativeLimit->value() : 0.0;
    cfg.positiveLimit       = m_positiveLimit ? m_positiveLimit->value() : 0.0;
    cfg.errorBandPluse      = m_errorBand ? m_errorBand->value() : 0.0;
    cfg.sourceSelLimit      = comboData(m_sourceSelLimit);
    cfg.softwareLimitAction = comboData(m_softLimitAction);
    return cfg;
}

void AxisParamFormWidget::onSaveClicked()
{
    if (!PermService::instance()->canAction(MotionActionKeys::AxisSave))
        return;
    if (!m_service || m_axisName.isEmpty())
        return;

    const AxisConfigData cfg = collectConfig();
    if (!m_service->setAxisConfig(m_axisName, cfg))
        return;
    m_service->saveDeviceConfig(); // 持久化 device_config，不仅内存
    emit configSaved(m_axisName);
}

void AxisParamFormWidget::applySavePermission()
{
    if (!m_saveBtn)
        return;
    m_saveBtn->setEnabled(PermService::instance()->canAction(MotionActionKeys::AxisSave));
}

void AxisParamFormWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace motion_control_plugin
