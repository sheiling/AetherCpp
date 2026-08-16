/**
 * @file virtual_pixel_view.cpp
 * @brief VirtualPixelView 观察区预览、通道配置与保存交互实现
 */

#include "virtual_pixel_view.h"

#include "virtual_pixel_view_model.h"
#include "lighten_param_persist.h"
#include "recipe_model.h"
#include "databinding.h"

#include "uibase/thememanager.h"

#include "interface/igraphicsView.h"
#include "extensionsystem/pluginmanager.h"
#include "messagecenter/message_center.h"
#include <common/core/device/camAsync/cam_async_api.h>

#include <common/core/device/camera/camera_base.h>
#include <common/core/device/deviceManager/device_manager.h>
using namespace ExtensionSystem;

#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QButtonGroup>
#include <QHeaderView>
#include <QScrollArea>
#include <QDateTime>
#include <QLabel>
#include <QAbstractItemView>
#include <QTableWidgetItem>
#include <QRadioButton>
#include <QAbstractButton>
#include <QSizePolicy>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QFontMetrics>
#include <QListView>
#include <QStyledItemDelegate>
#include <QStyleFactory>
#include <QStyleOptionComboBox>
#include <QTimer>
#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QEvent>
#include <QResizeEvent>
#include <QAbstractSpinBox>
#include <QStyleOptionSpinBox>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <utility>

#include <string>
#include <vector>
// ── helpers ──────────────────────────────────────────────────────────────────
/** VirtualPixelView 灯珠网格绘制与通道 UI 辅助 */
namespace {

static QStringList cameraDisplayNames(const std::vector<std::string> &cameraKeys)
{
    QStringList keys;
    keys.reserve(static_cast<int>(cameraKeys.size()));
    for (const auto &key : cameraKeys)
        keys.append(QString::fromStdString(key));
    if (keys.isEmpty()) {
        keys << QStringLiteral("主相机") << QStringLiteral("前相机")
             << QStringLiteral("左相机") << QStringLiteral("右相机")
             << QStringLiteral("后相机");
    }
    return keys;
}

constexpr const char *kMuted  = "#9aa4b5";
constexpr const char *kBorder = "#3a4a5d";
constexpr const char *kBlue   = "#1890ff";

/** 与 recipe_config_view::applyRecipeTheme 中 #recipeCenterShell 一致（产品配方 + 配方操作区底色） */
static QColor vpRecipePanelColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

static constexpr int kVpObservePreviewWinId = 2;
static constexpr int kVpObserveToolbarCtrlH = 26;
/** 产品配方下拉：与 recipe_config_view 基本信息「坐标系」kRecipeBasicCoordComboH / QSS 一致 */
static constexpr int kVpConfigRecipeComboH = 34;

/** 下拉列表只画文字，去掉当前项前的勾选（与 recipe_config_view 调试区一致） */
class PlainComboPopupDelegate final : public QStyledItemDelegate
{
public:
    explicit PlainComboPopupDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->checkState = Qt::Unchecked;
        option->features &= ~QStyleOptionViewItem::HasCheckIndicator;
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize sh = QStyledItemDelegate::sizeHint(option, index);
        const QFontMetrics fm(option.font);
        const int minH = fm.lineSpacing() + 10;
        sh.setHeight(qMax(sh.height(), qMax(minH, 30)));
        return sh;
    }
};

/** 观察区工具条内下拉：弹出宽度与框一致、三角绘制与 recipe_config_view::PreviewToolBarComboBox 一致 */
class VpObserveToolBarComboBox : public QComboBox
{
public:
    explicit VpObserveToolBarComboBox(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
        if (QAbstractItemView *v = view())
            v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

protected:
    void showPopup() override
    {
        QComboBox::showPopup();
        QAbstractItemView *v = view();
        if (!v)
            return;
        const int w = width();
        v->setMinimumWidth(w);
        v->setMaximumWidth(w);

        QWidget *popup = v->parentWidget();
        while (popup && !popup->isWindow())
            popup = popup->parentWidget();
        if (!popup)
            return;
        QTimer::singleShot(0, this, [this, popup] {
            const int w = width();
            const QPoint below = mapToGlobal(QPoint(0, height()));
            popup->move(below.x(), below.y() + 2);
            if (QLayout *ly = popup->layout())
                ly->setContentsMargins(0, 0, 0, 0);
            popup->setFixedWidth(w);
        });
    }

    void paintEvent(QPaintEvent *event) override
    {
        QComboBox::paintEvent(event);
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        const QRect r = style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                 QStyle::SC_ComboBoxArrow, this);
        if (!r.isValid())
            return;

        const QRect fill = r.adjusted(1, 1, -2, -1);
        if (fill.width() <= 0 || fill.height() <= 0)
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        constexpr QRgb kFieldBg = qRgb(0x15, 0x2a, 0x3d);
        p.fillRect(fill, QColor::fromRgb(kFieldBg));

        const int cx = r.center().x();
        const int cy = r.center().y() + 1;
        QPolygon tri;
        tri << QPoint(cx - 4, cy - 3) << QPoint(cx + 4, cy - 3) << QPoint(cx, cy + 3);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawPolygon(tri);
    }
};

/** 下拉弹出层挂在 view 上，与 recipe_config_view::previewToolbarComboPopupViewStyleSheet 一致 */
static QString vpObserveToolbarComboPopupViewStyleSheet()
{
    return QStringLiteral(
        "QAbstractItemView {"
        "  background-color: #152a3d;"
        "  color: #ffffff;"
        "  font-size: 11px;"
        "  outline: none;"
        "  border: 1px solid #cfd8e3;"
        "  padding: 0px;"
        "  margin: 0px;"
        "  selection-background-color: transparent;"
        "  selection-color: #ffffff;"
        "}"
        "QAbstractItemView::item {"
        "  min-height: 30px;"
        "  padding: 6px 10px 6px 6px;"
        "  border: none;"
        "  color: #ffffff;"
        "  font-size: 11px;"
        "  background-color: transparent;"
        "}"
        "QAbstractItemView::item:hover {"
        "  background-color: #3a4d63;"
        "}"
        "QAbstractItemView::item:selected {"
        "  background-color: #3d5a80;"
        "  color: #ffffff;"
        "}"
        "QAbstractItemView::indicator {"
        "  width: 0px;"
        "  height: 0px;"
        "}"
        "QScrollBar:vertical {"
        "  width: 6px;"
        "  background: #152a3d;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #4a6078;"
        "  min-height: 16px;"
        "  border-radius: 2px;"
        "}"
    );
}

/** 与 recipe_config_view::paintComboLikeSpinArrows 一致，用于灯点行列 Spin */
static void vpPaintComboLikeSpinArrows(QAbstractSpinBox *spin, const QStyleOptionSpinBox &opt)
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

/** 与 recipe_config_view::RecipeComboStyleSpinBox 一致（Fusion + 下拉式箭头绘制） */
class VpRecipeComboStyleSpinBox final : public QSpinBox
{
public:
    explicit VpRecipeComboStyleSpinBox(QWidget *parent = nullptr)
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
        vpPaintComboLikeSpinArrows(this, opt);
    }
};

