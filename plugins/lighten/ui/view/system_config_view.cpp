/**
 * @file system_config_view.cpp
 * @brief SystemConfigView 四象限布局、控件绑定与保存提示实现
 */

#include "system_config_view.h"

#include <QtGlobal>

#include "system_config_view_model.h"
#include "lightenplugin.h"

#include "messagecenter/message_center.h"
#include "databinding.h"
#include "uibase/thememanager.h"

#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOptionSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QFrame>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPolygon>
#include <QMarginsF>
#include <QRectF>
#include <QSizePolicy>
#include <functional>

/**
 * @brief 胶囊轨开关（左关 / 右开，自绘白圆滑块）
 *
 * 目标图样式：胶囊轨 + 白圆滑块，不借用 CheckBox 等语义无关控件。
 */
class CfgPillSwitch final : public QWidget
{
public:
    explicit CfgPillSwitch(bool checked, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_checked(checked)
    {
        setFixedSize(52, 26);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
    }

    bool isChecked() const { return m_checked; }
    void setChecked(bool on)
    {
        if (m_checked == on)
            return;
        m_checked = on;
        update();
    }

    void setOnToggled(std::function<void(bool)> cb) { m_onToggled = std::move(cb); }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        if (!isEnabled())
            p.setOpacity(0.42);

        const QRectF outer = QRectF(rect());
        const QRectF r     = outer.adjusted(1.0, 1.0, -1.0, -1.0);
        const qreal  rad   = qMax(1.0, r.height() / 2.0);

        const QColor borderOff(0x4a, 0x60, 0x78);
        const QColor borderOn(0x64, 0xb5, 0xf6);
        const QColor offTrack(0x1e, 0x2d, 0x40);
        /** 与 recipe_config_view RecipePillSwitch / #recipeToolbarActionBtn 一致 */
        const QColor onTrack(0x1a, 0x4a, 0x8c);

        QPen pen(m_checked ? borderOn : borderOff, 1.0);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(m_checked ? onTrack : offTrack);
        p.drawRoundedRect(r, rad, rad);

        // 白圆相对「轨槽内腔」上下对称：内腔按笔画宽度内缩，再用 qreal 求垂直中点（避免 QPoint::center 取整 + 笔画偏心）
        const qreal    strokeW = pen.widthF();
        const QRectF   inner   = r.marginsRemoved(QMarginsF(strokeW * 0.5, strokeW * 0.5, strokeW * 0.5, strokeW * 0.5));
        const qreal    padV    = 3.0;
        const qreal    thumbD = qMax(8.0, inner.height() - 2.0 * padV);
        const qreal    tr     = thumbD * 0.5;
        const qreal    cy     = inner.top() + inner.height() * 0.5;
        const qreal    padH   = 3.0;
        const qreal    cx     = m_checked ? (inner.right() - padH - tr) : (inner.left() + padH + tr);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xf5, 0xf8, 0xfc));
        p.drawEllipse(QPointF(cx, cy), tr, tr);
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && isEnabled() && rect().contains(e->pos())) {
            m_checked = !m_checked;
            update();
            if (m_onToggled)
                m_onToggled(m_checked);
        }
        QWidget::mouseReleaseEvent(e);
    }

    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::EnabledChange)
            setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        QWidget::changeEvent(event);
    }

    QSize sizeHint() const override { return QSize(52, 26); }
    QSize minimumSizeHint() const override { return QSize(52, 26); }

private:
    bool                      m_checked;
    std::function<void(bool)> m_onToggled;
};

/** 与 recipe_config_view PreviewToolBarComboBox 箭头区一致：三角 + #152a3d 底 */
static void paintCfgComboLikeSpinArrows(QAbstractSpinBox *spin, const QStyleOptionSpinBox &opt)
{
    if (spin->buttonSymbols() == QAbstractSpinBox::NoButtons)
        return;

    QPainter p(spin);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor arrow = spin->isEnabled() ? QColor(Qt::white) : QColor(0x7a, 0x88, 0x99);

    auto drawOne = [&](QStyle::SubControl sc, bool up) {
        const QRect r = spin->style()->subControlRect(QStyle::CC_SpinBox, &opt, sc, spin);
        if (!r.isValid())
            return;
        const QRect fill = r.adjusted(1, 1, -2, -2);
        if (fill.width() <= 0 || fill.height() <= 0)
            return;
        constexpr QRgb kFieldBg = qRgb(0x15, 0x2a, 0x3d);
        p.fillRect(fill, QColor::fromRgb(kFieldBg));
        const int cx = r.center().x();
        const int cy = r.center().y() + (up ? 0 : 1);
        QPolygon tri;
        if (up)
            tri << QPoint(cx, cy - 3) << QPoint(cx - 4, cy + 2) << QPoint(cx + 4, cy + 2);
        else
            tri << QPoint(cx - 4, cy - 3) << QPoint(cx + 4, cy - 3) << QPoint(cx, cy + 3);
        p.setPen(Qt::NoPen);
        p.setBrush(arrow);
        p.drawPolygon(tri);
    };

    drawOne(QStyle::SC_SpinBoxUp, true);
    drawOne(QStyle::SC_SpinBoxDown, false);
}

/** @brief 与配方页风格一致的 SpinBox（Fusion + 下拉式箭头绘制） */
class CfgComboStyleSpinBox final : public QSpinBox
{
public:
    explicit CfgComboStyleSpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        if (QStyle *fus = QStyleFactory::create(QStringLiteral("Fusion")))
            setStyle(fus);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QSpinBox::paintEvent(event);
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        paintCfgComboLikeSpinArrows(this, opt);
    }
};

/** SystemConfigView 本地控件样式与布局辅助 */
namespace {

/** 仅用于 cfgAccentBtn：压矮按钮以贴近默认行高的输入框/Spin，不拉高 LineEdit/Spin */
constexpr int kCfgAccentBtnH = 26;
/** 切屏拍照间隔延时 / 当前次数：同列同宽 */
constexpr int kSwitchDelayCurCountSpinW = 120;

QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}


QScrollArea *scrollWrap(QWidget *content)
{
    auto *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setWidget(content);
    return sa;
}

