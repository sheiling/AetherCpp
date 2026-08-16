/**
 * @file io_param_form_widget.cpp
 * @brief IoParamFormWidget 实现：紧凑双列表单 + 内容驱动固定高度
 *
 * 布局常量见匿名命名空间；主题色通过 ThemeTokens 注入 QSS，避免 QWidget palette 叠色。
 */

#include "io_param_form_widget.h"

#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/indicator/themed_glass_indicator_dot.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/radio/themed_radio_button.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/controls/switch/pill_switch.h"
#include "component/behavior/no_wheel_input_guard.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"
#include "uibase/thememanager.h"

#include <QButtonGroup>
#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace motion_control_plugin {

namespace {

using namespace ui;

// | 常量 | 含义 |
// |------|------|
// | kFormRowHeight / kFormRowSpacing | 每行控件与标签固定高度、行间距 |
// | kHeaderHeight | 标题栏固定高度 |
// | kPadH / kPadV | 卡片左右/底、内容框内边距（仅 cardLay + contentLay 各一层，不重复叠加） |
// | kGapHeader | 标题栏与参数区之间的间距 |
// | kFormRowCount | 表单行数（卡名 / 有效电平 / 端口） |
// | kFormRowCount | 表单基础行数（卡名 / 有效电平 / 端口 / 扩展卡）；扩展卡号按勾选追加 |
constexpr int kFormLabelWidthMin = 56;
constexpr int kFormRowHeight    = 32;
constexpr int kFormRowSpacing   = 6;
constexpr int kFormRadioSpacing = 12;
constexpr int kHeaderHeight     = 34;
constexpr int kPadH             = 10;
constexpr int kPadV             = 6;
constexpr int kGapHeader        = 8;
constexpr int kFormColGap       = 10; ///< 标签列与控件列间距
constexpr int kFormBaseRowCount = 4;
constexpr int kFormMaxRowCount  = 5; ///< 含扩展卡号行的最大行数
constexpr int kComboExtraWidth  = 36; ///< 下拉箭头与内边距（与 ThemedComboBox QSS 一致）
constexpr int kRadioIndicatorW  = 22; ///< 单选圆点 + 与文案间距
constexpr int kCtrlWidthMin     = 72;

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

void styleFormLabel(ThemedFormLabel *label, int labelWidth, int rowHeight = kFormRowHeight)
{
    if (!label)
        return;
    label->setFixedSize(labelWidth, rowHeight);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label->setPixelSize(10);
}

/** 按当前文案测量标签列宽（中英文切换后重算，避免固定 56px 挤压） */
int calcFormLabelWidth(const QList<ThemedFormLabel *> &labels)
{
    int          maxW = kFormLabelWidthMin;
    QFontMetrics fm(labels.isEmpty() ? QFont() : labels.first()->font());
    for (const ThemedFormLabel *lbl : labels) {
        if (!lbl)
            continue;
        fm = QFontMetrics(lbl->font());
        maxW = qMax(maxW, fm.horizontalAdvance(lbl->text()) + 4);
    }
    return maxW;
}

/** 按当前字体测量卡名下拉所需宽度（不依赖 polish 后 sizeHint，避免换主题后变窄） */
int comboFieldWidth(const ThemedComboBox *combo)
{
    if (!combo)
        return 0;

    QFontMetrics fm(combo->font());
    int          textW = fm.horizontalAdvance(combo->currentText());
    for (int i = 0; i < combo->count(); ++i)
        textW = qMax(textW, fm.horizontalAdvance(combo->itemText(i)));
    return textW + kComboExtraWidth;
}

/** 按 FontMetrics 测量有效电平单选行宽 */
int radioRowWidth(const ThemedRadioButton *lowActive, const ThemedRadioButton *highActive)
{
    if (!lowActive || !highActive)
        return 0;

    QFontMetrics fm(lowActive->font());
    const int    lowW  = kRadioIndicatorW + fm.horizontalAdvance(lowActive->text());
    const int    highW = kRadioIndicatorW + fm.horizontalAdvance(highActive->text());
    return lowW + kFormRadioSpacing + highW;
}

/** 控件列理想宽度：FontMetrics 测量 + Spin hint */
int calcIdealControlWidth(const ThemedRadioButton *lowActive, const ThemedRadioButton *highActive,
                          const ThemedSpinBox *portSpin, const ThemedSpinBox *nodeSpin,
                          const ThemedComboBox *cardCombo)
{
    int ctrlWidth = radioRowWidth(lowActive, highActive);

    if (portSpin) {
        portSpin->ensurePolished();
        ctrlWidth = qMax(ctrlWidth, portSpin->minimumSizeHint().width());
    }
    if (nodeSpin) {
        nodeSpin->ensurePolished();
        ctrlWidth = qMax(ctrlWidth, nodeSpin->minimumSizeHint().width());
    }
    ctrlWidth = qMax(ctrlWidth, comboFieldWidth(cardCombo));
    return qMax(ctrlWidth, kCtrlWidthMin);
}

/** 锁定网格行高与行距；未用行须清零 minimumHeight，避免隐藏行仍占位导致 spacing 被压扁 */
void applyFormGridMetrics(QGridLayout *grid, int activeRowCount)
{
    if (!grid || activeRowCount <= 0)
        return;

    grid->setVerticalSpacing(kFormRowSpacing);
    for (int row = 0; row < kFormMaxRowCount; ++row) {
        if (row < activeRowCount) {
            grid->setRowMinimumHeight(row, kFormRowHeight);
        } else {
            grid->setRowMinimumHeight(row, 0);
        }
        grid->setRowStretch(row, 0);
    }
}

} // namespace