/** 与 recipe_config_view::RecipeCamTableComboBox 一致，基类为 VpObserveToolBarComboBox */
class VpConfigCamTableComboBox final : public VpObserveToolBarComboBox
{
public:
    explicit VpConfigCamTableComboBox(int pixelH, QWidget *parent = nullptr)
        : VpObserveToolBarComboBox(parent)
        , m_h(pixelH)
    {
        if (QStyle *fus = QStyleFactory::create(QStringLiteral("Fusion")))
            setStyle(fus);
        clampHeight();
        QSizePolicy sp = sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        sp.setVerticalPolicy(QSizePolicy::Fixed);
        setSizePolicy(sp);
    }

    QSize sizeHint() const override
    {
        QSize s = VpObserveToolBarComboBox::sizeHint();
        s.setHeight(m_h);
        return s;
    }

    QSize minimumSizeHint() const override
    {
        QSize s = VpObserveToolBarComboBox::minimumSizeHint();
        s.setHeight(m_h);
        return s;
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        VpObserveToolBarComboBox::resizeEvent(event);
        clampHeight();
    }

    void changeEvent(QEvent *event) override
    {
        VpObserveToolBarComboBox::changeEvent(event);
        if (event->type() == QEvent::StyleChange || event->type() == QEvent::FontChange)
            clampHeight();
    }

private:
    void clampHeight() { setFixedHeight(m_h); }

    int m_h;
};

/** 排列方式示意图：正常为两列横条；1-4 / 2-3 为三列错位横条 */
class ArrangementDiagram final : public QWidget
{
public:
    enum Mode { Normal, OneFour, TwoThree };

    explicit ArrangementDiagram(Mode m, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_mode(m)
    {
        setFixedSize(108, 78);
        setAttribute(Qt::WA_StyledBackground, true);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter pa(this);
        pa.setRenderHint(QPainter::Antialiasing);
        pa.fillRect(rect(), QColor(QStringLiteral("#151d2a")));

        const QColor lamp(QString::fromLatin1(kBlue));
        pa.setPen(Qt::NoPen);
        pa.setBrush(lamp);

        constexpr int barW  = 22;
        constexpr int barH  = 7;
        constexpr int gapX  = 5;
        constexpr int gapY  = 6;
        constexpr int baseX = 10;
        constexpr int baseY = 12;

        auto drawBar = [&](int x, int y) {
            pa.drawRoundedRect(x, y, barW, barH, 2, 2);
        };

        if (m_mode == Normal) {
            constexpr int colGap = 14;
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 2; ++c) {
                    const int x = baseX + c * (barW + colGap);
                    const int y = baseY + r * (barH + gapY);
                    drawBar(x, y);
                }
            }
            return;
        }

        constexpr int colCount = 3;
        for (int r = 0; r < 3; ++r) {
            int rowOff = 0;
            if (m_mode == OneFour && (r % 2) == 1)
                rowOff = (barW + gapX) / 2;
            else if (m_mode == TwoThree && (r % 2) == 0)
                rowOff = (barW + gapX) / 2;
            for (int c = 0; c < colCount; ++c) {
                const int x = baseX + c * (barW + gapX) + rowOff;
                const int y = baseY + r * (barH + gapY);
                if (x + barW <= width() - 4)
                    drawBar(x, y);
            }
        }
    }

private:
    Mode m_mode;
};