/** 与 help_view 思路一致：GroupBox 无标题字，区域标题放在内部的 QLabel。 */
struct CfgSectionShell {
    QGroupBox *shell = nullptr;
    QWidget   *body  = nullptr;
};

CfgSectionShell makeCfgSection(const QString &title, QWidget *parent)
{
    CfgSectionShell out;
    out.shell = new QGroupBox(parent);
    out.shell->setObjectName(QStringLiteral("cfgSection"));
    out.shell->setTitle(QString());
    out.shell->setAttribute(Qt::WA_StyledBackground, true);
    out.shell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *outer = new QVBoxLayout(out.shell);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    auto *head = new QLabel(title, out.shell);
    head->setObjectName(QStringLiteral("cfgSectionTitle"));
    QFont hf = head->font();
    hf.setBold(true);
    hf.setPointSizeF(hf.pointSizeF() + 1.0);
    head->setFont(hf);
    outer->addWidget(head);

    out.body = new QWidget(out.shell);
    out.body->setObjectName(QStringLiteral("cfgSectionBody"));
    out.body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    outer->addWidget(out.body, 1);
    return out;
}

CfgPillSwitch *makeSwitch(bool checked, QWidget *parent)
{
    return new CfgPillSwitch(checked, parent);
}

template<typename Getter, typename Setter>
void bindConfigPillSwitch(ViewModel *vm, const QString &prop, Getter get, Setter set, CfgPillSwitch *sw)
{
    if (!sw || !vm || !vm->hasProperty(prop))
        return;

    using SC = lighten::SystemConfig;

    sw->setChecked(get(vm->getProperty(prop).value<SC>()));
    QObject::connect(vm, &ViewModel::propertyChanged, sw,
                     [sw, prop, get](const QString &name, const QVariant &val) {
                         if (name == prop)
                             sw->setChecked(get(val.value<SC>()));
                     });
    sw->setOnToggled([vm, prop, set](bool checked) {
        SC cfg = vm->getProperty(prop).value<SC>();
        set(cfg, checked);
        vm->setProperty(prop, QVariant::fromValue(cfg));
    });
}

using DetectRowSwitchList = QVector<lighten::DetectRowSwitches>;

void applyDetectRowRgbUi(const lighten::SystemConfig &cfg, int row, const lighten::DetectRowSwitches &sw)
{
    if (row >= cfg.detectItems.size() || !cfg.detectItems[row].hasRgbColumns)
        return;

    const lighten::DetectItemColorSwitches &item = cfg.detectItems[row];
    const bool interactive = item.masterEnabled;

    auto sync = [&](CfgPillSwitch *pill, bool on) {
        if (!pill)
            return;
        pill->setEnabled(interactive);
        pill->blockSignals(true);
        pill->setChecked(interactive && on);
        pill->blockSignals(false);
    };
    sync(sw.red, item.redEnabled);
    sync(sw.green, item.greenEnabled);
    sync(sw.blue, item.blueEnabled);
}

bool clampDetectRgbWhenMasterOff(lighten::SystemConfig &cfg)
{
    bool changed = false;
    for (lighten::DetectItemColorSwitches &item : cfg.detectItems) {
        if (!item.hasRgbColumns || item.masterEnabled)
            continue;
        if (item.redEnabled || item.greenEnabled || item.blueEnabled) {
            item.redEnabled = item.greenEnabled = item.blueEnabled = false;
            changed          = true;
        }
    }
    return changed;
}

/** 检测项表：总开关关时 RGB 强制关且不可点；总开关开时 RGB 全部打开且可手动切换（无 RGB 列不处理） */
void bindDetectItemSwitches(ViewModel *vm, const QString &prop, const DetectRowSwitchList &rows)
{
    if (!vm || !vm->hasProperty(prop))
        return;

    using SC = lighten::SystemConfig;

    auto syncFromConfig = [vm, prop, &rows](const SC &cfgIn) {
        SC cfg = cfgIn;
        if (clampDetectRgbWhenMasterOff(cfg)) {
            vm->setProperty(prop, QVariant::fromValue(cfg));
            return;
        }
        for (int row = 0; row < rows.size(); ++row) {
            const lighten::DetectRowSwitches &sw = rows[row];
            if (sw.master) {
                const bool on = row < cfg.detectItems.size() ? cfg.detectItems[row].masterEnabled : true;
                sw.master->blockSignals(true);
                sw.master->setChecked(on);
                sw.master->blockSignals(false);
            }
            applyDetectRowRgbUi(cfg, row, sw);
        }
    };

    syncFromConfig(vm->getProperty(prop).value<SC>());

    QObject::connect(vm, &ViewModel::propertyChanged, vm,
                     [syncFromConfig, prop](const QString &name, const QVariant &val) {
                         if (name == prop)
                             syncFromConfig(val.value<SC>());
                     });

    for (int row = 0; row < rows.size(); ++row) {
        const lighten::DetectRowSwitches &sw = rows[row];
        if (sw.master) {
            sw.master->setOnToggled([vm, prop, row](bool checked) {
                SC cfg = vm->getProperty(prop).value<SC>();
                if (row >= cfg.detectItems.size())
                    return;
                lighten::DetectItemColorSwitches &item = cfg.detectItems[row];
                item.masterEnabled                       = checked;
                if (item.hasRgbColumns)
                    item.redEnabled = item.greenEnabled = item.blueEnabled = checked;
                vm->setProperty(prop, QVariant::fromValue(cfg));
            });
        }
        auto bindColor = [vm, prop, row](CfgPillSwitch *pill, bool lighten::DetectItemColorSwitches::*member) {
            if (!pill)
                return;
            pill->setOnToggled([vm, prop, row, pill, member](bool checked) {
                SC cfg = vm->getProperty(prop).value<SC>();
                if (row >= cfg.detectItems.size())
                    return;
                lighten::DetectItemColorSwitches &item = cfg.detectItems[row];
                if (!item.hasRgbColumns || !item.masterEnabled) {
                    pill->blockSignals(true);
                    pill->setChecked(false);
                    pill->blockSignals(false);
                    return;
                }
                item.*member = checked;
                vm->setProperty(prop, QVariant::fromValue(cfg));
            });
        };
        bindColor(sw.red, &lighten::DetectItemColorSwitches::redEnabled);
        bindColor(sw.green, &lighten::DetectItemColorSwitches::greenEnabled);
        bindColor(sw.blue, &lighten::DetectItemColorSwitches::blueEnabled);
    }
}