IoParamFormWidget::IoParamFormWidget(IMotionControlService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    setObjectName(QStringLiteral("motionIoParamForm"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buildUi();
    retranslateUi();
    refreshCardStyle();

    bindThemeRefresh(this, [this]() { onThemeRefresh(); });
}

void IoParamFormWidget::onThemeRefresh()
{
    refreshCardStyle();
    syncLevelRowLayout();
    updateGeometry();
}

void IoParamFormWidget::buildUi()
{
    // 根布局仅承载 m_cardFrame；纵向 Fixed 由 syncCardLayout 写入
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_cardFrame = new QFrame(this);
    m_cardFrame->setObjectName(QStringLiteral("motionIoParamCard"));
    m_cardFrame->setAttribute(Qt::WA_StyledBackground, true);
    m_cardFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    root->addWidget(m_cardFrame);

    auto *cardLay = new QVBoxLayout(m_cardFrame);
    cardLay->setContentsMargins(kPadH, 0, kPadH, kPadH);
    cardLay->setSpacing(0);

    buildHeaderFrame(cardLay);
    buildContentFrame(cardLay);

    m_cardFrame->installEventFilter(this);
    if (m_contentFrame)
        m_contentFrame->installEventFilter(this);

    connect(m_portSpin, qOverload<int>(&ThemedSpinBox::valueChanged), this,
            [this](int) { updatePortHeader(); });
}

void IoParamFormWidget::buildHeaderFrame(QVBoxLayout *cardLay)
{
    // 左：DO/DI+端口；中：IO 名称；右：运行态指示灯；输出且可写时点击标题栏翻转
    m_headerFrame = new QFrame(m_cardFrame);
    m_headerFrame->setObjectName(QStringLiteral("motionIoParamHeader"));
    m_headerFrame->setAttribute(Qt::WA_StyledBackground, true);
    m_headerFrame->setFixedHeight(kHeaderHeight);
    m_headerFrame->installEventFilter(this);

    auto *headRow = new QHBoxLayout(m_headerFrame);
    headRow->setContentsMargins(kPadH, 0, kPadH, 0);
    headRow->setSpacing(8);

    m_portHeaderLbl = new ThemedFormLabel(QString(), m_headerFrame);
    m_portHeaderLbl->setObjectName(QStringLiteral("motionIoParamPort"));
    m_portHeaderLbl->setPixelSize(10);
    headRow->addWidget(m_portHeaderLbl, 0, Qt::AlignVCenter);

    m_titleLbl = new ThemedFormLabel(QString(), m_headerFrame);
    m_titleLbl->setObjectName(QStringLiteral("motionIoParamTitle"));
    m_titleLbl->setAlignment(Qt::AlignCenter);
    m_titleLbl->setPixelSize(11);
    headRow->addWidget(m_titleLbl, 1, Qt::AlignVCenter);

    m_stateDot = new ThemedGlassIndicatorDot(m_headerFrame);
    m_stateDot->setDiameter(12);
    headRow->addWidget(m_stateDot, 0, Qt::AlignVCenter);

    cardLay->addWidget(m_headerFrame);
}

void IoParamFormWidget::buildContentFrame(QVBoxLayout *cardLay)
{
    // m_contentSlot 不含 stretch；slot 不再叠加水平 margin，与标题栏共用 cardLay 的 kPadH
    m_contentSlot = new QWidget(m_cardFrame);
    m_contentSlot->setAttribute(Qt::WA_StyledBackground, false);
    m_contentSlot->setAutoFillBackground(false);
    m_contentSlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *slotLay = new QVBoxLayout(m_contentSlot);
    // 水平方向仅 cardLay 留 kPadH；此处只保留标题栏与参数区之间的上间距，避免左侧双层缩进
    slotLay->setContentsMargins(0, kGapHeader, 0, 0);
    slotLay->setSpacing(0);

    m_contentFrame = new QFrame(m_contentSlot);
    m_contentFrame->setObjectName(QStringLiteral("motionIoParamContent"));
    m_contentFrame->setAttribute(Qt::WA_StyledBackground, true);
    m_contentFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto *contentLay = new QVBoxLayout(m_contentFrame);
    contentLay->setContentsMargins(kPadH, kPadV, kPadH, kPadV);

    // QGridLayout + rowMinimumHeight：主题切换后子控件 polish 变高时也不压缩行距
    m_formGrid = new QGridLayout();
    m_formGrid->setContentsMargins(0, 0, 0, 0);
    m_formGrid->setHorizontalSpacing(kFormColGap);

    m_cardLbl = new ThemedFormLabel(QString(), m_contentFrame);
    styleFormLabel(m_cardLbl, kFormLabelWidthMin);
    m_cardCombo = makeComboBox(m_contentFrame);
    m_cardCombo->setFixedHeight(kFormRowHeight);
    // 表单内宽度由 syncLevelRowLayout 固定；禁止 AdjustToContents 在换肤后按 sizeHint 缩窄
    m_cardCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_cardCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_levelLbl = new ThemedFormLabel(QString(), m_contentFrame);
    styleFormLabel(m_levelLbl, kFormLabelWidthMin);

    auto *levelRow = new QHBoxLayout();
    levelRow->setContentsMargins(0, 0, 0, 0);
    levelRow->setSpacing(kFormRadioSpacing);
    levelRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_lowActive  = makeRadioButton(trKey("motion.io.low_active"), m_contentFrame);
    m_highActive = makeRadioButton(trKey("motion.io.high_active"), m_contentFrame);
    m_lowActive->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_highActive->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto *levelGroup = new QButtonGroup(m_contentFrame);
    levelGroup->setExclusive(true);
    levelGroup->addButton(m_lowActive, 0);
    levelGroup->addButton(m_highActive, 1);
    m_lowActive->setChecked(true);
    levelRow->addWidget(m_lowActive, 0, Qt::AlignVCenter);
    levelRow->addWidget(m_highActive, 0, Qt::AlignVCenter);
    m_levelWrap = new QWidget(m_contentFrame);
    m_levelWrap->setObjectName(QStringLiteral("motionIoLevelWrap"));
    m_levelWrap->setAttribute(Qt::WA_StyledBackground, false);
    m_levelWrap->setAutoFillBackground(false);
    m_levelWrap->setLayout(levelRow);

    m_portLbl = new ThemedFormLabel(QString(), m_contentFrame);
    styleFormLabel(m_portLbl, kFormLabelWidthMin);
    m_portSpin = makeSpinBox(m_contentFrame);
    m_portSpin->setFixedHeight(kFormRowHeight);
    m_portSpin->setRange(0, 999);
    m_portSpin->setAlignment(Qt::AlignCenter);
    m_portSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_extendLbl = new ThemedFormLabel(QString(), m_contentFrame);
    styleFormLabel(m_extendLbl, kFormLabelWidthMin);
    m_extendSwitch = makePillSwitch(false, m_contentFrame);

    m_nodeLbl = new ThemedFormLabel(QString(), m_contentFrame);
    styleFormLabel(m_nodeLbl, kFormLabelWidthMin);
    m_nodeSpin = makeSpinBox(m_contentFrame);
    m_nodeSpin->setFixedHeight(kFormRowHeight);
    m_nodeSpin->setRange(0, 255);
    m_nodeSpin->setAlignment(Qt::AlignCenter);
    m_nodeSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(m_extendSwitch, &PillSwitch::toggled, this, [this](bool) {
        syncExtendNodeVisible();
        syncLevelRowLayout();
    });

    m_formGrid->addWidget(m_cardLbl, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_cardCombo, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_levelLbl, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_levelWrap, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_portLbl, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_portSpin, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_extendLbl, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_extendSwitch, 3, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_nodeLbl, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_formGrid->addWidget(m_nodeSpin, 4, 1, Qt::AlignLeft | Qt::AlignVCenter);

    NoWheelInputGuard::installOn(m_contentFrame);

    contentLay->addLayout(m_formGrid);

    syncExtendNodeVisible();
    syncLevelRowLayout();

    // 背景块按表单宽度收缩，在卡片内水平居中，避免 grid 拉宽卡片后左贴表单、右侧留白
    slotLay->addWidget(m_contentFrame, 0, Qt::AlignHCenter | Qt::AlignTop);
    cardLay->addWidget(m_contentSlot, 0, Qt::AlignTop);
    syncCardLayout();
}

void IoParamFormWidget::syncCardLayout()
{
    if (!m_contentFrame)
        return;

    const QList<ThemedFormLabel *> labels = {m_cardLbl, m_levelLbl, m_portLbl, m_extendLbl, m_nodeLbl};
    const int                       labelWidth =
        calcFormLabelWidth(labels);
    const int idealCtrl =
        calcIdealControlWidth(m_lowActive, m_highActive, m_portSpin, m_nodeSpin, m_cardCombo);

    const int rowCount = (m_extendSwitch && m_extendSwitch->isChecked()) ? kFormBaseRowCount + 1
                                                                         : kFormBaseRowCount;
    const int formBodyH = rowCount * kFormRowHeight + (rowCount - 1) * kFormRowSpacing;
    const int contentH  = formBodyH + 2 * kPadV;

    int outerW = width() > 0 ? width() : (labelWidth + kFormColGap + idealCtrl + 4 * kPadH);
    int contentMaxW = outerW - 2 * kPadH;
    int maxCtrl     = contentMaxW - 2 * kPadH - labelWidth - kFormColGap;
    int ctrlWidth   = idealCtrl;
    if (maxCtrl > 0)
        ctrlWidth = qBound(kCtrlWidthMin, idealCtrl, maxCtrl);

    const int formW    = labelWidth + kFormColGap + ctrlWidth;
    const int contentW = formW + 2 * kPadH;

    m_contentFrame->setFixedSize(contentW, contentH);

    if (m_cardFrame)
        m_cardFrame->setFixedWidth(outerW);

    if (m_contentSlot) {
        const int slotH = contentH + kGapHeader;
        m_contentSlot->setFixedHeight(slotH);
    }

    if (m_cardFrame) {
        const int cardH = kHeaderHeight + (m_contentSlot && m_detailsVisible ? m_contentSlot->height() : 0)
                          + kPadH;
        m_cardFrame->setFixedHeight(cardH);
    }

    setFixedHeight(m_cardFrame ? m_cardFrame->height() : sizeHint().height());
}

void IoParamFormWidget::syncLevelRowLayout()
{
    if (!m_levelWrap || !m_lowActive || !m_highActive)
        return;

    const QList<ThemedFormLabel *> labels = {m_cardLbl, m_levelLbl, m_portLbl, m_extendLbl, m_nodeLbl};
    const int                       labelWidth = calcFormLabelWidth(labels);
    for (ThemedFormLabel *lbl : labels)
        styleFormLabel(lbl, labelWidth);

    const int idealCtrl =
        calcIdealControlWidth(m_lowActive, m_highActive, m_portSpin, m_nodeSpin, m_cardCombo);

    int outerW = width() > 0 ? width() : (labelWidth + kFormColGap + idealCtrl + 4 * kPadH);
    int contentMaxW = outerW - 2 * kPadH;
    int maxCtrl     = contentMaxW - 2 * kPadH - labelWidth - kFormColGap;
    int ctrlWidth   = idealCtrl;
    if (maxCtrl > 0)
        ctrlWidth = qBound(kCtrlWidthMin, idealCtrl, maxCtrl);

    const int rowCount = (m_extendSwitch && m_extendSwitch->isChecked()) ? kFormBaseRowCount + 1
                                                                         : kFormBaseRowCount;
    applyFormGridMetrics(m_formGrid, rowCount);

    m_levelWrap->setFixedSize(ctrlWidth, kFormRowHeight);
    if (m_portSpin)
        m_portSpin->setFixedSize(ctrlWidth, kFormRowHeight);
    if (m_nodeSpin)
        m_nodeSpin->setFixedSize(ctrlWidth, kFormRowHeight);
    if (m_cardCombo)
        m_cardCombo->setFixedSize(ctrlWidth, kFormRowHeight);
    syncCardLayout();
}

void IoParamFormWidget::syncExtendNodeVisible()
{
    const bool showNode = m_extendSwitch && m_extendSwitch->isChecked();
    if (m_nodeLbl)
        m_nodeLbl->setVisible(showNode);
    if (m_nodeSpin)
        m_nodeSpin->setVisible(showNode);
}

void IoParamFormWidget::refreshCardStyle()
{
    if (!m_cardFrame || !m_headerFrame || !m_contentFrame)
        return;

    const ThemeTokens t = ThemeTokens::resolve();

    // 外框仅描边；内容区使用窗体背景色，与 accent 高亮底区分
    refreshSelectionStyle();

    m_headerFrame->setStyleSheet(QStringLiteral(
        "QFrame#motionIoParamHeader {"
        "  background-color: %1; border: none;"
        "  border-top-left-radius: 7px; border-top-right-radius: 7px;"
        "  border-bottom: 1px solid %2;"
        "}")
                                     .arg(t.surfaceElevated.name(), t.surfaceBorder.name()));

    m_contentFrame->setStyleSheet(QStringLiteral(
        "QFrame#motionIoParamContent {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 4px;"
        "}")
                                      .arg(t.background.name()));
}

void IoParamFormWidget::setCardNameOptions(const QStringList &cardNames)
{
    m_cardNames = cardNames;
    bindCardCombo(m_config.cardName);
}

void IoParamFormWidget::bindCardCombo(const QString &current)
{
    if (!m_cardCombo)
        return;

    m_cardCombo->clear();
    QStringList options = m_cardNames;
    if (!current.isEmpty() && !options.contains(current))
        options.prepend(current);
    options.removeDuplicates();
    for (const QString &name : options)
        m_cardCombo->addItem(name, name);

    const int idx = m_cardCombo->findText(current);
    m_cardCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    syncLevelRowLayout();
}

QString IoParamFormWidget::formatPortHeader(bool isOutput, int port)
{
    QString portStr = QString::number(port);
    if (portStr.size() == 1)
        portStr = QStringLiteral(" ") + portStr;
    return (isOutput ? QStringLiteral("DO:") : QStringLiteral("DI:")) + portStr;
}

void IoParamFormWidget::loadConfig(const IoConfigData &config)
{
    m_config = config;
    if (m_titleLbl)
        m_titleLbl->setText(config.name);
    bindCardCombo(config.cardName);
    if (m_portSpin)
        m_portSpin->setValue(config.port);
    if (m_extendSwitch)
        m_extendSwitch->setChecked(config.extend != 0);
    if (m_nodeSpin)
        m_nodeSpin->setValue(config.node);
    syncExtendNodeVisible();
    if (config.logic == 1)
        m_highActive->setChecked(true);
    else
        m_lowActive->setChecked(true);
    updatePortHeader();
    updateHeaderInteraction();
    refreshRuntimeState(false);
    setDetailsVisible(m_detailsVisible);
    syncLevelRowLayout();
}

IoConfigData IoParamFormWidget::collectConfig() const
{
    IoConfigData cfg = m_config;
    if (m_cardCombo)
        cfg.cardName = m_cardCombo->currentText().trimmed();
    if (m_portSpin)
        cfg.port = m_portSpin->value();
    cfg.extend = (m_extendSwitch && m_extendSwitch->isChecked()) ? 1 : 0;
    if (m_nodeSpin)
        cfg.node = m_nodeSpin->value();
    cfg.logic = (m_highActive && m_highActive->isChecked()) ? 1 : 0;
    return cfg;
}

void IoParamFormWidget::setDetailsVisible(bool visible)
{
    m_detailsVisible = visible;
    if (m_contentSlot)
        m_contentSlot->setVisible(visible);
    syncCardLayout();
    updateGeometry();
}

void IoParamFormWidget::setOutputClickEnabled(bool enabled)
{
    m_outputClickEnabled = enabled;
    updateHeaderInteraction();
}

void IoParamFormWidget::setSelected(bool selected)
{
    if (m_selected == selected)
        return;
    m_selected = selected;
    refreshSelectionStyle();
}

void IoParamFormWidget::refreshSelectionStyle()
{
    if (!m_cardFrame)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    const QString borderColor =
        m_selected ? t.accent.name() : t.surfaceBorder.name();
    const QString borderWidth = m_selected ? QStringLiteral("2px") : QStringLiteral("1px");

    m_cardFrame->setStyleSheet(QStringLiteral(
        "QFrame#motionIoParamCard {"
        "  background-color: transparent;"
        "  border: %1 solid %2;"
        "  border-radius: 8px;"
        "}")
                                   .arg(borderWidth, borderColor));
}

void IoParamFormWidget::updateHeaderInteraction()
{
    if (!m_headerFrame)
        return;

    const bool clickable = m_config.isOutput && m_outputClickEnabled;
    m_headerFrame->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    m_headerFrame->setProperty("clickable", clickable);
    m_headerFrame->style()->unpolish(m_headerFrame);
    m_headerFrame->style()->polish(m_headerFrame);
}

void IoParamFormWidget::toggleOutput()
{
    if (!m_service || m_config.name.isEmpty())
        return;

    // 先读后写翻转，不写本地 m_config（运行态由 ioStatesUpdated 刷新指示灯）
    bool value = false;
    if (!m_service->readIo(m_config.name, &value))
        return;
    m_service->writeIo(m_config.name, !value);
}

void IoParamFormWidget::refreshRuntimeState(bool active)
{
    if (!m_stateDot)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    m_stateDot->setAccentColor(active ? t.successText : t.textSecondary);
}

void IoParamFormWidget::updatePortHeader()
{
    if (!m_portHeaderLbl)
        return;
    const int port = m_portSpin ? m_portSpin->value() : m_config.port;
    m_portHeaderLbl->setText(formatPortHeader(m_config.isOutput, port));
}

void IoParamFormWidget::retranslateUi()
{
    if (m_cardLbl)
        m_cardLbl->setText(trKey("motion.io.card_name"));
    if (m_levelLbl)
        m_levelLbl->setText(trKey("motion.io.active_level"));
    if (m_lowActive)
        m_lowActive->setText(trKey("motion.io.low_active"));
    if (m_highActive)
        m_highActive->setText(trKey("motion.io.high_active"));
    if (m_portLbl)
        m_portLbl->setText(trKey("motion.io.port"));
    if (m_extendLbl)
        m_extendLbl->setText(trKey("motion.io.extend_card"));
    if (m_nodeLbl)
        m_nodeLbl->setText(trKey("motion.io.extend_node"));
    if (m_stateDot)
        m_stateDot->setToolTip(trKey("motion.io.state_hint"));
    syncLevelRowLayout();
}

bool IoParamFormWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_headerFrame && event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (m_config.isOutput && m_outputClickEnabled) {
                toggleOutput();
                return true;
            }
            emit cardSelected();
            return false;
        }
    }

    if ((watched == m_cardFrame || watched == m_contentFrame) && event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
            emit cardSelected();
    }

    return QWidget::eventFilter(watched, event);
}

void IoParamFormWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncLevelRowLayout();
}

void IoParamFormWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace motion_control_plugin