static void applyVirtualPixelTheme(QWidget *root)
{
    const QString kRecipeShellBg =
        vpRecipePanelColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a"))).name();
    const QString kRecipeShellBr =
        vpRecipePanelColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668"))).name();
    const QString vpShellFrameStyle =
        QStringLiteral("#virtualPixelRoot QFrame#vpPanelFrame {"
                       "  background-color: ")
        + kRecipeShellBg
        + QStringLiteral("; border: 1px solid ")
        + kRecipeShellBr
        + QStringLiteral("; border-radius: 6px;"
                         "}");

    root->setObjectName(QStringLiteral("virtualPixelRoot"));
    root->setAttribute(Qt::WA_StyledBackground, true);

    const QString stylePrefix = QStringLiteral(
        "#virtualPixelRoot { background: transparent; }"

        "#virtualPixelRoot QLabel#vpSectionTitle {"
        "  color: #ffffff; font-size: 13px; font-weight: 700;"
        "  background: transparent;"
        "}");

    const QString styleRest = QStringLiteral(

        "#virtualPixelRoot QComboBox {"
        "  min-height: 28px;"
        "  padding: 2px 10px;"
        "  background-color: #1f2d40;"
        "  color: #e8eef6;"
        "  border: 1px solid #4a5f79;"
        "  border-radius: 4px;"
        "}"
        "#virtualPixelRoot QComboBox::drop-down { border: none; width: 22px; }"
        "#virtualPixelRoot QComboBox QAbstractItemView {"
        "  background-color: #1f2d40;"
        "  color: #e8eef6;"
        "  selection-background-color: %4;"
        "}"

        "#virtualPixelRoot QFrame#vpToolbar {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QComboBox {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 26px; max-height: 26px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QComboBox:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveToolIconBtn {"
        "  min-width: 26px; max-width: 26px; min-height: 26px; max-height: 26px;"
        "  padding: 0px; margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveToolIconBtn:checked {"
        "  background-color: #1a3550; color: #ffffff;"
        "  border: 1px solid #42a5f5; font-weight: bold;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveToolIconBtn:hover:!checked {"
        "  background-color: #263a52; border: 1px solid #6b8cae;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveColorBtn {"
        "  min-height: 26px; max-height: 26px; min-width: 20px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px;"
        "  margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveColorBtn:checked {"
        "  background-color: #1a3550; color: #ffffff;"
        "  border: 1px solid #42a5f5; font-weight: bold;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveColorBtn:hover:!checked {"
        "  background-color: #263a52; border: 1px solid #6b8cae;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveActionBtn {"
        "  min-height: 26px; max-height: 26px; min-width: 56px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 8px; padding-right: 8px;"
        "  margin: 0px;"
        "  background-color: #1a4a8c; color: #ffffff;"
        "  border: 1px solid #64b5f6; border-radius: 5px;"
        "  font-weight: bold;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveActionBtn:hover {"
        "  background-color: #1e5bb0; border: 1px solid #90caf9;"
        "}"
        "#virtualPixelRoot QFrame#vpToolbar QPushButton#vpObserveActionBtn:pressed {"
        "  background-color: #153a75;"
        "}"

        "#virtualPixelRoot QPushButton#vpChannelPill {"
        "  min-width: 26px; max-width: 26px; min-height: 26px; max-height: 26px;"
        "  padding: 0px;"
        "  border-radius: 13px;"
        "  background-color: #1f2d40;"
        "  color: #dbe4ee;"
        "  border: 1px solid #4a5f79;"
        "  font-weight: bold; font-size: 11px;"
        "}"
        "#virtualPixelRoot QPushButton#vpChannelPill:hover:!checked { background-color: #29405d; }"
        "#virtualPixelRoot QPushButton#vpChannelPill:checked {"
        "  background-color: #184c86;"
        "  color: #e8eef6;"
        "  border: 1px solid #2f82da;"
        "}"
        "#virtualPixelRoot QPushButton#vpChannelPill:checked:hover { background-color: #1f5ea2; }"
        "#virtualPixelRoot QPushButton#vpChannelPill:checked:pressed { background-color: #17477b; }"

        "#virtualPixelRoot QLabel#vpPreviewPane {"
        "  background-color: #151d2a;"
        "  border: 1px dashed %2;"
        "  border-radius: 6px;"
        "  color: %3;"
        "  font-size: 15px;"
        "}"

        "#virtualPixelRoot QTableWidget {"
        "  background-color: #1a2332;"
        "  alternate-background-color: #202b3d;"
        "  color: #e8eef6;"
        "  gridline-color: %2;"
        "  border: 1px solid %2;"
        "  border-radius: 4px;"
        "}"
        "#virtualPixelRoot QTableWidget::item { padding: 4px; }"
        "#virtualPixelRoot QHeaderView::section {"
        "  background-color: #1f2d40;"
        "  color: #dbe4ee;"
        "  padding: 6px;"
        "  border: none;"
        "  border-bottom: 1px solid %2;"
        "  font-weight: bold;"
        "}"

        "#virtualPixelRoot QTextEdit#logBox {"
        "  background-color: #0d1117;"
        "  color: #c9d1d9;"
        "  border: 1px solid #30363d;"
        "  border-radius: 4px;"
        "  font-family: Consolas, 'Courier New', monospace;"
        "  font-size: 12px;"
        "}"

        "#virtualPixelRoot QSpinBox {"
        "  min-height: 28px;"
        "  padding: 2px 8px;"
        "  background-color: #1f2d40;"
        "  color: #e8eef6;"
        "  border: 1px solid #4a5f79;"
        "  border-radius: 4px;"
        "}"
        "#virtualPixelRoot QLineEdit#vpMapRule {"
        "  min-height: 30px;"
        "  padding: 4px 10px;"
        "  background-color: #151d2a;"
        "  color: #dbe4ee;"
        "  border: 1px solid %2;"
        "  border-radius: 4px;"
        "  font-family: Consolas, 'Courier New', monospace;"
        "}"

        "#virtualPixelRoot QLabel#vpFormLabel { color: #dbe4ee; font-size: 13px; }"
        "#virtualPixelRoot QWidget#vpConfigBasicForm {"
        "  background-color: transparent;"
        "}"
        "#virtualPixelRoot QWidget#vpConfigBasicForm QLabel#vpConfigFormLabel {"
        "  color: #9eb8d4; font-size: 11px; background: transparent;"
        "  padding: 0px; margin: 0px;"
        "}"
        "#virtualPixelRoot QWidget#vpConfigBasicForm QComboBox#vpConfigRecipeCombo {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 34px; max-height: 34px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#virtualPixelRoot QWidget#vpConfigBasicForm QComboBox#vpConfigRecipeCombo:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#virtualPixelRoot QWidget#vpConfigBasicForm QComboBox#vpConfigRecipeCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#virtualPixelRoot QLabel#vpSectionLabel {"
        "  color: #ffffff; font-size: 13px; font-weight: 600;"
        "  margin: 0; padding: 0;"
        "}"
        "#virtualPixelRoot QLabel#vpHint { color: %3; font-size: 12px; }"

        "#virtualPixelRoot QFrame#vpChannelSettings {"
        "  background-color: #1a2332;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "  padding: 0px;"
        "  min-height: 0px;"
        "}"

        "#virtualPixelRoot QFrame#vpChannelSettings QLabel#vpSectionLabel {"
        "  margin: 0; padding: 0;"
        "  min-height: 0px;"
        "}"

        "#virtualPixelRoot QFrame#vpChannelSettings QSpinBox#vpConfigCamSpin {"
        "  background-color: #15263d;"
        "  color: #e8eef6;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 2px 22px 2px 6px;"
        "  min-height: 24px;"
        "  min-width: 46px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "#virtualPixelRoot QFrame#vpChannelSettings QSpinBox#vpConfigCamSpin::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 16px; height: 12px;"
        "  border: none; background-color: transparent;"
        "}"
        "#virtualPixelRoot QFrame#vpChannelSettings QSpinBox#vpConfigCamSpin::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 16px; height: 12px;"
        "  border: none; background-color: transparent;"
        "}"

        "#virtualPixelRoot QFrame#vpDemoWrap {"
        "  background-color: #1a2332;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "}"
        "#virtualPixelRoot QFrame#vpDemoWrap[active=\"true\"] {"
        "  border: 2px solid %4;"
        "  background-color: #182436;"
        "}"

        "#virtualPixelRoot QRadioButton#vpDemoRadio { color: #ffffff; font-weight: 600; font-size: 12px; }"
        "#virtualPixelRoot QRadioButton#vpDemoRadio::indicator {"
        "  width: 14px; height: 14px;"
        "  border-radius: 7px;"
        "  border: 1px solid #4a5f79;"
        "  background-color: #1f2d40;"
        "}"
        "#virtualPixelRoot QRadioButton#vpDemoRadio::indicator:checked {"
        "  background-color: %4;"
        "  border: 1px solid #6bb7ff;"
        "}"

        "#virtualPixelRoot QPushButton#vpSaveBtn {"
        "  min-height: 30px; min-width: 52px;"
        "  padding: 0 12px;"
        "  background-color: #184c86;"
        "  color: #e8eef6;"
        "  border: 1px solid #2f82da;"
        "  border-radius: 4px;"
        "  font-weight: bold; font-size: 12px;"
        "}"
        "#virtualPixelRoot QPushButton#vpSaveBtn:hover { background-color: #1f5ea2; }"
        "#virtualPixelRoot QPushButton#vpSaveBtn:pressed { background-color: #17477b; }"

        "#virtualPixelRoot QScrollArea { background: transparent; border: none; }"
        "#virtualPixelRoot QScrollArea > QWidget > QWidget { background: transparent; }"
        )
        .arg(QString::fromLatin1(kBorder))
        .arg(QString::fromLatin1(kMuted))
        .arg(QString::fromLatin1(kBlue));

    root->setStyleSheet(stylePrefix + vpShellFrameStyle + styleRest);
}

/** 虚拟像素配置区「灯点行列」控件聚合 */
struct VpLampWidgets {
    QWidget *               panel  = nullptr;
    VpRecipeComboStyleSpinBox *rowSpin = nullptr;
    VpRecipeComboStyleSpinBox *colSpin = nullptr;
};

static VpLampWidgets makeVpLampPanel(int rows, int cols, QWidget *parent)
{
    VpLampWidgets out;
    out.panel = new QWidget(parent);
    auto *root = new QHBoxLayout(out.panel);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(10);

    auto *gridLbl = new QLabel(QStringLiteral("灯点行列"), out.panel);
    gridLbl->setObjectName(QStringLiteral("vpSectionLabel"));
    gridLbl->setContentsMargins(0, 0, 0, 0);

    out.rowSpin = new VpRecipeComboStyleSpinBox(out.panel);
    out.rowSpin->setObjectName(QStringLiteral("vpConfigCamSpin"));
    out.rowSpin->setRange(1, 9999);
    out.rowSpin->setValue(rows);
    out.rowSpin->setAlignment(Qt::AlignCenter);
    out.rowSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    out.colSpin = new VpRecipeComboStyleSpinBox(out.panel);
    out.colSpin->setObjectName(QStringLiteral("vpConfigCamSpin"));
    out.colSpin->setRange(1, 9999);
    out.colSpin->setValue(cols);
    out.colSpin->setAlignment(Qt::AlignCenter);
    out.colSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    root->addWidget(gridLbl, 0, Qt::AlignVCenter);
    root->addWidget(out.rowSpin, 1);
    root->addWidget(out.colSpin, 1);

    out.panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    return out;
}

static void syncDemoWrapActive(QFrame *wrap, bool on)
{
    wrap->setProperty("active", on);
    wrap->style()->unpolish(wrap);
    wrap->style()->polish(wrap);
    wrap->update();
}

} // namespace