template<typename Getter, typename Setter>
void bindThresholdSpin(ViewModel *vm, const QString &prop, Getter get, Setter set, QSpinBox *spin)
{
    if (!spin || !vm || !vm->hasProperty(prop))
        return;

    using SC = lighten::SystemConfig;

    spin->setValue(get(vm->getProperty(prop).value<SC>()));
    QObject::connect(vm, &ViewModel::propertyChanged, spin,
                     [spin, prop, get](const QString &name, const QVariant &val) {
                         if (name == prop)
                             spin->setValue(get(val.value<SC>()));
                     });
    QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), vm,
                     [vm, prop, set](int value) {
                         SC cfg = vm->getProperty(prop).value<SC>();
                         set(cfg, value);
                         vm->setProperty(prop, QVariant::fromValue(cfg));
                     });
}

/** 检测表开关单元格：占满单元格后居中，避免单层 QHBoxLayout 高度塌成 ~26px 裁切底部 */
QWidget *makeDetectTableSwitchCell(QWidget *parent, CfgPillSwitch **switchOut = nullptr)
{
    auto *cell = new QWidget(parent);
    cell->setAttribute(Qt::WA_TranslucentBackground, true);
    cell->setAutoFillBackground(false);
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *v = new QVBoxLayout(cell);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    v->addStretch(1);
    auto *h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addStretch(1);
    auto *sw = makeSwitch(false, cell);
    if (switchOut)
        *switchOut = sw;
    h->addWidget(sw, 0, Qt::AlignVCenter);
    h->addStretch(1);
    v->addLayout(h);
    v->addStretch(1);
    return cell;
}

QWidget *makeLabeledSwitchRow(const QString &label, QWidget *parent, CfgPillSwitch **switchOut = nullptr)
{
    auto *row = new QWidget(parent);
    row->setObjectName(QStringLiteral("cfgSwitchItem"));
    row->setAttribute(Qt::WA_StyledBackground, true);
    auto *rl  = new QHBoxLayout(row);
    rl->setContentsMargins(6, 3, 6, 3);
    rl->setSpacing(10);
    auto *lbl = new QLabel(label, row);
    lbl->setObjectName(QStringLiteral("cfgMutedLabel"));
    lbl->setWordWrap(false);
    rl->addWidget(lbl, 1);
    auto *sw = makeSwitch(false, row);
    if (switchOut)
        *switchOut = sw;
    rl->addWidget(sw, 0, Qt::AlignRight);
    return row;
}

QWidget *makePathRow(const QString &label, QWidget *parent, QLineEdit **pathEditOut = nullptr)
{
    auto *row = new QWidget(parent);
    row->setObjectName(QStringLiteral("cfgSettingRow"));
    auto *rl  = new QHBoxLayout(row);
    rl->setContentsMargins(0, 6, 0, 6);
    rl->setSpacing(10);

    auto *lbl = new QLabel(label + QStringLiteral(":"), row);
    lbl->setFixedWidth(132);
    lbl->setObjectName(QStringLiteral("cfgFormLabel"));

    auto *edit = new QLineEdit(row);
    if (pathEditOut)
        *pathEditOut = edit;
    edit->setObjectName(QStringLiteral("cfgLineEdit"));
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *btn = new QPushButton(QStringLiteral("选择文件夹"), row);
    btn->setObjectName(QStringLiteral("cfgAccentBtn"));
    btn->setFixedWidth(104);
    btn->setFixedHeight(kCfgAccentBtnH);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(btn, &QPushButton::clicked, row, [edit, row] {
        const QString dir = QFileDialog::getExistingDirectory(row, QStringLiteral("选择文件夹"),
                                                                edit->text().isEmpty() ? QString() : edit->text());
        if (!dir.isEmpty())
            edit->setText(dir);
    });

    rl->addWidget(lbl);
    rl->addWidget(edit, 1);
    rl->addWidget(btn, 0, Qt::AlignVCenter);
    return row;
}

QWidget *makeThresholdBlock(const QString &name, QWidget *parent, QSpinBox **lowerOut = nullptr,
                            QSpinBox **upperOut = nullptr)
{
    auto *block = new QWidget(parent);
    block->setObjectName(QStringLiteral("cfgThresholdItem"));
    block->setAttribute(Qt::WA_StyledBackground, true);
    auto *hb = new QHBoxLayout(block);
    hb->setContentsMargins(10, 8, 10, 8);
    hb->setSpacing(8);

    auto *nl = new QLabel(name, block);
    nl->setMinimumWidth(72);
    nl->setObjectName(QStringLiteral("cfgFormLabel"));

    auto *lbLo = new QLabel(QStringLiteral("下限"), block);
    lbLo->setObjectName(QStringLiteral("cfgMutedLabel"));
    auto *spLo = new CfgComboStyleSpinBox(block);
    spLo->setRange(0, 9999);
    spLo->setObjectName(QStringLiteral("cfgSpin"));
    if (lowerOut)
        *lowerOut = spLo;

    auto *lbHi = new QLabel(QStringLiteral("上限"), block);
    lbHi->setObjectName(QStringLiteral("cfgMutedLabel"));
    auto *spHi = new CfgComboStyleSpinBox(block);
    spHi->setRange(0, 9999);
    spHi->setObjectName(QStringLiteral("cfgSpin"));
    if (upperOut)
        *upperOut = spHi;

    hb->addWidget(nl);
    hb->addStretch(1);
    hb->addWidget(lbLo);
    hb->addWidget(spLo);
    hb->addSpacing(6);
    hb->addWidget(lbHi);
    hb->addWidget(spHi);
    return block;
}

} // namespace

/** @brief SystemConfigView 成员实现（四象限布局、数据绑定与保存提示） */
namespace lighten {

SystemConfigView::SystemConfigView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("lighten.system_config"));
    setPageTitle(QStringLiteral("系统配置"));
}

void SystemConfigView::applyMainStylesheet()
{
    // 区域外框 = secondaryLightColor；表格/输入/Spin/判定项等边框与之同源（与 QGroupBox#cfgSection %3 一致）
    const QString kSectionBg = QStringLiteral("#17273b");
    const QString kPanelBorder =
        themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668"))).name();
    const QString kBorder = kPanelBorder;

    setStyleSheet(QStringLiteral(
        "#lightenSystemConfigRoot {"
        "  background-color: #151c26;"
        "}"

        "#lightenSystemConfigRoot QWidget#cfgSettingRow {"
        "  background-color: transparent;"
        "}"

        "#lightenSystemConfigRoot QWidget#cfgThresholdItem {"
        "  background-color: transparent;"
        "  border: 1px solid %1;"
        "  border-radius: 6px;"
        "}"

        "#lightenSystemConfigRoot QWidget#cfgSwitchItem {"
        "  background-color: transparent;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "}"

        "#lightenSystemConfigRoot QLabel { color: #e8eef6; font-size: 13px; background-color: transparent; }"
        "#lightenSystemConfigRoot QLabel#cfgFormLabel { color: #f0f6ff; font-size: 13px; background-color: transparent; }"
        "#lightenSystemConfigRoot QLabel#cfgMutedLabel { color: #a0aab7; font-size: 13px; background-color: transparent; }"
        "#lightenSystemConfigRoot QLabel#cfgSubCaption { color: #cfe6ff; font-size: 13px; font-weight: 700; background-color: transparent; }"

        "#lightenSystemConfigRoot QGroupBox#cfgSection {"
        "  background-color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  margin-top: 0px;"
        "  padding: 0px;"
        "}"
        "#lightenSystemConfigRoot QLabel#cfgSectionTitle {"
        "  color: #ffffff;"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  background-color: transparent;"
        "}"
        "#lightenSystemConfigRoot QWidget#cfgSectionBody {"
        "  background-color: transparent;"
        "}"
        "#lightenSystemConfigRoot QWidget#cfgThresholdCol {"
        "  background-color: transparent;"
        "}"

        "#lightenSystemConfigRoot QLineEdit#cfgLineEdit {"
        "  background-color: transparent;"
        "  color: #e8eef6;"
        "  border: 1px solid %1;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"

        "#lightenSystemConfigRoot QPushButton#cfgAccentBtn {"
        "  background-color: #184c86;"
        "  color: #ffffff;"
        "  border: 1px solid #2f82da;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "  padding: 0px 10px;"
        "  min-height: %4px;"
        "  max-height: %4px;"
        "}"
        "#lightenSystemConfigRoot QPushButton#cfgAccentBtn:hover { background-color: #1f5ea2; }"
        "#lightenSystemConfigRoot QPushButton#cfgAccentBtn:pressed { background-color: #17477b; }"

        "#lightenSystemConfigRoot QSpinBox#cfgSpin {"
        "  background-color: #15263d;"
        "  color: #e8eef6;"
        "  border: 1px solid %1;"
        "  border-radius: 4px;"
        "  padding: 2px 22px 2px 6px;"
        "  min-height: 24px;"
        "  min-width: 46px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "#lightenSystemConfigRoot QSpinBox#cfgSpin[noBtn=\"true\"] {"
        "  padding: 2px 6px 2px 6px;"
        "}"
        "#lightenSystemConfigRoot QSpinBox#cfgSpin::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 16px;"
        "  height: 12px;"
        "  border: none;"
        "  background-color: transparent;"
        "}"
        "#lightenSystemConfigRoot QSpinBox#cfgSpin::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 16px;"
        "  height: 12px;"
        "  border: none;"
        "  background-color: transparent;"
        "}"

        "#lightenSystemConfigRoot QTableWidget#cfgDetectTable {"
        "  background-color: transparent;"
        "  alternate-background-color: transparent;"
        "  color: #ffffff;"
        "  gridline-color: %1;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "#lightenSystemConfigRoot QWidget#cfgDetectTableWrap {"
        "  background-color: %2;"
        "}"
        "#lightenSystemConfigRoot QWidget#cfgDetectTableViewport {"
        "  background-color: %2;"
        "}"
        "#lightenSystemConfigRoot QTableWidget#cfgDetectTable QHeaderView {"
        "  background-color: %2;"
        "  border: none;"
        "  border-top-left-radius: 0px;"
        "  border-top-right-radius: 0px;"
        "  border-bottom-left-radius: 0px;"
        "  border-bottom-right-radius: 0px;"
        "}"
        "#lightenSystemConfigRoot QTableWidget#cfgDetectTable::item {"
        "  padding: 4px 4px;"
        "  background-color: transparent;"
        "}"
        "#lightenSystemConfigRoot QHeaderView::section {"
        "  background-color: rgba(36,54,82,0.85);"
        "  color: #ffffff;"
        "  padding: 8px 6px;"
        "  border: none;"
        "  border-right: 1px solid %1;"
        "  border-bottom: 1px solid %1;"
        "  font-weight: bold;"
        "}"
        "#lightenSystemConfigRoot QTableWidget#cfgDetectTable QHeaderView::section {"
        "  padding: 3px 4px;"
        "  background-color: rgba(36,54,82,0.85);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-top: 1px solid %1;"
        "  border-right: 1px solid %1;"
        "  border-bottom: 1px solid %1;"
        "  font-weight: bold;"
        "  border-top-left-radius: 0px;"
        "  border-top-right-radius: 0px;"
        "}"

        "#lightenSystemConfigRoot QScrollArea { background: transparent; border: none; }"
        "#lightenSystemConfigRoot QWidget#cfgScrollContents { background: transparent; }"
        /* cfgMainPanel 背景/边框由 applyMainPanelStyle() 同步 */
        "#lightenSystemConfigRoot QWidget#cfgAnchorSwitch, #lightenSystemConfigRoot QWidget#cfgRowBottom {"
        "  background-color: transparent;"
        "}"

        "#lightenSystemConfigRoot QScrollBar:vertical { background: #151c26; width: 10px; margin: 0; }"
        "#lightenSystemConfigRoot QScrollBar::handle:vertical {"
        "  background: #3a4a5d; min-height: 28px; border-radius: 4px;"
        "}"
        "#lightenSystemConfigRoot QScrollBar:horizontal { background: #151c26; height: 10px; margin: 0; }"
        "#lightenSystemConfigRoot QScrollBar::handle:horizontal {"
        "  background: #3a4a5d; min-width: 28px; border-radius: 4px;"
        "}"
    ).arg(kBorder, kSectionBg, kPanelBorder, QString::number(kCfgAccentBtnH)));
}