/** @brief VirtualPixelView 成员实现（观察预览、三通道配置与保存交互） */
namespace lighten {

VirtualPixelView::VirtualPixelView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("lighten.virtual_pixel");
    setPageTitle(QStringLiteral("虚拟像素"));
}

VirtualPixelView::~VirtualPixelView()
{
    MessageCenter::Center::Detach(CAM_ASYNC_EVENT_CAMERA_IMG, this, SLOT(slot_camera_img(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_CAMERA_STATE, this, SLOT(slot_camera_state(const QVariantList &)));
}

void VirtualPixelView::onLoad()
{
    buildLayout();
    setupViewModel();

    MessageCenter::Center::Attach(CAM_ASYNC_EVENT_CAMERA_IMG, this, SLOT(slot_camera_img(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_CAMERA_STATE, this, SLOT(slot_camera_state(const QVariantList &)));

    appendLog(QStringLiteral("已加载配方 PANEL-A01 的虚拟像素参数"), QStringLiteral("INFO"));
    appendLog(QStringLiteral("打屏完成，等待拍摄"), QStringLiteral("INFO"));
    appendLog(QStringLiteral("坐标(2,0) 映射后状态需复核"), QStringLiteral("WARN"));
    appendLog(QStringLiteral("检测项统计已刷新，当前总数 12"), QStringLiteral("INFO"));
    appendLog(QStringLiteral("执行单步调试: 检测"), QStringLiteral("INFO"));
    appendLog(QStringLiteral("坐标(9,0) 判定为串色，建议复检"), QStringLiteral("WARN"));
    appendLog(QStringLiteral("映射规则校验通过"), QStringLiteral("INFO"));
    appendLog(QStringLiteral("本次图像采集完成，耗时 0.42s"), QStringLiteral("INFO"));
}

void VirtualPixelView::syncPreviewButtonFromCaptureState()
{
    if (!m_observeCamCombo || !m_liveToggleBtn)
        return;

    const auto dm = tryGetDeviceManager();
    if (!dm)
        return;

    const auto camera = dm->CameraHandler(m_observeCamCombo->currentText().toStdString());
    if (!camera)
        return;

    const auto state = camera->GetCaptureState();

    if (m_snapBtn)
        m_snapBtn->setEnabled(state == common::device::CaptureState::Idle);

    switch (state) {
    case common::device::CaptureState::Idle:
        m_liveToggleBtn->setEnabled(true);
        m_liveToggleBtn->setText(QStringLiteral("▶"));
        m_liveToggleBtn->setToolTip(QStringLiteral("实时预览"));
        break;
    case common::device::CaptureState::Preview:
        m_liveToggleBtn->setEnabled(true);
        m_liveToggleBtn->setText(QStringLiteral("\u23F9"));
        m_liveToggleBtn->setToolTip(QStringLiteral("停止"));
        break;
    case common::device::CaptureState::Capturing:
        m_liveToggleBtn->setEnabled(false);
        m_liveToggleBtn->setText(QStringLiteral("▶"));
        m_liveToggleBtn->setToolTip(QStringLiteral("实时预览"));
        break;
    }
}

void VirtualPixelView::slot_camera_img(const QVariantList &args)
{
    if (args.size() < 3)
        return;

    const int winId = args.at(0).toInt();
    if (winId != kVpObservePreviewWinId)
        return;

    const QString cameraName = args.at(1).toString();
    if (!m_observeCamCombo || m_observeCamCombo->currentText() != cameraName)
        return;

    const QImage image = qvariant_cast<QImage>(args.at(2));

    if (!m_previewStack)
        return;

    constexpr int kPlaceholderPage = 0;
    constexpr int kGraphicsPage    = 1;

    if (!image.isNull() && m_graphicsView && m_previewStack->count() > kGraphicsPage) {
        m_graphicsView->SetImage(winId, image);
        m_previewStack->setCurrentIndex(kGraphicsPage);
    } else {
        m_previewStack->setCurrentIndex(kPlaceholderPage);
    }
}

void VirtualPixelView::slot_camera_state(const QVariantList &args)
{
    if (args.size() < 2)
        return;

    const QString cameraSn = args.at(0).toString();
    if (cameraSn.isEmpty() || !m_observeCamCombo || !m_liveToggleBtn)
        return;

    const auto dm = tryGetDeviceManager();
    if (!dm)
        return;

    const auto camera = dm->CameraHandler(m_observeCamCombo->currentText().toStdString());
    if (!camera)
        return;

    const QString currentSn = QString::fromStdString(camera->GetCameraInfo().cameraID);
    if (currentSn != cameraSn)
        return;

    syncPreviewButtonFromCaptureState();
}

void VirtualPixelView::buildLayout()
{
    applyVirtualPixelTheme(this);

    auto *root = new QHBoxLayout(this);
    // 与窗口边、页头留出空隙（左栏偏左/上/下，右栏偏右/上/下）
    root->setContentsMargins(10, 12, 10, 10);
    root->setSpacing(10);

    root->addWidget(buildObservePanel(), 50);
    root->addWidget(buildConfigPanel(),  50);

    // 观察区 红/绿/蓝 ↔ 配置区 R/G/B 与通道堆栈联动（黑/白不参与通道）
    if (m_observeColorGroup && m_channelGroup && m_channelStack && m_channelBtns.size() == 3) {
        connect(m_observeColorGroup, &QButtonGroup::idClicked, this, [this](int id) {
            if (m_syncingObserveConfigRgb)
                return;
            if (id < 0 || id > 2)
                return;
            m_syncingObserveConfigRgb = true;
            for (int i = 0; i < m_channelBtns.size(); ++i) {
                QSignalBlocker b(m_channelBtns[i]);
                m_channelBtns[i]->setChecked(i == id);
            }
            syncVpChannelSharedUi(id);
            m_syncingObserveConfigRgb = false;
        });
        connect(m_channelGroup, &QButtonGroup::idClicked, this, [this](int id) {
            if (m_syncingObserveConfigRgb)
                return;
            m_syncingObserveConfigRgb = true;
            syncVpChannelSharedUi(id);
            if (m_viewModel)
                m_viewModel->setActiveChannel(id);
            if (id >= 0 && id <= 2 && m_observeColorGroup) {
                if (QAbstractButton *ob = m_observeColorGroup->button(id))
                    ob->setChecked(true);
            }
            m_syncingObserveConfigRgb = false;
        });
    }
}

void VirtualPixelView::flushActiveChannelFromUi()
{
    if (m_vpActiveCfgChannel < 0 || m_vpActiveCfgChannel > 2)
        return;
    if (!m_demoPickGroup || m_mapRuleEdits.size() != 3 || m_vpLampRowSpins.size() != 3
        || m_vpLampColSpins.size() != 3)
        return;

    const int o = m_vpActiveCfgChannel;
    int demo    = m_demoPickGroup->checkedId();
    if (demo < 0 || demo > 2)
        demo = m_vpChannelCfg[o].layoutDemoId;
    m_vpChannelCfg[o].layoutDemoId = demo;
    m_vpChannelCfg[o].mapRule0     = m_mapRuleEdits[0]->text();
    m_vpChannelCfg[o].mapRule1     = m_mapRuleEdits[1]->text();
    m_vpChannelCfg[o].mapRule2     = m_mapRuleEdits[2]->text();
    m_vpChannelCfg[o].lampRows     = m_vpLampRowSpins[o]->value();
    m_vpChannelCfg[o].lampCols     = m_vpLampColSpins[o]->value();
}

void VirtualPixelView::syncUiToDocument()
{
    if (!m_viewModel)
        return;

    flushActiveChannelFromUi();

    const QString recipeId = m_recipeCombo ? m_recipeCombo->currentText() : QString();

    m_viewModel->modifyDocument([&](VirtualPixelDocument &doc) {
        if (!recipeId.isEmpty())
            doc.selectedRecipeId = recipeId;
        for (int i = 0; i < 3; ++i)
            doc.channels[i] = m_vpChannelCfg[i];
        if (m_observeCamCombo)
            doc.observe.camera = static_cast<CameraIndex>(m_observeCamCombo->currentIndex());
        if (m_observeColorGroup) {
            const int colorId = m_observeColorGroup->checkedId();
            if (colorId >= 0 && colorId <= 4)
                doc.observe.colorChannel = static_cast<ColorChannel>(colorId);
        }
    });

    const int ch = (m_vpActiveCfgChannel >= 0 && m_vpActiveCfgChannel <= 2) ? m_vpActiveCfgChannel : 0;
    m_viewModel->setActiveChannel(ch);
}

void VirtualPixelView::applyDocumentToUi()
{
    if (!m_viewModel || m_applyingDocument)
        return;

    const VirtualPixelDocument doc = m_viewModel->document();

    m_applyingDocument = true;

    if (m_recipeCombo) {
        QSignalBlocker blocker(m_recipeCombo);
        const int idx = m_recipeCombo->findText(doc.selectedRecipeId);
        if (idx >= 0)
            m_recipeCombo->setCurrentIndex(idx);
        else if (!doc.selectedRecipeId.isEmpty())
            m_recipeCombo->setCurrentText(doc.selectedRecipeId);
    }

    for (int i = 0; i < 3; ++i)
        m_vpChannelCfg[i] = doc.channels[i];

    int ch = m_viewModel->activeChannel();
    if (ch < 0 || ch > 2)
        ch = 0;
    syncVpChannelSharedUi(ch);

    if (m_observeCamCombo) {
        QSignalBlocker blocker(m_observeCamCombo);
        const int camIdx = qBound(0, static_cast<int>(doc.observe.camera),
                                  m_observeCamCombo->count() - 1);
        m_observeCamCombo->setCurrentIndex(camIdx);
    }

    if (m_observeColorGroup) {
        m_syncingObserveConfigRgb = true;
        const int colorIdx = qBound(0, static_cast<int>(doc.observe.colorChannel), 4);
        if (QAbstractButton *ob = m_observeColorGroup->button(colorIdx))
            ob->setChecked(true);
        m_syncingObserveConfigRgb = false;
    }

    m_applyingDocument = false;
}

void VirtualPixelView::refreshRecipeCombo()
{
    if (!m_recipeCombo || !m_viewModel)
        return;

    VirtualPixelRepository::load();

    const QString previousId = m_viewModel->document().selectedRecipeId;

    m_applyingDocument = true;
    {
        QSignalBlocker blocker(m_recipeCombo);
        m_recipeCombo->clear();
        const QVector<RecipeListItem> items = RecipeRepository::listRecipes();
        for (const RecipeListItem &item : items)
            m_recipeCombo->addItem(item.id);
    }

    if (m_recipeCombo->count() == 0) {
        m_applyingDocument = false;
        return;
    }

    QString nextId = previousId;
    if (m_recipeCombo->findText(previousId) < 0) {
        nextId = RecipeRepository::defaultRecipeId();
        if (m_recipeCombo->findText(nextId) < 0)
            nextId = m_recipeCombo->itemText(0);
        m_recipeCombo->setCurrentText(nextId);
        m_viewModel->slot_loadForRecipe(nextId);
        applyDocumentToUi();
    } else {
        const int idx = m_recipeCombo->findText(previousId);
        if (idx >= 0)
            m_recipeCombo->setCurrentIndex(idx);
    }

    m_applyingDocument = false;
}

void VirtualPixelView::setupViewModel()
{
    if (m_viewModel)
        return;

    m_viewModel = new VirtualPixelViewModel(this);

    refreshRecipeCombo();
    applyDocumentToUi();

    connect(RecipeCatalogNotifier::instance(), &RecipeCatalogNotifier::sig_recipeListChanged, this,
            &VirtualPixelView::refreshRecipeCombo);

    if (m_recipeCombo) {
        connect(m_recipeCombo, &QComboBox::currentTextChanged, this, [this](const QString &recipeId) {
            if (m_applyingDocument || !m_viewModel || recipeId.isEmpty())
                return;
            syncUiToDocument();
            m_viewModel->slot_loadForRecipe(recipeId);
            applyDocumentToUi();
        });
    }

    if (m_saveBtn) {
        DataBinding::bindCommand(m_viewModel->saveCommand(), m_saveBtn, [this]() {
            syncUiToDocument();
        });
    }

    connect(m_viewModel, &VirtualPixelViewModel::sig_saveSucceeded, this, &VirtualPixelView::showSaveSuccessTip);

    connect(m_viewModel, &VirtualPixelViewModel::sig_statusChanged, this, [this]() {
        if (!m_viewModel)
            return;
        const QString status = m_viewModel->status();
        if (status.isEmpty())
            return;
        const QString level = status.contains(QStringLiteral("失败")) ? QStringLiteral("ERROR")
                            : status.contains(QStringLiteral("请先")) ? QStringLiteral("WARN")
                                                                      : QStringLiteral("INFO");
        appendLog(status, level);
    });

    connect(m_viewModel, &ViewModel::propertyChanged, this,
            [this](const QString &name, const QVariant &) {
                if (name == VirtualPixelViewModel::PROP_DOCUMENT)
                    applyDocumentToUi();
            });
}

void VirtualPixelView::syncVpChannelSharedUi(int newCh)
{
    if (newCh < 0 || newCh > 2)
        return;
    const bool haveUi = m_demoPickGroup && m_demoWrapFrames.size() == 3 && m_mapRuleEdits.size() == 3
        && m_vpLampRowSpins.size() == 3 && m_vpLampColSpins.size() == 3 && m_channelStack;
    if (!haveUi)
        return;

    flushActiveChannelFromUi();

    m_vpActiveCfgChannel = newCh;

    const VpChannelCfgSnap &snap = m_vpChannelCfg[newCh];
    const int               d    = qBound(0, snap.layoutDemoId, 2);
    for (int i = 0; i < 3; ++i) {
        if (QAbstractButton *bu = m_demoPickGroup->button(i)) {
            QSignalBlocker bl(bu);
            bu->setChecked(i == d);
        }
    }
    for (int i = 0; i < m_demoWrapFrames.size(); ++i)
        syncDemoWrapActive(m_demoWrapFrames[i], i == d);

    m_mapRuleEdits[0]->setText(snap.mapRule0);
    m_mapRuleEdits[1]->setText(snap.mapRule1);
    m_mapRuleEdits[2]->setText(snap.mapRule2);

    for (int ch = 0; ch < 3; ++ch) {
        QSignalBlocker br(m_vpLampRowSpins[ch]);
        QSignalBlocker bc(m_vpLampColSpins[ch]);
        m_vpLampRowSpins[ch]->setValue(m_vpChannelCfg[ch].lampRows);
        m_vpLampColSpins[ch]->setValue(m_vpChannelCfg[ch].lampCols);
    }

    m_channelStack->setCurrentIndex(newCh);
}

// ── Observe panel (left) ──────────────────────────────────────────────────────

QWidget *VirtualPixelView::buildObservePanel()
{
    auto *box = new QFrame(this);
    box->setObjectName(QStringLiteral("vpPanelFrame"));
    box->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 8, 12, 10);
    layout->setSpacing(8);

    auto *mainTitle = new QLabel(QStringLiteral("观察与结果输出"), box);
    mainTitle->setObjectName(QStringLiteral("vpSectionTitle"));
    layout->addWidget(mainTitle);

    // Toolbar（与 recipe_config_view 调试区 recipePreviewToolbar 同款：Fusion 下拉、固定行高 26、弹出层样式）
    auto *toolbar = new QFrame(box);
    toolbar->setObjectName(QStringLiteral("vpToolbar"));
    toolbar->setAttribute(Qt::WA_StyledBackground, true);
    auto *tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(8, 2, 8, 2);
    tl->setSpacing(6);

    auto pinToolbarCtrlHeight = [](QWidget *w) {
        w->setFixedHeight(kVpObserveToolbarCtrlH);
        QSizePolicy sp = w->sizePolicy();
        sp.setVerticalPolicy(QSizePolicy::Fixed);
        w->setSizePolicy(sp);
    };

    const std::vector<std::string> cameraKeys = DeviceConfigReader::loadCameraKeys();

    m_observeCamCombo = new VpObserveToolBarComboBox(toolbar);
    m_observeCamCombo->addItems(cameraDisplayNames(cameraKeys));
    m_observeCamCombo->setMinimumWidth(112);
    tl->addWidget(m_observeCamCombo, 0, Qt::AlignVCenter);

    auto *snapBtn = new QPushButton(QStringLiteral("📷"), toolbar);
    snapBtn->setObjectName(QStringLiteral("vpObserveToolIconBtn"));
    snapBtn->setCheckable(false);
    snapBtn->setToolTip(QStringLiteral("抓拍 "));
    m_snapBtn = snapBtn;
    tl->addWidget(snapBtn, 0, Qt::AlignVCenter);
    connect(snapBtn, &QPushButton::clicked, this, [this]() {
        if (!m_observeCamCombo)
            return;

        const QString cameraName = m_observeCamCombo->currentText();
        cam_async::GrabOneImage(cameraName, kVpObservePreviewWinId);
    });
    auto *liveBtn = new QPushButton(QStringLiteral("▶"), toolbar);
    liveBtn->setObjectName(QStringLiteral("vpObserveToolIconBtn"));
    m_liveToggleBtn = liveBtn;
    syncPreviewButtonFromCaptureState();
    tl->addWidget(liveBtn, 0, Qt::AlignVCenter);
    connect(liveBtn, &QPushButton::clicked, this, [this]() {
        if (!m_observeCamCombo)
            return;

        const auto dm = tryGetDeviceManager();
        if (!dm)
            return;

        const auto camera = dm->CameraHandler(m_observeCamCombo->currentText().toStdString());
        if (!camera)
            return;

        const auto state = camera->GetCaptureState();
        const QString cameraName = m_observeCamCombo->currentText();

        if (state == common::device::CaptureState::Idle) {
            if (!cam_async::StartAsyncGrab(cameraName, kVpObservePreviewWinId))
                revertLiveToggleButton();
        } else if (state == common::device::CaptureState::Preview) {
            QString name = cameraName;
            if (!cam_async::StopAsyncGrab(name))
                revertLiveToggleButton();
        }
    });

    tl->addSpacing(4);
    m_observeColorGroup = new QButtonGroup(this);
    m_observeColorGroup->setExclusive(true);
    const QStringList colorLabels = {QStringLiteral("红"), QStringLiteral("绿"), QStringLiteral("蓝"),
                                     QStringLiteral("黑"), QStringLiteral("白")};
    for (int i = 0; i < colorLabels.size(); ++i) {
        auto *b = new QPushButton(colorLabels[i], toolbar);
        b->setObjectName(QStringLiteral("vpObserveColorBtn"));
        b->setCheckable(true);
        b->setChecked(i == 0);
        m_observeColorGroup->addButton(b, i);
        tl->addWidget(b, 0, Qt::AlignVCenter);
    }

    tl->addSpacing(6);
    QVector<QPushButton *> observeActionBtns;
    observeActionBtns.reserve(4);
    for (const QString &s : {QStringLiteral("进料"), QStringLiteral("出料"), QStringLiteral("打屏"),
                             QStringLiteral("检测")}) {
        auto *pb = new QPushButton(s, toolbar);
        pb->setObjectName(QStringLiteral("vpObserveActionBtn"));
        tl->addWidget(pb, 0, Qt::AlignVCenter);
        observeActionBtns.append(pb);
    }

    if (QStyle *st = QStyleFactory::create(QStringLiteral("Fusion")))
        m_observeCamCombo->setStyle(st);

    const QString popupViewStyle = vpObserveToolbarComboPopupViewStyleSheet();
    const QString popupViewStyle12 =
        popupViewStyle
        + QStringLiteral(" QAbstractItemView, QAbstractItemView::item { font-size: 12px; }"
                         " QAbstractItemView::item { min-height: 30px; }");
    if (QAbstractItemView *v = m_observeCamCombo->view()) {
        v->setStyleSheet(popupViewStyle12);
        if (auto *lv = qobject_cast<QListView *>(v))
            lv->setItemDelegate(new PlainComboPopupDelegate(lv));
    }

    QFont tf = toolbar->font();
    tf.setPixelSize(12);
    tf.setBold(false);
    m_observeCamCombo->setFont(tf);
    snapBtn->setFont(tf);
    liveBtn->setFont(tf);
    for (QAbstractButton *b : m_observeColorGroup->buttons())
        b->setFont(tf);
    QFont tfb = tf;
    tfb.setBold(true);
    for (QPushButton *pb : observeActionBtns)
        pb->setFont(tfb);

    if (QAbstractItemView *v = m_observeCamCombo->view())
        v->setFont(tf);

    pinToolbarCtrlHeight(m_observeCamCombo);
    pinToolbarCtrlHeight(snapBtn);
    pinToolbarCtrlHeight(liveBtn);
    for (QAbstractButton *b : m_observeColorGroup->buttons())
        pinToolbarCtrlHeight(b);
    for (QPushButton *pb : observeActionBtns)
        pinToolbarCtrlHeight(pb);

    tl->addStretch();
    layout->addWidget(toolbar);

    // Preview pane (placeholder + optional graphics view in stack)
    m_previewStack = new QStackedWidget(box);
    auto *placeholderPane = new QWidget(m_previewStack);
    placeholderPane->setAttribute(Qt::WA_StyledBackground, true);
    placeholderPane->setStyleSheet(QStringLiteral("background-color: #000000;"));
    m_previewStack->addWidget(placeholderPane);

    m_graphicsView = PluginManager::getInterface<IGraphicsVew>();
    if (m_graphicsView)
    {
        QWidget *graphicsPane = m_graphicsView->CreateWidget(kVpObservePreviewWinId);
        m_graphicsView->SetVisible(kVpObservePreviewWinId, true, false);
        m_previewStack->addWidget(graphicsPane);
    }

    m_previewStack->setCurrentWidget(placeholderPane);
    m_previewStack->setMinimumHeight(200);
    layout->addWidget(m_previewStack, 1);

    connect(m_observeCamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                if (index < 0 || !m_previewStack)
                    return;
                m_previewStack->setCurrentIndex(0);
                syncPreviewButtonFromCaptureState();
                cam_async::SetWinId(kVpObservePreviewWinId);
            });

    // Result table
    auto *resBox = new QFrame(box);
    resBox->setObjectName(QStringLiteral("vpPanelFrame"));
    resBox->setAttribute(Qt::WA_StyledBackground, true);
    auto *resLayout = new QVBoxLayout(resBox);
    resLayout->setContentsMargins(12, 8, 12, 10);
    resLayout->setSpacing(6);

    auto *resTitle = new QLabel(QStringLiteral("结果输出"), resBox);
    resTitle->setObjectName(QStringLiteral("vpSectionTitle"));
    resLayout->addWidget(resTitle);

    m_resultTable = new QTableWidget(12, 4, resBox);
    m_resultTable->setHorizontalHeaderLabels({QStringLiteral("原始坐标(x,y)"),
                                                QStringLiteral("映射坐标(x',y')"),
                                                QStringLiteral("通道"),
                                                QStringLiteral("检测项")});
    auto *resultHeader = m_resultTable->horizontalHeader();
    resultHeader->setStretchLastSection(true);
    resultHeader->setSectionResizeMode(0, QHeaderView::Interactive);
    resultHeader->setSectionResizeMode(1, QHeaderView::Interactive);
    constexpr int kResultCoordColMinW = 150;
    m_resultTable->setColumnWidth(0, kResultCoordColMinW);
    m_resultTable->setColumnWidth(1, kResultCoordColMinW);
    connect(resultHeader, &QHeaderView::sectionResized, m_resultTable,
            [table = m_resultTable, kResultCoordColMinW](int idx, int, int newSize) {
                if (idx <= 1 && newSize < kResultCoordColMinW)
                    table->setColumnWidth(idx, kResultCoordColMinW);
            });
    m_resultTable->verticalHeader()->setVisible(false);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setAlternatingRowColors(true);
    m_resultTable->setShowGrid(true);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setSelectionMode(QAbstractItemView::SingleSelection);

    const QStringList defects = {QStringLiteral("死灯"), QStringLiteral("暗灯"), QStringLiteral("亮灯"),
                                 QStringLiteral("串色"), QStringLiteral("串灯"), QStringLiteral("混色"),
                                 QStringLiteral("死灯"), QStringLiteral("暗灯"), QStringLiteral("亮灯"),
                                 QStringLiteral("串色"), QStringLiteral("串灯"), QStringLiteral("混色")};
    const QStringList channels = {QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B"),
                                  QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B"),
                                  QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B"),
                                  QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B")};
    for (int r = 0; r < 12; ++r) {
        const auto setC = [&](int c, const QString &t) {
            auto *item = new QTableWidgetItem(t);
            item->setTextAlignment(Qt::AlignCenter);
            m_resultTable->setItem(r, c, item);
        };
        setC(0, QStringLiteral("(%1,0)").arg(r));
        setC(1, QStringLiteral("(%1,127)").arg(r));
        setC(2, channels[r]);
        setC(3, defects[r]);
    }
    resLayout->addWidget(m_resultTable);
    layout->addWidget(resBox);

    // Log
    m_logEdit = new QTextEdit(box);
    m_logEdit->setReadOnly(true);
    m_logEdit->setObjectName(QStringLiteral("logBox"));
    m_logEdit->setFixedHeight(110);
    m_logEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_logEdit, &QTextEdit::customContextMenuRequested, this, [this](const QPoint &pos) {
        if (!m_logEdit)
            return;
        QMenu menu(m_logEdit);
        QAction *clearAct = menu.addAction(QStringLiteral("清屏"));
        if (menu.exec(m_logEdit->mapToGlobal(pos)) == clearAct)
            m_logEdit->clear();
    });
    layout->addWidget(m_logEdit);

    return box;
}

// ── Config panel (right) ─────────────────────────────────────────────────────

QWidget *VirtualPixelView::buildConfigPanel()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *box = new QFrame(scroll);
    box->setObjectName(QStringLiteral("vpPanelFrame"));
    box->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 8, 12, 10);
    layout->setSpacing(10);

    auto *configTitle = new QLabel(QStringLiteral("虚拟像素配置"), box);
    configTitle->setObjectName(QStringLiteral("vpSectionTitle"));
    layout->addWidget(configTitle);

    // 产品配方（与 recipe_config_view 基本信息：坐标系下拉 + 分辨率 Spin 同款控件与样式）
    auto *basicForm = new QWidget(box);
    basicForm->setObjectName(QStringLiteral("vpConfigBasicForm"));
    basicForm->setAttribute(Qt::WA_StyledBackground, true);
    auto *bfLay = new QVBoxLayout(basicForm);
    bfLay->setContentsMargins(0, 0, 0, 0);
    bfLay->setSpacing(0);

    auto *recipeLbl = new QLabel(QStringLiteral("产品配方"), basicForm);
    recipeLbl->setObjectName(QStringLiteral("vpConfigFormLabel"));
    bfLay->addWidget(recipeLbl);
    bfLay->addSpacing(3);

    m_recipeCombo = new VpConfigCamTableComboBox(kVpConfigRecipeComboH, basicForm);
    m_recipeCombo->setObjectName(QStringLiteral("vpConfigRecipeCombo"));
    m_recipeCombo->addItems({QStringLiteral("PANEL-A01"), QStringLiteral("PANEL-A02"), QStringLiteral("PANEL-B03")});
    bfLay->addWidget(m_recipeCombo);

    const QString recipePopupStyle = vpObserveToolbarComboPopupViewStyleSheet();
    const QString recipePopupStyle12 =
        recipePopupStyle
        + QStringLiteral(" QAbstractItemView, QAbstractItemView::item { font-size: 12px; }"
                         " QAbstractItemView::item { min-height: 32px; }");
    if (QAbstractItemView *rv = m_recipeCombo->view()) {
        rv->setStyleSheet(recipePopupStyle12);
        if (auto *lv = qobject_cast<QListView *>(rv))
            lv->setItemDelegate(new PlainComboPopupDelegate(lv));
    }
    QFont rtf = basicForm->font();
    rtf.setPixelSize(12);
    rtf.setBold(false);
    m_recipeCombo->setFont(rtf);
    if (QAbstractItemView *rv = m_recipeCombo->view())
        rv->setFont(rtf);

    layout->addWidget(basicForm);

    m_demoWrapFrames.clear();
    m_mapRuleEdits.clear();
    m_vpLampRowSpins.clear();
    m_vpLampColSpins.clear();
    m_vpActiveCfgChannel = -1;

    m_vpChannelCfg[0] = VirtualPixelChannelConfig{0, 64, 128, QStringLiteral("x' = x"),
                                         QStringLiteral("y' = 127 - y"),
                                         QStringLiteral("index' = y' * 128 + x'")};
    m_vpChannelCfg[1] = VirtualPixelChannelConfig{1, 48, 96, QStringLiteral("x' = x - 16"),
                                         QStringLiteral("y' = 95 - y"),
                                         QStringLiteral("index' = y' * 96 + x'")};
    m_vpChannelCfg[2] = VirtualPixelChannelConfig{2, 32, 64, QStringLiteral("x' = 63 - x"),
                                         QStringLiteral("y' = 63 - y"),
                                         QStringLiteral("index' = y' * 64 + x'")};

    m_channelBtns.clear();

    // R / G / B
    auto *tabBar = new QWidget(box);
    auto *tabRl  = new QHBoxLayout(tabBar);
    tabRl->setContentsMargins(0, 4, 0, 4);
    tabRl->setSpacing(10);

    m_channelGroup = new QButtonGroup(this);
    m_channelGroup->setExclusive(true);
    const QStringList channels = {QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B")};
    for (int i = 0; i < channels.size(); ++i) {
        auto *b = new QPushButton(channels[i], tabBar);
        b->setObjectName(QStringLiteral("vpChannelPill"));
        b->setCheckable(true);
        b->setChecked(i == 0);
        m_channelGroup->addButton(b, i);
        m_channelBtns.append(b);
        tabRl->addWidget(b);
    }
    tabRl->addStretch();
    layout->addWidget(tabBar);
    layout->addSpacing(2);

    // 排列方式示例（置于灯点行列设置上方）
    auto *demoBox = new QFrame(box);
    demoBox->setObjectName(QStringLiteral("vpPanelFrame"));
    demoBox->setAttribute(Qt::WA_StyledBackground, true);
    auto *demoLayout = new QVBoxLayout(demoBox);
    demoLayout->setContentsMargins(12, 8, 12, 10);
    demoLayout->setSpacing(8);

    auto *demoTitle = new QLabel(QStringLiteral("排列方式示例"), demoBox);
    demoTitle->setObjectName(QStringLiteral("vpSectionTitle"));
    demoLayout->addWidget(demoTitle);

    auto *demoHint = new QLabel(
        QStringLiteral("用于说明 ‘正常 / 1-4 / 2-3’ 排布差异，便于打屏后拍照对照。"), demoBox);
    demoHint->setObjectName(QStringLiteral("vpHint"));
    demoHint->setWordWrap(true);
    demoLayout->addWidget(demoHint);

    auto *demoRow = new QHBoxLayout();
    demoRow->setSpacing(10);

    m_demoPickGroup = new QButtonGroup(this);
    m_demoPickGroup->setExclusive(true);

    struct DemoSpec {
        QString title;
        ArrangementDiagram::Mode mode;
    };
    const DemoSpec demos[] = {
        {QStringLiteral("正常排列"), ArrangementDiagram::Normal},
        {QStringLiteral("1-4 排列"), ArrangementDiagram::OneFour},
        {QStringLiteral("2-3 排列"), ArrangementDiagram::TwoThree},
    };

    for (int di = 0; di < 3; ++di) {
        const DemoSpec &ds = demos[di];
        auto *wrap = new QFrame(demoBox);
        wrap->setObjectName(QStringLiteral("vpDemoWrap"));
        auto *vl = new QVBoxLayout(wrap);
        vl->setSpacing(6);

        auto *rb = new QRadioButton(ds.title, wrap);
        rb->setObjectName(QStringLiteral("vpDemoRadio"));
        m_demoPickGroup->addButton(rb, di);
        vl->addWidget(rb);
        vl->addWidget(new ArrangementDiagram(ds.mode, wrap), 0, Qt::AlignHCenter);

        demoRow->addWidget(wrap, 1);
        m_demoWrapFrames.append(wrap);
    }

    connect(m_demoPickGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (m_vpActiveCfgChannel >= 0 && m_vpActiveCfgChannel <= 2)
            m_vpChannelCfg[m_vpActiveCfgChannel].layoutDemoId = id;
        for (int i = 0; i < m_demoWrapFrames.size(); ++i)
            syncDemoWrapActive(m_demoWrapFrames[i], i == id);
    });

    demoLayout->addLayout(demoRow);
    layout->addWidget(demoBox);

    auto *chFrame = new QFrame(box);
    chFrame->setObjectName(QStringLiteral("vpChannelSettings"));
    chFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *chLay = new QVBoxLayout(chFrame);
    chLay->setContentsMargins(8, 2, 8, 2);
    chLay->setSpacing(0);

    m_channelStack = new QStackedWidget(chFrame);
    m_channelStack->setFrameShape(QFrame::NoFrame);
    for (int ch = 0; ch < 3; ++ch) {
        const VpChannelCfgSnap &snap = m_vpChannelCfg[ch];
        VpLampWidgets           lamp = makeVpLampPanel(snap.lampRows, snap.lampCols, chFrame);
        m_vpLampRowSpins.append(lamp.rowSpin);
        m_vpLampColSpins.append(lamp.colSpin);
        connect(lamp.rowSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v) {
            m_vpChannelCfg[ch].lampRows = v;
        });
        connect(lamp.colSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v) {
            m_vpChannelCfg[ch].lampCols = v;
        });
        m_channelStack->addWidget(lamp.panel);
    }
    QWidget *lampPage0 = m_channelStack->widget(0);
    const int lampRowPx = qMax(26, lampPage0->minimumSizeHint().height());
    m_channelStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_channelStack->setFixedHeight(lampRowPx);
    chLay->addWidget(m_channelStack);
    layout->addWidget(chFrame);

    // R/G/B → 堆栈页：与观察区 红/绿/蓝 的联动在 buildLayout 中 connect

    // 坐标映射规则
    auto *mapBox = new QFrame(box);
    mapBox->setObjectName(QStringLiteral("vpPanelFrame"));
    mapBox->setAttribute(Qt::WA_StyledBackground, true);
    auto *mapLayout = new QVBoxLayout(mapBox);
    mapLayout->setContentsMargins(12, 8, 12, 10);
    mapLayout->setSpacing(6);

    auto *mapTitle = new QLabel(QStringLiteral("坐标映射规则"), mapBox);
    mapTitle->setObjectName(QStringLiteral("vpSectionTitle"));
    mapLayout->addWidget(mapTitle);

    for (int li = 0; li < 3; ++li) {
        auto *edit = new QLineEdit(mapBox);
        edit->setObjectName(QStringLiteral("vpMapRule"));
        edit->setReadOnly(true);
        edit->setFrame(true);
        m_mapRuleEdits.append(edit);
        mapLayout->addWidget(edit);
    }

    layout->addWidget(mapBox);
    // 坐标映射规则暂不对用户展示，保留控件与数据绑定以便后续恢复
    mapBox->setVisible(false);

    layout->addSpacing(8);
    m_saveBtn = new QPushButton(QStringLiteral("保存"), box);
    m_saveBtn->setObjectName(QStringLiteral("vpSaveBtn"));
    m_saveBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(m_saveBtn);

    layout->addStretch();

    scroll->setWidget(box);

    syncVpChannelSharedUi(0);
    return scroll;
}

void VirtualPixelView::showSaveSuccessTip()
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
    auto *tipLabel = new QLabel(QStringLiteral("保存成功"), tipDlg);
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

// ── Log API ───────────────────────────────────────────────────────────────────

void VirtualPixelView::appendLog(const QString &text, const QString &level)
{
    if (!m_logEdit)
        return;
    const QString time = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"));
    QString color = QStringLiteral("#c9d1d9");
    if (level == QStringLiteral("INFO"))
        color = QStringLiteral("#4caf50");
    else if (level == QStringLiteral("WARN"))
        color = QStringLiteral("#ff9800");
    else if (level == QStringLiteral("ERROR"))
        color = QStringLiteral("#f44336");

    m_logEdit->append(QStringLiteral("<span style='color:%1;'>[%2] <b>%3</b> %4</span>")
                          .arg(color, time, level, text.toHtmlEscaped()));
}

void VirtualPixelView::revertLiveToggleButton()
{
    QTimer::singleShot(0, this, [this]() { syncPreviewButtonFromCaptureState(); });
}

} // namespace lighten