void SystemConfigView::applyMainPanelStyle()
{
    if (!m_mainPanel)
        return;

    const QColor panelBg     = themeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a")));
    const QColor panelBorder = themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668")));

    m_mainPanel->setStyleSheet(QStringLiteral(
        "QWidget#cfgMainPanel {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(panelBg.name(), panelBorder.name()));
}

void SystemConfigView::applyTheme()
{
    setObjectName(QStringLiteral("lightenSystemConfigRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    applyMainStylesheet();
    applyMainPanelStyle();
    for (QSpinBox *sb : findChildren<QSpinBox *>(QStringLiteral("cfgSpin"))) {
        sb->setStyle(nullptr);
        if (dynamic_cast<CfgComboStyleSpinBox *>(sb)) {
            if (QStyle *fus = QStyleFactory::create(QStringLiteral("Fusion")))
                sb->setStyle(fus);
        }
    }
}

void SystemConfigView::onLoad()
{
    setObjectName(QStringLiteral("lightenSystemConfigRoot"));
    setAttribute(Qt::WA_StyledBackground, true);

    connect(TM, &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        applyMainPanelStyle();
    });

    auto *content = new QWidget;
    content->setObjectName(QStringLiteral("cfgScrollContents"));

    auto *scrollOuterLay = new QVBoxLayout(content);
    scrollOuterLay->setContentsMargins(0, 0, 0, 0);
    scrollOuterLay->setSpacing(0);

    auto *mainPanel = new QWidget(content);
    mainPanel->setObjectName(QStringLiteral("cfgMainPanel"));
    mainPanel->setAttribute(Qt::WA_StyledBackground, true);
    m_mainPanel = mainPanel;

    auto *panelLay = new QVBoxLayout(mainPanel);
    panelLay->setContentsMargins(14, 14, 14, 14);
    panelLay->setSpacing(14);

    auto *rowTopWidget = new QWidget(mainPanel);
    rowTopWidget->setObjectName(QStringLiteral("cfgAnchorSwitch"));
    auto *rowTop = new QHBoxLayout(rowTopWidget);
    rowTop->setContentsMargins(0, 0, 0, 0);
    rowTop->setSpacing(14);

    // ── 左上：一、开关设置 ─────────────────────────────────────────────
    const auto secTL  = makeCfgSection(QStringLiteral("一、开关设置"), rowTopWidget);
    auto      *topLeft = secTL.shell;
    auto      *tlBody = secTL.body;
    auto      *tlMain = new QVBoxLayout(tlBody);
    tlMain->setSpacing(2);

    auto *genCap = new QLabel(QStringLiteral("通用开关"), tlBody);
    genCap->setObjectName(QStringLiteral("cfgSubCaption"));
    tlMain->addWidget(genCap);

    auto *genGrid = new QGridLayout();
    genGrid->setHorizontalSpacing(28);
    genGrid->setVerticalSpacing(6);
    genGrid->setColumnStretch(0, 1);
    genGrid->setColumnStretch(1, 1);
    // 两列排布五开关（对齐稿图疏密）
    tlMain->addLayout(genGrid);
    genGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("是否扫码"), tlBody, &m_scanBarcodeSwitch), 0, 0);
    genGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("是否保存原图"), tlBody, &m_saveRawImageSwitch), 0, 1);
    genGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("是否保存结果图"), tlBody, &m_saveResultImageSwitch), 1, 0);
    genGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("是否保存算法过程数据"), tlBody, &m_saveAlgoDataSwitch),
                       1, 1);
    genGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("算法检测失败告警"), tlBody, &m_algoFailAlarmSwitch),
                       2, 0);

    auto *ifCap = new QLabel(QStringLiteral("对接开关"), tlBody);
    ifCap->setObjectName(QStringLiteral("cfgSubCaption"));
    tlMain->addSpacing(4);
    tlMain->addWidget(ifCap);

    // 与通用开关同一套两列网格，列宽与水平间距一致，保证上游/下游 switch 与上面对齐
    auto *ifGrid = new QGridLayout();
    ifGrid->setContentsMargins(0, 0, 0, 0);
    ifGrid->setHorizontalSpacing(28);
    ifGrid->setVerticalSpacing(6);
    ifGrid->setColumnStretch(0, 1);
    ifGrid->setColumnStretch(1, 1);
    ifGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("上游对接"), tlBody, &m_upstreamSwitch), 0, 0);
    ifGrid->addWidget(makeLabeledSwitchRow(QStringLiteral("下游对接"), tlBody, &m_downstreamSwitch), 0, 1);
    tlMain->addLayout(ifGrid);
    tlMain->addStretch();

    // ── 右上：检测项开关 ───────────────────────────────────────────────
    const auto secTR   = makeCfgSection(QStringLiteral("检测项开关（含颜色区分）"), rowTopWidget);
    auto      *topRight = secTR.shell;
    auto      *trBody   = secTR.body;
    auto      *trLay    = new QVBoxLayout(trBody);

    const QStringList items = {QStringLiteral("死灯"), QStringLiteral("暗灯"), QStringLiteral("亮灯"),
                               QStringLiteral("串色"), QStringLiteral("串灯"), QStringLiteral("混色"),
                               QStringLiteral("黑屏检测"), QStringLiteral("黑屏亮灯"), QStringLiteral("Flash")};
    constexpr int kDetectRowH = 40;

    // 左侧 1px 线；顶边画在表头 section 上，避免单独一行与表头之间的缝和黑底
    auto *tableWrap = new QWidget(trBody);
    tableWrap->setObjectName(QStringLiteral("cfgDetectTableWrap"));
    tableWrap->setAttribute(Qt::WA_StyledBackground, true);
    auto *wrapRow = new QHBoxLayout(tableWrap);
    wrapRow->setContentsMargins(0, 0, 0, 0);
    wrapRow->setSpacing(0);
    const QString detectLineColor =
        themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668"))).name();
    const QString detectLineStyle =
        QStringLiteral("background-color: %1; border: none;").arg(detectLineColor);
    auto *leftLine = new QWidget(tableWrap);
    leftLine->setFixedWidth(1);
    leftLine->setAttribute(Qt::WA_StyledBackground, true);
    leftLine->setStyleSheet(detectLineStyle);

    auto *table = new QTableWidget(items.size(), 5, tableWrap);
    table->setObjectName(QStringLiteral("cfgDetectTable"));
    table->viewport()->setObjectName(QStringLiteral("cfgDetectTableViewport"));
    table->viewport()->setAttribute(Qt::WA_StyledBackground, true);
    table->setFrameShape(QFrame::NoFrame);
    table->setHorizontalHeaderLabels({QStringLiteral("检测项"), QStringLiteral("红"), QStringLiteral("绿"),
                                      QStringLiteral("蓝"), QStringLiteral("总开关")});
    if (QTableWidgetItem *h0 = table->horizontalHeaderItem(0))
        h0->setTextAlignment(Qt::AlignCenter);
    for (int c = 0; c < 5; ++c)
        table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);

    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->verticalHeader()->setDefaultSectionSize(kDetectRowH);
    table->horizontalHeader()->setFixedHeight(kDetectRowH);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setShowGrid(true);

    const QSet<int> noColorRows = {6, 7, 8};
    m_detectRowSwitches.resize(items.size());
    for (int r = 0; r < items.size(); ++r) {
        table->setRowHeight(r, kDetectRowH);
        auto *nameItem = new QTableWidgetItem(items[r]);
        nameItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(r, 0, nameItem);
        for (int c = 1; c <= 3; ++c) {
            if (noColorRows.contains(r)) {
                auto *dash = new QTableWidgetItem(QStringLiteral("—"));
                dash->setTextAlignment(Qt::AlignCenter);
                dash->setForeground(QColor(QStringLiteral("#7a8aa0")));
                table->setItem(r, c, dash);
            } else {
                CfgPillSwitch **target = c == 1 ? &m_detectRowSwitches[r].red
                                                 : (c == 2 ? &m_detectRowSwitches[r].green
                                                           : &m_detectRowSwitches[r].blue);
                table->setCellWidget(r, c, makeDetectTableSwitchCell(table, target));
            }
        }
        table->setCellWidget(r, 4, makeDetectTableSwitchCell(table, &m_detectRowSwitches[r].master));
    }

    const int nRows     = items.size();
    // 勿再加 qMin/qMax 的“余量”，否则视口高于表头+各行之和，竖线会画到最下横线之下
    const int tablePixH = kDetectRowH + nRows * kDetectRowH + 2 * table->frameWidth() - 2;
    table->setFixedHeight(tablePixH);
    leftLine->setFixedHeight(tablePixH);

    wrapRow->addWidget(leftLine);
    wrapRow->addWidget(table, 1);

    trLay->addWidget(tableWrap);

    rowTop->addWidget(topLeft, 1);
    rowTop->addWidget(topRight, 1);
    panelLay->addWidget(rowTopWidget, 1);

    auto *rowBottom = new QWidget(mainPanel);
    rowBottom->setObjectName(QStringLiteral("cfgRowBottom"));
    auto *rowBotLay = new QHBoxLayout(rowBottom);
    rowBotLay->setContentsMargins(0, 0, 0, 0);
    rowBotLay->setSpacing(14);

    // ── 左下：二、通用设置 ─────────────────────────────────────────────
    const auto secBL  = makeCfgSection(QStringLiteral("二、通用设置"), rowBottom);
    auto      *botLeft = secBL.shell;
    auto      *blBody  = secBL.body;
    auto      *blLay   = new QVBoxLayout(blBody);
    blLay->setSpacing(4);

    blLay->addWidget(
        makePathRow(QStringLiteral("图像存储路径"), blBody, &m_imagePathEdit));
    blLay->addWidget(
        makePathRow(QStringLiteral("结果数据存储路径"), blBody, &m_resultPathEdit));
    blLay->addWidget(
        makePathRow(QStringLiteral("第三方软件路径"), blBody, &m_thirdPartyPathEdit));

    // QGridLayout：第 2 列 / 第 4 列 Spin 上下对齐；列宽由同列控件统一
    auto *genParam = new QWidget(blBody);
    genParam->setObjectName(QStringLiteral("cfgSettingRow"));
    auto *g = new QGridLayout(genParam);
    g->setContentsMargins(0, 6, 0, 6);
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(6);

    constexpr int kGenLblW  = 176;
    constexpr int kGenSpinW = 96;

    auto *lbLight = new QLabel(QStringLiteral("点亮打屏间隔延时(ms):"), genParam);
    lbLight->setFixedWidth(kGenLblW);
    lbLight->setObjectName(QStringLiteral("cfgFormLabel"));
    m_lightDelaySpin = new CfgComboStyleSpinBox(genParam);
    m_lightDelaySpin->setRange(0, 999999);
    m_lightDelaySpin->setFixedWidth(kGenSpinW);
    m_lightDelaySpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_lightDelaySpin->setObjectName(QStringLiteral("cfgSpin"));

    auto *lbSw = new QLabel(QStringLiteral("切屏拍照间隔延时(ms):"), genParam);
    lbSw->setFixedWidth(kGenLblW);
    lbSw->setObjectName(QStringLiteral("cfgFormLabel"));
    m_switchDelaySpin = new CfgComboStyleSpinBox(genParam);
    m_switchDelaySpin->setRange(0, 999999);
    m_switchDelaySpin->setFixedWidth(kSwitchDelayCurCountSpinW);
    m_switchDelaySpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_switchDelaySpin->setObjectName(QStringLiteral("cfgSpin"));

    auto *lbMax = new QLabel(QStringLiteral("新件使用寿命次数:"), genParam);
    lbMax->setFixedWidth(kGenLblW);
    lbMax->setObjectName(QStringLiteral("cfgFormLabel"));
    m_maxUseSpin = new CfgComboStyleSpinBox(genParam);
    m_maxUseSpin->setRange(0, 99999999);
    m_maxUseSpin->setFixedWidth(kGenSpinW);
    m_maxUseSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_maxUseSpin->setObjectName(QStringLiteral("cfgSpin"));

    auto *lbCur = new QLabel(QStringLiteral("当前使用次数:"), genParam);
    lbCur->setFixedWidth(kGenLblW);
    lbCur->setObjectName(QStringLiteral("cfgFormLabel"));
    m_curUseSpin = new CfgComboStyleSpinBox(genParam);
    m_curUseSpin->setRange(0, 99999999);
    m_curUseSpin->setReadOnly(true);
    m_curUseSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_curUseSpin->setFixedWidth(kSwitchDelayCurCountSpinW);
    m_curUseSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_curUseSpin->setObjectName(QStringLiteral("cfgSpin"));
    m_curUseSpin->setProperty("noBtn", QVariant(true));

    auto *clearBtn = new QPushButton(QStringLiteral("使用次数清零"), genParam);
    clearBtn->setObjectName(QStringLiteral("cfgAccentBtn"));
    clearBtn->setFixedWidth(120);
    clearBtn->setFixedHeight(kCfgAccentBtnH);
    clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    g->addWidget(lbLight, 0, 0, Qt::AlignVCenter);
    g->addWidget(m_lightDelaySpin, 0, 1, Qt::AlignVCenter);
    g->addWidget(lbSw, 0, 2, Qt::AlignVCenter);
    g->addWidget(m_switchDelaySpin, 0, 3, Qt::AlignVCenter);
    g->setColumnStretch(4, 1);

    g->addWidget(lbMax, 1, 0, Qt::AlignVCenter);
    g->addWidget(m_maxUseSpin, 1, 1, Qt::AlignVCenter);
    g->addWidget(lbCur, 1, 2, Qt::AlignVCenter);
    g->addWidget(m_curUseSpin, 1, 3, Qt::AlignVCenter);
    g->addWidget(clearBtn, 1, 4, Qt::AlignVCenter);
    g->setColumnStretch(5, 1);

    blLay->addWidget(genParam);
    blLay->addStretch();

    // ── 右下：判定阈值范围设置（2 列，从左到右、从上到下）────────────
    const auto secBR   = makeCfgSection(QStringLiteral("判定阈值范围设置"), rowBottom);
    auto      *botRight = secBR.shell;
    auto      *brBody   = secBR.body;
    auto      *brGrid   = new QGridLayout(brBody);
    brGrid->setContentsMargins(0, 0, 0, 0);
    brGrid->setHorizontalSpacing(24);
    brGrid->setVerticalSpacing(8);

    const QStringList thresholdNames = {
        QStringLiteral("死灯数量"),
        QStringLiteral("暗灯数量"),
        QStringLiteral("亮灯数量"),
        QStringLiteral("串色数量"),
        QStringLiteral("串灯数量"),
        QStringLiteral("混色数量"),
        QStringLiteral("缺陷总数"),
    };
    constexpr int kThresholdColCount = 2;
    for (int i = 0; i < thresholdNames.size(); ++i) {
        ThresholdSpinPair pair;
        pair.name = thresholdNames.at(i);
        const int row = i / kThresholdColCount;
        const int col = i % kThresholdColCount;
        brGrid->addWidget(
            makeThresholdBlock(pair.name, brBody, &pair.lower, &pair.upper), row, col);
        m_thresholdSpins.append(pair);
    }
    brGrid->setColumnStretch(0, 1);
    brGrid->setColumnStretch(1, 1);
    brGrid->setRowStretch((thresholdNames.size() + kThresholdColCount - 1) / kThresholdColCount, 1);

    rowBotLay->addWidget(botLeft, 1);
    rowBotLay->addWidget(botRight, 1);
    panelLay->addWidget(rowBottom, 1);

    scrollOuterLay->addWidget(mainPanel, 1);

    m_scroll = scrollWrap(content);

    auto *saveAllBtn = new QPushButton(QStringLiteral("更改参数"), this);
    saveAllBtn->setObjectName(QStringLiteral("cfgAccentBtn"));
    saveAllBtn->setFixedHeight(kCfgAccentBtnH);
    saveAllBtn->setFixedWidth(100);
    saveAllBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto *btnRow = new QHBoxLayout();
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->addWidget(saveAllBtn, 0, Qt::AlignLeft);
    btnRow->addStretch(1);

    auto *rootLay = new QVBoxLayout(this);
    rootLay->setContentsMargins(18, 14, 18, 14);
    rootLay->setSpacing(14);
    rootLay->addLayout(btnRow);
    rootLay->addWidget(m_scroll, 1);

    if (!m_viewModel)
        m_viewModel = new SystemConfigViewModel(this);

    setupBindings();
    DataBinding::bindCommand(m_viewModel->saveCommand(), saveAllBtn);
    DataBinding::bindCommand(m_viewModel->clearUseCountCommand(), clearBtn);
    connect(m_viewModel, &SystemConfigViewModel::sig_saveSucceeded, this, &SystemConfigView::showSaveSuccessTip);
    connect(m_viewModel, &SystemConfigViewModel::sig_saveFailed, this, &SystemConfigView::showSaveFailedTip);
    connect(m_viewModel, &SystemConfigViewModel::sig_configChanged, this, [this]() {
        const auto &gs = m_viewModel->config().generalSettings;
        MessageCenter::Center::Notify(
            EVENT_LIGHTEN_GENERAL_SETTINGS_CHANGED,
            QVariantList{gs.currentUseCount, gs.maxUseCount});
    });

    applyTheme();
}

void SystemConfigView::showSaveSuccessTip()
{
    showSaveTip(QStringLiteral("保存成功"));
}

void SystemConfigView::showSaveFailedTip()
{
    showSaveTip(QStringLiteral("保存失败"));
}

void SystemConfigView::showSaveTip(const QString &message)
{
    if (m_saveSuccessTipDlg)
        m_saveSuccessTipDlg->close();

    auto *tipDlg = new QDialog(this, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    tipDlg->setAttribute(Qt::WA_DeleteOnClose);
    tipDlg->setFixedSize(280, 60);
    m_saveSuccessTipDlg = tipDlg;

    auto *opacityFx = new QGraphicsOpacityEffect(tipDlg);
    opacityFx->setOpacity(1.0);
    tipDlg->setGraphicsEffect(opacityFx);

    auto *tipLayout = new QVBoxLayout(tipDlg);
    tipLayout->setContentsMargins(0, 0, 0, 0);
    auto *tipLabel = new QLabel(message, tipDlg);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLayout->addWidget(tipLabel);

    tipDlg->setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #4d8ec4, stop:0.42 #3a6f9e, stop:1 #244a6e);"
        "  border: none;"
        "  border-radius: 10px;"
        "}"
        "QLabel {"
        "  color: #f8fcff;"
        "  font-size: 17px;"
        "  font-weight: 600;"
        "  letter-spacing: 3px;"
        "  background: transparent;"
        "  border: none;"
        "}"));

    const QPoint center = mapToGlobal(rect().center());
    tipDlg->move(center.x() - 140, center.y() - 30);

    auto *fadeAnim = new QPropertyAnimation(opacityFx, "opacity", tipDlg);
    fadeAnim->setDuration(900);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(fadeAnim, &QPropertyAnimation::finished, tipDlg, &QDialog::close);
    connect(tipDlg, &QObject::destroyed, this, [this]() { m_saveSuccessTipDlg = nullptr; });
    QTimer::singleShot(1000, tipDlg, [fadeAnim]() { fadeAnim->start(); });

    tipDlg->show();
}

void SystemConfigView::setupBindings()
{
    if (!m_viewModel)
        return;

    using SC = SystemConfig;
    const QString prop = SystemConfigViewModel::PROP_CONFIG;

    auto ensureThreshold = [](SC &cfg, const QString &name) -> ThresholdRange & {
        for (ThresholdRange &t : cfg.thresholdRanges) {
            if (t.name == name)
                return t;
        }
        ThresholdRange created;
        created.name = name;
        cfg.thresholdRanges.append(created);
        return cfg.thresholdRanges.last();
    };

    auto thresholdLower = [&ensureThreshold](const SC &cfg, const QString &name) {
        for (const ThresholdRange &t : cfg.thresholdRanges) {
            if (t.name == name)
                return t.lower;
        }
        return 0;
    };
    auto thresholdUpper = [&ensureThreshold](const SC &cfg, const QString &name) {
        for (const ThresholdRange &t : cfg.thresholdRanges) {
            if (t.name == name)
                return t.upper;
        }
        return 0;
    };

    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.generalSwitches.scanBarcode; },
        [](SC &c, bool v) { c.generalSwitches.scanBarcode = v; }, m_scanBarcodeSwitch);
    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.generalSwitches.saveRawImage; },
        [](SC &c, bool v) { c.generalSwitches.saveRawImage = v; }, m_saveRawImageSwitch);
    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.generalSwitches.saveResultImage; },
        [](SC &c, bool v) { c.generalSwitches.saveResultImage = v; }, m_saveResultImageSwitch);
    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.generalSwitches.saveAlgoProcessData; },
        [](SC &c, bool v) { c.generalSwitches.saveAlgoProcessData = v; }, m_saveAlgoDataSwitch);
    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.generalSwitches.algoFailAlarm; },
        [](SC &c, bool v) { c.generalSwitches.algoFailAlarm = v; }, m_algoFailAlarmSwitch);

    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.interfaceSwitches.upstreamEnabled; },
        [](SC &c, bool v) { c.interfaceSwitches.upstreamEnabled = v; }, m_upstreamSwitch);
    bindConfigPillSwitch(
        m_viewModel, prop, [](const SC &c) { return c.interfaceSwitches.downstreamEnabled; },
        [](SC &c, bool v) { c.interfaceSwitches.downstreamEnabled = v; }, m_downstreamSwitch);

    bindDetectItemSwitches(m_viewModel, prop, m_detectRowSwitches);

    if (m_imagePathEdit) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.imageStoragePath; },
            [](SC &c, const QString &v) { c.generalSettings.imageStoragePath = v; },
            m_imagePathEdit);
    }
    if (m_resultPathEdit) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.resultStoragePath; },
            [](SC &c, const QString &v) { c.generalSettings.resultStoragePath = v; },
            m_resultPathEdit);
    }
    if (m_thirdPartyPathEdit) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.thirdPartySoftwarePath; },
            [](SC &c, const QString &v) { c.generalSettings.thirdPartySoftwarePath = v; },
            m_thirdPartyPathEdit);
    }
    if (m_lightDelaySpin) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.lightDelayMs; },
            [](SC &c, int v) { c.generalSettings.lightDelayMs = v; },
            m_lightDelaySpin);
    }
    if (m_switchDelaySpin) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.switchDelayMs; },
            [](SC &c, int v) { c.generalSettings.switchDelayMs = v; },
            m_switchDelaySpin);
    }
    if (m_maxUseSpin) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.maxUseCount; },
            [](SC &c, int v) { c.generalSettings.maxUseCount = v; },
            m_maxUseSpin);
    }
    if (m_curUseSpin) {
        DataBinding::bindStructField<SC>(
            m_viewModel, prop,
            [](const SC &c) { return c.generalSettings.currentUseCount; },
            [](SC &c, int v) { c.generalSettings.currentUseCount = v; },
            m_curUseSpin);
    }

    for (const ThresholdSpinPair &pair : m_thresholdSpins) {
        if (pair.lower) {
            bindThresholdSpin(
                m_viewModel, prop,
                [thresholdLower, name = pair.name](const SC &c) { return thresholdLower(c, name); },
                [ensureThreshold, name = pair.name](SC &c, int v) {
                    ensureThreshold(c, name).lower = v;
                },
                pair.lower);
        }
        if (pair.upper) {
            bindThresholdSpin(
                m_viewModel, prop,
                [thresholdUpper, name = pair.name](const SC &c) { return thresholdUpper(c, name); },
                [ensureThreshold, name = pair.name](SC &c, int v) {
                    ensureThreshold(c, name).upper = v;
                },
                pair.upper);
        }
    }
}

} // namespace lighten
