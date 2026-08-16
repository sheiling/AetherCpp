/**
 * @file recipe_config_view.cpp
 * @brief RecipeConfigView 布局构建、预览区 ROI 与配方 CRUD 交互实现
 */

#include "recipe_config_view.h"

#include "recipe_config_view_model.h"
#include "command.h"
#include "databinding.h"
#include "viewmodel.h"
#include "lighten_types.h"

#include <QtGlobal>

#include "uibase/thememanager.h"

#include "interface/igraphicsView.h"
#include "extensionsystem/pluginmanager.h"
#include "messagecenter/message_center.h"
#include <common/core/device/camAsync/cam_async_api.h>
#include <common/core/device/camAsync/camera_callback_handler.h>
#include "lighten_param_persist.h"
#include "recipe_result_chip_frame.h"
#include "recipe_step_debug_common.h"
#include "recipe_step_debug_pure_color.h"
#include "recipe_step_debug_string_light.h"
#include "system_config_model.h"

#include <common/core/device/camera/camera_base.h>
#include <common/core/device/deviceManager/device_manager.h>

using namespace ExtensionSystem;

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_icon_button.h"
#include "component/controls/button/themed_push_button.h"
#include "component/font/fa_icon_button.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/dialog/themed_panel_dialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractItemView>
#include <QListView>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStyledItemDelegate>
#include <QStyleOptionSpinBox>
#include <QStyleOptionViewItem>
#include <QLabel>
#include <QFrame>
#include <QStackedWidget>
#include <QScrollArea>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QDateTime>
#include <QSizePolicy>
#include <QString>
#include <QStringLiteral>
#include <QVector>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QMenu>
#include <QEvent>
#include <QPainter>
#include <QMarginsF>
#include <QRectF>
#include <QPointF>
#include <QFont>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOptionComboBox>
#include <QPolygon>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QLayout>
#include <array>
#include <cstring>
#include <QList>
#include <QSignalBlocker>
#include <functional>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <3rdparty/nlohmann/json.hpp>

static QColor recipeThemeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

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

/** 预览工具条单行高度：须与 applyRecipeTheme 中 #recipePreviewToolbar 的 min/max-height(px) 保持一致 */
static constexpr int kRecipePreviewToolbarCtrlH = 26;
static constexpr int kRecipePreviewToolbarIconPx = 12;
/** 特殊检测项表默认行高约 40px；下拉固定高度须与 applyRecipeTheme 中 #recipeSpecialDetectTable QComboBox min/max-height 一致 */
static constexpr int kRecipeSpecTableComboH = 32;
/** 基本信息「坐标系」下拉：12px 字，须与 applyRecipeTheme 中 #recipeBasicForm QComboBox#recipeCamCombo min/max-height 一致 */
static constexpr int kRecipeBasicCoordComboH = 34;

/** 预览工具条 / 特殊检测项表：胶囊轨 + 白圆点（trackHeight 默认工具条行高，表中可用更大值） */
class RecipePillSwitch final : public QWidget
{
public:
    explicit RecipePillSwitch(bool checked, QWidget *parent = nullptr,
                            int trackHeight = kRecipePreviewToolbarCtrlH)
        : QWidget(parent)
        , m_checked(checked)
        , m_trackH(qMax(kRecipePreviewToolbarCtrlH, trackHeight))
    {
        const int w = m_trackH * 2 + 8;
        setFixedSize(w, m_trackH);
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

    void setOnToggled(std::function<void(bool)> handler) { m_onToggled = std::move(handler); }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        const QRectF outer = QRectF(rect());
        const QRectF r     = outer.adjusted(1.0, 1.0, -1.0, -1.0);
        const qreal  rad   = qMax(1.0, r.height() / 2.0);

        const QColor borderOff(0x4a, 0x60, 0x78);
        const QColor borderOn(0x64, 0xb5, 0xf6);
        const QColor offTrack(0x1e, 0x2d, 0x40);
        /** 与 applyRecipeTheme 中 #recipeToolbarActionBtn / #recipeColorBtn 一致 */
        const QColor onTrack(0x1a, 0x4a, 0x8c);

        QPen pen(m_checked ? borderOn : borderOff, 1.0);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(m_checked ? onTrack : offTrack);
        p.drawRoundedRect(r, rad, rad);

        const qreal    strokeW = pen.widthF();
        const QRectF   inner   = r.marginsRemoved(QMarginsF(strokeW * 0.5, strokeW * 0.5, strokeW * 0.5, strokeW * 0.5));
        const qreal    padV    = (m_trackH <= 24) ? 0.5 : 3.0;
        const qreal    padH    = (m_trackH <= 24) ? 1.0 : 3.0;
        const qreal    thumbD = qMax(3.0, inner.height() - 2.0 * padV);
        const qreal    tr     = thumbD * 0.5;
        const qreal    cy     = inner.top() + inner.height() * 0.5;
        const qreal    cx     = m_checked ? (inner.right() - padH - tr) : (inner.left() + padH + tr);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xf5, 0xf8, 0xfc));
        p.drawEllipse(QPointF(cx, cy), tr, tr);
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
            m_checked = !m_checked;
            update();
            if (m_onToggled)
                m_onToggled(m_checked);
        }
        QWidget::mouseReleaseEvent(e);
    }

    QSize sizeHint() const override
    {
        return QSize(m_trackH * 2 + 8, m_trackH);
    }
    QSize minimumSizeHint() const override { return sizeHint(); }

private:
    bool                       m_checked;
    int                        m_trackH;
    std::function<void(bool)> m_onToggled;
};

/** 相机 Tab：样式在 applyRecipeTheme 中 #recipeCamTab（避免根 QPushButton 规则覆盖） */
class RecipeCamTabButton final : public QPushButton
{
public:
    static constexpr int kTabH = 20;

    explicit RecipeCamTabButton(const QString &label, QWidget *parent = nullptr)
        : QPushButton(label, parent)
        , m_label(label)
    {
        setObjectName(QStringLiteral("recipeCamTab"));
        setCheckable(true);
        setFlat(true);
        setAutoDefault(false);
        setDefault(false);
        setFocusPolicy(Qt::NoFocus);

        QFont f = font();
        f.setPixelSize(10);
        f.setBold(true);
        setFont(f);

        setFixedHeight(kTabH);
        {
            const QFontMetrics fmw(font());
            const int textW = fmw.horizontalAdvance(m_label);
            setMinimumWidth(qMax(textW + 32, 72));
        }
        QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed);
        setSizePolicy(sp);
    }

    QSize sizeHint() const override
    {
        const QFontMetrics fm(font());
        const int textW = fm.horizontalAdvance(m_label);
        return QSize(qMax(textW + 32, minimumWidth()), kTabH);
    }

    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QPushButton::resizeEvent(event);
        if (height() != kTabH)
            setFixedHeight(kTabH);
    }

private:
    QString m_label;
};

namespace {

/** 特殊检测项表内开关：与系统配置检测表一致，占满单元格垂直居中 */
constexpr int kRecipeSpecSwitchTrackH = 26;

static QWidget *makeSpecialDetectSwitchCell(bool checked, QWidget *parent,
                                            RecipePillSwitch **switchOut = nullptr)
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
    auto *sw = new RecipePillSwitch(checked, cell, kRecipeSpecSwitchTrackH);
    if (switchOut)
        *switchOut = sw;
    h->addWidget(sw, 0, Qt::AlignVCenter);
    h->addStretch(1);
    v->addLayout(h);
    v->addStretch(1);
    return cell;
}

/** 灯板坐标系图示：8 种轴朝向，与「坐标系」下拉 currentIndex 0..7 一一对应（与目标图两行各四格顺序一致） */
class RecipeCoordIllustration final : public QWidget
{
public:
    explicit RecipeCoordIllustration(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("recipeBasicIllus"));
        setAttribute(Qt::WA_StyledBackground, true);
        setMinimumHeight(168);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    }

    void setCoordIndex(int idx)
    {
        idx = qBound(0, idx, 7);
        if (m_idx == idx)
            return;
        m_idx = idx;
        update();
    }

    int coordIndex() const { return m_idx; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_idx = 0;
};

void RecipeCoordIllustration::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QRectF r = QRectF(rect()).adjusted(6, 6, -6, -6);
    const qreal    len = qMax(32.0, qMin(r.width(), r.height()) * 0.52);

    struct Spec
    {
        Qt::Corner corner;
        QPointF    xDir;
        QPointF    yDir;
    };
    static const Spec kSpecs[8] = {
        {Qt::TopLeftCorner,     {1, 0}, {0, 1}},
        {Qt::TopRightCorner,    {0, 1}, {-1, 0}},
        {Qt::BottomLeftCorner,  {-1, 0}, {0, -1}},
        {Qt::BottomRightCorner, {0, -1}, {1, 0}},
        {Qt::TopLeftCorner,     {0, 1}, {1, 0}},
        {Qt::TopRightCorner,    {-1, 0}, {0, 1}},
        {Qt::BottomLeftCorner,  {0, -1}, {-1, 0}},
        {Qt::BottomRightCorner, {1, 0}, {0, -1}},
    };

    auto cornerPt = [&r](Qt::Corner c) -> QPointF {
        switch (c) {
        case Qt::TopLeftCorner:
            return r.topLeft();
        case Qt::TopRightCorner:
            return r.topRight();
        case Qt::BottomLeftCorner:
            return r.bottomLeft();
        case Qt::BottomRightCorner:
            return r.bottomRight();
        default:
            return r.center();
        }
    };

    auto drawAxis = [&p](const QPointF &o, QPointF dir, qreal L, const QString &tag) {
        const qreal dl = QLineF(QPointF(), dir).length();
        if (dl < 1e-6)
            return;
        dir /= dl;
        const QPointF e = o + dir * L;
        QPen axisPen(QColor(0xf0, 0xf4, 0xf8), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(axisPen);
        p.drawLine(o, e);
        const qreal ah = qBound(6.0, L * 0.12, 12.0);
        const QPointF back = -dir * ah;
        QPointF       perp(-dir.y(), dir.x());
        perp *= ah * 0.45;
        p.setBrush(QColor(0xf0, 0xf4, 0xf8));
        p.setPen(Qt::NoPen);
        p.drawPolygon(QPolygonF() << e << (e + back + perp) << (e + back - perp));
        p.setPen(QColor(0xc5, 0xce, 0xd9));
        p.setBrush(Qt::NoBrush);
        QFont f = p.font();
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        QPointF mid = o + dir * (L * 0.52);
        QPointF tOff(-dir.y(), dir.x());
        const qreal tl = QLineF(QPointF(), tOff).length();
        if (tl > 1e-3)
            tOff = tOff / tl * 13.0;
        p.drawText(mid + tOff, tag);
    };

    const Spec &sp = kSpecs[m_idx];
    QPointF      xd = sp.xDir;
    QPointF      yd = sp.yDir;
    const qreal  lx = QLineF(QPointF(), xd).length();
    const qreal  ly = QLineF(QPointF(), yd).length();
    if (lx > 1e-6)
        xd /= lx;
    if (ly > 1e-6)
        yd /= ly;

    const QPointF O  = cornerPt(sp.corner);
    const QPointF P1 = O + xd * len;
    const QPointF P2 = O + yd * len;
    qreal           minx = qMin(O.x(), qMin(P1.x(), P2.x()));
    qreal           maxx = qMax(O.x(), qMax(P1.x(), P2.x()));
    qreal           miny = qMin(O.y(), qMin(P1.y(), P2.y()));
    qreal           maxy = qMax(O.y(), qMax(P1.y(), P2.y()));
    QRectF          bbox(minx, miny, maxx - minx, maxy - miny);
    bbox.adjust(-16, -16, 16, 16); // 箭头与 x/y 文字余量
    const QPointF delta = r.center() - bbox.center();
    const QPointF Odraw = O + delta;

    drawAxis(Odraw, sp.xDir, len, QStringLiteral("x"));
    drawAxis(Odraw, sp.yDir, len, QStringLiteral("y"));
}

/** 下拉列表只画文字，去掉当前项前的勾选/指示（部分样式会在当前项画勾） */
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
        // Fusion + QSS 时 delegate 默认高度常不含 ::item 的 padding（6px 上下）
        const QFontMetrics fm(option.font);
        const int minH = fm.lineSpacing() + 12;
        sh.setHeight(qMax(sh.height(), qMax(minH, 30)));
        return sh;
    }
};

/** 按实际条目数收缩/展开下拉弹出层高度；超出 maxVisibleItems 时保留滚动 */
static void adjustToolbarComboPopupHeight(QComboBox *combo, QWidget *popup, QAbstractItemView *view)
{
    if (!combo || !popup || !view)
        return;
    QAbstractItemModel *model = view->model();
    if (!model)
        return;
    const int itemCount = combo->count();
    if (itemCount <= 0)
        return;

    if (auto *lv = qobject_cast<QListView *>(view))
        lv->setSpacing(0);

    popup->setMinimumHeight(0);
    popup->setMaximumHeight(QWIDGETSIZE_MAX);
    view->setMinimumHeight(0);
    view->setMaximumHeight(QWIDGETSIZE_MAX);

    const int maxVisible  = combo->maxVisibleItems();
    const int visibleRows = qMin(itemCount, maxVisible);

    int contentH = 0;
    for (int i = 0; i < visibleRows; ++i)
        contentH += qMax(view->sizeHintForRow(i), 0);
    if (contentH <= 0) {
        const QFontMetrics fm(view->font());
        contentH = visibleRows * qMax(fm.lineSpacing() + 12, 30);
    }

    // QSS border 1px 上下；frameWidth() 在纯 QSS 边框时常为 0
    const int borderSlack = qMax(view->frameWidth() * 2, 2);
    const int viewH       = contentH + borderSlack;

    view->setFixedHeight(viewH);
    view->setVerticalScrollBarPolicy(itemCount > visibleRows ? Qt::ScrollBarAsNeeded
                                                             : Qt::ScrollBarAlwaysOff);

    if (QLayout *ly = popup->layout()) {
        ly->setContentsMargins(0, 0, 0, 0);
        ly->setSpacing(0);
    }
    if (popup->layout())
        popup->layout()->activate();
    popup->setFixedHeight(viewH);
}

/** 预览工具条内下拉：弹出宽度与框一致、与框底留 2px（弹出层不在 #lightenRecipeRoot 下，需在 show 后调整） */
class PreviewToolBarComboBox : public QComboBox
{
public:
    explicit PreviewToolBarComboBox(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
        if (QAbstractItemView *v = view()) {
            v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            if (auto *lv = qobject_cast<QListView *>(v))
                lv->setSpacing(0);
        }
    }

protected:
    void showPopup() override
    {
        m_savedMaxVisibleItems = maxVisibleItems();
        if (const int n = count(); n > 0)
            setMaxVisibleItems(qMin(n, m_savedMaxVisibleItems));

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
        QTimer::singleShot(0, this, [this, popup, v] {
            const int w = width();
            const QPoint below = mapToGlobal(QPoint(0, height()));
            popup->move(below.x(), below.y() + 2);
            if (QLayout *ly = popup->layout())
                ly->setContentsMargins(0, 0, 0, 0);
            popup->setFixedWidth(w);
            adjustToolbarComboPopupHeight(this, popup, v);
        });
    }

    void hidePopup() override
    {
        QComboBox::hidePopup();
        setMaxVisibleItems(m_savedMaxVisibleItems);
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

private:
    int m_savedMaxVisibleItems = 10;
};

/** 参数表「旋转角度」：与调试区 PreviewToolBarComboBox 同款箭头/弹出逻辑，固定行内高度、可调最小宽度 */
class RecipeCamTableComboBox final : public PreviewToolBarComboBox
{
public:
    explicit RecipeCamTableComboBox(int pixelH, QWidget *parent = nullptr)
        : PreviewToolBarComboBox(parent)
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
        QSize s = PreviewToolBarComboBox::sizeHint();
        s.setHeight(m_h);
        return s;
    }

    QSize minimumSizeHint() const override
    {
        QSize s = PreviewToolBarComboBox::minimumSizeHint();
        s.setHeight(m_h);
        return s;
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        PreviewToolBarComboBox::resizeEvent(event);
        clampHeight();
    }

    void changeEvent(QEvent *event) override
    {
        PreviewToolBarComboBox::changeEvent(event);
        if (event->type() == QEvent::StyleChange || event->type() == QEvent::FontChange)
            clampHeight();
    }

private:
    void clampHeight() { setFixedHeight(m_h); }

    int m_h;
};

/** 与 PreviewToolBarComboBox::paintEvent 中三角、箭头区底色一致，覆盖 Spin 默认箭头 */
static void paintComboLikeSpinArrows(QAbstractSpinBox *spin, const QStyleOptionSpinBox &opt)
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

class RecipeComboStyleSpinBox final : public QSpinBox
{
public:
    explicit RecipeComboStyleSpinBox(QWidget *parent = nullptr)
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
        paintComboLikeSpinArrows(this, opt);
    }
};

class RecipeComboStyleDoubleSpinBox final : public QDoubleSpinBox
{
public:
    explicit RecipeComboStyleDoubleSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent)
    {
        if (QStyle *fus = QStyleFactory::create(QStringLiteral("Fusion")))
            setStyle(fus);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QDoubleSpinBox::paintEvent(event);
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        paintComboLikeSpinArrows(this, opt);
    }
};

/** 下拉弹出层为独立窗口，不能依赖 #lightenRecipeRoot；挂在 view 上。
 *  与 #recipePreviewToolbar QComboBox 的 padding-left/right(6px) 一致；行高由 QSS + delegate::sizeHint 共同保证不裁字、不过分增高。 */
static QString previewToolbarComboPopupViewStyleSheet()
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

static QTableWidgetItem *recipeTableLabelItem(const QString &text)
{
    auto *it = new QTableWidgetItem(text);
    it->setTextAlignment(Qt::AlignCenter);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    return it;
}

static void initRecipeGridTable(QTableWidget *table)
{
    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);
    table->setFrameShape(QFrame::NoFrame);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(false);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setHighlightSections(false);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

static void setRecipeTableFixedHeight(QTableWidget *table, int dataRows, int pad = 12)
{
    const int rowH    = table->verticalHeader()->defaultSectionSize();
    const int headerH = qMax(32, table->horizontalHeader()->sizeHint().height());
    table->setFixedHeight(headerH + dataRows * rowH + pad);
}

static void styleRecipeComboPopup(QComboBox *combo, int fontPx, const QString &viewExtra = QString())
{
    if (!combo)
        return;
    QString sheet = previewToolbarComboPopupViewStyleSheet();
    if (!viewExtra.isEmpty())
        sheet += viewExtra;
    if (QAbstractItemView *v = combo->view()) {
        v->setStyleSheet(sheet);
        if (auto *lv = qobject_cast<QListView *>(v))
            lv->setItemDelegate(new PlainComboPopupDelegate(lv));
        QFont tf = combo->font();
        tf.setPixelSize(fontPx);
        tf.setBold(false);
        combo->setFont(tf);
        v->setFont(tf);
    }
}

static RecipeComboStyleSpinBox *makeRecipeIntSpin(QWidget *parent, int minVal, int maxVal)
{
    auto *spin = new RecipeComboStyleSpinBox(parent);
    spin->setObjectName(QStringLiteral("recipeCamSpin"));
    spin->setRange(minVal, maxVal);
    spin->setAlignment(Qt::AlignCenter);
    return spin;
}

static RecipeComboStyleDoubleSpinBox *makeRecipeDoubleSpin(QWidget *parent, double minVal,
                                                           double maxVal)
{
    auto *spin = new RecipeComboStyleDoubleSpinBox(parent);
    spin->setObjectName(QStringLiteral("recipeCamSpin"));
    spin->setRange(minVal, maxVal);
    spin->setDecimals(2);
    spin->setSingleStep(0.1);
    spin->setAlignment(Qt::AlignCenter);
    return spin;
}

static RecipeCamTableComboBox *makeRecipeTableCombo(int rowH, QWidget *parent,
                                                    const QStringList &items, int fontPx = 11,
                                                    const QString &viewExtra = QString())
{
    auto *sel = new RecipeCamTableComboBox(rowH, parent);
    sel->setObjectName(QStringLiteral("recipeCamCombo"));
    sel->addItems(items);
    styleRecipeComboPopup(sel, fontPx, viewExtra);
    return sel;
}

/** 图像质量分析项：点击切换 OK / NG */
class RecipeQualityLabel final : public QLabel
{
public:
    explicit RecipeQualityLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);
        setObjectName(QStringLiteral("recipeCamQualityCell"));
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);
    }

    void setQualityOk(bool ok)
    {
        setText(ok ? QStringLiteral("OK") : QStringLiteral("NG"));
        setStyleSheet(ok ? QStringLiteral("color:#69f0ae; font-weight:bold; font-size:13px; background:transparent;")
                         : QStringLiteral("color:#ff5252; font-weight:bold; font-size:13px; background:transparent;"));
    }

    void setOnToggled(std::function<void()> cb) { m_onToggled = std::move(cb); }

protected:
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && isEnabled() && rect().contains(e->pos()) && m_onToggled)
            m_onToggled();
        QLabel::mouseReleaseEvent(e);
    }

private:
    std::function<void()> m_onToggled;
};

template<typename Getter, typename Setter>
void bindRecipePillSwitch(ViewModel *vm, const QString &prop, Getter get, Setter set, RecipePillSwitch *sw)
{
    if (!sw || !vm || !vm->hasProperty(prop))
        return;

    using RD = lighten::RecipeDocument;

    sw->setChecked(get(vm->getProperty(prop).value<RD>()));
    QObject::connect(vm, &ViewModel::propertyChanged, sw,
                     [sw, prop, get](const QString &name, const QVariant &val) {
                         if (name == prop)
                             sw->setChecked(get(val.value<RD>()));
                     });
    sw->setOnToggled([vm, prop, set](bool checked) {
        RD doc = vm->getProperty(prop).value<RD>();
        set(doc, checked);
        vm->setProperty(prop, QVariant::fromValue(doc));
    });
}

template<typename Getter, typename Setter>
void bindStructComboIndex(ViewModel *vm, const QString &prop, Getter get, Setter set, QComboBox *combo)
{
    if (!combo || !vm || !vm->hasProperty(prop))
        return;

    using RD = lighten::RecipeDocument;

    combo->setCurrentIndex(get(vm->getProperty(prop).value<RD>()));
    QObject::connect(vm, &ViewModel::propertyChanged, combo,
                     [combo, prop, get](const QString &name, const QVariant &val) {
                         if (name == prop) {
                             QSignalBlocker blocker(combo);
                             combo->setCurrentIndex(get(val.value<RD>()));
                         }
                     });
    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), vm,
                     [vm, prop, set](int index) {
                         if (index < 0)
                             return;
                         RD doc = vm->getProperty(prop).value<RD>();
                         set(doc, index);
                         vm->setProperty(prop, QVariant::fromValue(doc));
                     });
}

template<typename Getter, typename Setter>
void bindStructButtonGroupIndex(ViewModel *vm, const QString &prop, Getter get, Setter set,
                                QButtonGroup *group)
{
    if (!group || !vm || !vm->hasProperty(prop))
        return;

    using RD = lighten::RecipeDocument;

    if (QAbstractButton *btn = group->button(get(vm->getProperty(prop).value<RD>())))
        btn->setChecked(true);
    QObject::connect(vm, &ViewModel::propertyChanged, group,
                     [group, prop, get](const QString &name, const QVariant &val) {
                         if (name == prop) {
                             QSignalBlocker blocker(group);
                             if (QAbstractButton *btn = group->button(get(val.value<RD>())))
                                 btn->setChecked(true);
                         }
                     });
    QObject::connect(group,
                     static_cast<void (QButtonGroup::*)(QAbstractButton *)>(
                         &QButtonGroup::buttonClicked),
                     vm, [vm, prop, set, group](QAbstractButton *btn) {
                         const int id = group->id(btn);
                         if (id < 0)
                             return;
                         RD doc = vm->getProperty(prop).value<RD>();
                         set(doc, id);
                         vm->setProperty(prop, QVariant::fromValue(doc));
                     });
}

} // namespace

namespace {
constexpr int kRecipePreviewWinId = 1;

using common::imgproc::ImageFormat;

ImageFormat imageFormatFromPixelFormatString(const QString &pixelFormat)
{
    static const struct {
        const char *name;
        ImageFormat format;
    } kPixelFormatTable[] = {
        {"Mono8", ImageFormat::MONO8},
        {"Mono10", ImageFormat::MONO10},
        {"Mono12", ImageFormat::MONO12},
        {"Mono14", ImageFormat::MONO14},
        {"Mono16", ImageFormat::MONO16},
        {"BayerGR8", ImageFormat::BAYER_GR8},
        {"BayerRG8", ImageFormat::BAYER_RG8},
        {"BayerGB8", ImageFormat::BAYER_GB8},
        {"BayerBG8", ImageFormat::BAYER_BG8},
        {"BayerGR10", ImageFormat::BAYER_GR10},
        {"BayerRG10", ImageFormat::BAYER_RG10},
        {"BayerGB10", ImageFormat::BAYER_GB10},
        {"BayerBG10", ImageFormat::BAYER_BG10},
        {"BayerGR12", ImageFormat::BAYER_GR12},
        {"BayerRG12", ImageFormat::BAYER_RG12},
        {"BayerGB12", ImageFormat::BAYER_GB12},
        {"BayerBG12", ImageFormat::BAYER_BG12},
        {"BayerGR16", ImageFormat::BAYER_GR16},
        {"BayerRG16", ImageFormat::BAYER_RG16},
        {"BayerGB16", ImageFormat::BAYER_GB16},
        {"BayerBG16", ImageFormat::BAYER_BG16},
        {"RGB8", ImageFormat::RGB24},
        {"BGR8", ImageFormat::BGR24},
    };

    for (const auto &entry : kPixelFormatTable) {
        if (pixelFormat.compare(QString::fromLatin1(entry.name), Qt::CaseInsensitive) == 0)
            return entry.format;
    }
    return ImageFormat::UNKNOWN;
}

int elemSizeFromImageFormat(ImageFormat format)
{
    return common::imgproc::DepthBytes(common::imgproc::ImageFormatToPixelDepth(format));
}

bool resolveRawFrameMeta(const QString &cameraName, int &width, int &height, ImageFormat &format)
{
    lighten::CameraImageConfig config;
    if (!lighten::DeviceConfigReader::loadCameraImageConfig(cameraName, &config))
        return false;

    width  = config.xResolution;
    height = config.yResolution;
    format = imageFormatFromPixelFormatString(config.pixelFormat);
    return width > 0 && height > 0 && format != ImageFormat::UNKNOWN;
}

QImage loadRawFileToQImage(const QString &filePath, const QString &cameraName, QString *errorOut = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorOut)
            *errorOut = QStringLiteral("无法打开 RAW 文件: %1").arg(filePath);
        qWarning() << "[RecipeConfigView] failed to open raw file:" << filePath;
        return QImage();
    }

    const QByteArray rawData = file.readAll();
    if (rawData.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("RAW 文件为空: %1").arg(filePath);
        qWarning() << "[RecipeConfigView] empty raw file:" << filePath;
        return QImage();
    }

    int width  = 0;
    int height = 0;
    ImageFormat format = ImageFormat::UNKNOWN;
    if (!resolveRawFrameMeta(cameraName, width, height, format)) {
        if (errorOut)
            *errorOut = QStringLiteral("无法加载相机 \"%1\" 的图像配置").arg(cameraName);
        qWarning() << "[RecipeConfigView] failed to load camera image config for:" << cameraName;
        return QImage();
    }

    const auto *const data = reinterpret_cast<const unsigned char *>(rawData.constData());
    const QImage image = cam_async::ConvertCameraBufferToQImage(
        height, width, data, rawData.size(), format);
    if (image.isNull() && errorOut)
        *errorOut = QStringLiteral("RAW 图像解码失败（尺寸或格式不匹配）: %1").arg(filePath);
    return image;
}

} // namespace

/** @brief RecipeConfigView 成员实现（布局、预览 ROI、配方 CRUD 与数据绑定） */
namespace lighten {

namespace {

int resultChipSlotIndex(DetectItemKind kind)
{
    switch (kind) {
    case DetectItemKind::DeadLed:
        return 0;
    case DetectItemKind::DimLed:
        return 1;
    case DetectItemKind::BrightLed:
        return 2;
    case DetectItemKind::ColorVariation:
        return 3;
    case DetectItemKind::ColorMapDead:
        return 4;
    case DetectItemKind::ColorMapLive:
        return 5;
    case DetectItemKind::MixedColor:
        return 6;
    case DetectItemKind::BlackScreenOn:
        return 7;
    default:
        return -1;
    }
}

} // namespace

RecipeConfigView::RecipeConfigView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("lighten.recipe_config"));
    setPageTitle(QStringLiteral("产品配方配置"));
}

RecipeConfigView::~RecipeConfigView()
{
    MessageCenter::Center::Detach(CAM_ASYNC_EVENT_CAMERA_IMG, this, SLOT(slot_camera_img(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_CAMERA_STATE, this, SLOT(slot_camera_state(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_DEBUG_NORMAL_DEFECT_COUNT,
                                  this,
                                  SLOT(slot_debug_pure_color_defect_count(const QVariantList &)));
}

void RecipeConfigView::onLoad()
{
    applyRecipeTheme();
    buildLayout();
    setupViewModel();

    MessageCenter::Center::Attach(CAM_ASYNC_EVENT_CAMERA_IMG, this, SLOT(slot_camera_img(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_CAMERA_STATE, this, SLOT(slot_camera_state(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_DEBUG_NORMAL_DEFECT_COUNT,
                                  this,
                                  SLOT(slot_debug_pure_color_defect_count(const QVariantList &)));
}

void RecipeConfigView::syncPreviewButtonFromCaptureState()
{
    if (!m_toolbarCamCombo || !m_liveBtn)
        return;

    const auto dm = tryGetDeviceManager();
    if (!dm)
        return;

    const auto camera = dm->CameraHandler(m_toolbarCamCombo->currentText().toStdString());
    if (!camera)
        return;

    const auto state = camera->GetCaptureState();

    if (m_snapBtn)
        m_snapBtn->setEnabled(state == common::device::CaptureState::Idle);

    switch (state) {
    case common::device::CaptureState::Idle:
        m_liveBtn->setEnabled(true);
        ui::applyFaIcon(m_liveBtn, ui::FaIcon::Play, kRecipePreviewToolbarIconPx);
        m_liveBtn->setToolTip(QStringLiteral("预览"));
        break;
    case common::device::CaptureState::Preview:
        m_liveBtn->setEnabled(true);
        ui::applyFaIcon(m_liveBtn, ui::FaIcon::Stop, kRecipePreviewToolbarIconPx);
        m_liveBtn->setToolTip(QStringLiteral("停止"));
        break;
    case common::device::CaptureState::Capturing:
        m_liveBtn->setEnabled(false);
        ui::applyFaIcon(m_liveBtn, ui::FaIcon::Play, kRecipePreviewToolbarIconPx);
        m_liveBtn->setToolTip(QStringLiteral("预览"));
        break;
    }
}

void RecipeConfigView::slot_camera_img(const QVariantList &args)
{
    if (args.size() < 3)
        return;

    const int winId = args.at(0).toInt();
    if (winId != kRecipePreviewWinId)
        return;

    const QString cameraName = args.at(1).toString();
    if (!m_toolbarCamCombo || m_toolbarCamCombo->currentText() != cameraName)
        return;

    const QImage image = qvariant_cast<QImage>(args.at(2));

    if (image.isNull()) {
        appendLog(QStringLiteral("图像转换失败"), QStringLiteral("ERROR"));
        syncPreviewButtonFromCaptureState();
        return;
    }

    showPreviewImage(image);
    syncPreviewButtonFromCaptureState();
}

void RecipeConfigView::slot_camera_state(const QVariantList &args)
{
    if (args.size() < 2)
        return;

    const QString cameraSn = args.at(0).toString();
    if (cameraSn.isEmpty() || !m_toolbarCamCombo || !m_liveBtn)
        return;

    const auto dm = tryGetDeviceManager();
    if (!dm)
        return;

    const auto camera = dm->CameraHandler(m_toolbarCamCombo->currentText().toStdString());
    if (!camera)
        return;

    const QString currentSn = QString::fromStdString(camera->GetCameraInfo().cameraID);
    if (currentSn != cameraSn)
        return;

    syncPreviewButtonFromCaptureState();
}

void RecipeConfigView::applyTheme()
{
    applyRecipeTheme();
}

void RecipeConfigView::applyRecipeTheme()
{
    setObjectName(QStringLiteral("lightenRecipeRoot"));
    setAttribute(Qt::WA_StyledBackground, true);

    const QString kRecipePanelBg =
        recipeThemeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a"))).name();
    const QString kRecipePanelBr =
        recipeThemeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668"))).name();

    setStyleSheet(QStringLiteral(
        "#lightenRecipeRoot { background: transparent; }"

        "#lightenRecipeRoot QLabel#recipeSectionTitle {"
        "  color: #ffffff; font-size: 13px; font-weight: 700;"
        "  background: transparent;"
        "}"

        "#lightenRecipeRoot QFrame#recipeCenterShell {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"

        "#lightenRecipeRoot QFrame#recipePreviewToolbar {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QLabel#recipeRoiSwitchLabel {"
        "  color: #f0f4f8; font-size: 12px; background: transparent;"
        "  min-height: 26px; max-height: 26px; padding: 0px; margin: 0px;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QWidget#recipeRoiSwitchWrap {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QComboBox {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 26px; max-height: 26px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QComboBox:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeColorBtn {"
        "  min-height: 26px; max-height: 26px; min-width: 20px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px;"
        "  margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "  font-size: 12px; font-weight: normal;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeColorBtn:checked {"
        "  background-color: #1a3550; color: #ffffff;"
        "  border: 1px solid #42a5f5;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeColorBtn:hover:!checked {"
        "  background-color: #263a52; border: 1px solid #6b8cae;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeToolbarActionBtn {"
        "  min-height: 26px; max-height: 26px; min-width: 72px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 8px; padding-right: 8px;"
        "  margin: 0px;"
        "  background-color: #1a4a8c; color: #ffffff;"
        "  border: 1px solid #64b5f6; border-radius: 5px;"
        "  font-weight: bold;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeToolbarActionBtn:hover {"
        "  background-color: #1e5bb0; border: 1px solid #90caf9;"
        "}"
        "#lightenRecipeRoot QFrame#recipePreviewToolbar QPushButton#recipeToolbarActionBtn:pressed {"
        "  background-color: #153a75;"
        "}"

        "#lightenRecipeRoot QFrame#recipeTopActionShell {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"
        "#lightenRecipeRoot QFrame#recipeTopActionShell QPushButton {"
        "  min-width: 88px;"
        "  min-height: 22px;"
        "  max-height: 24px;"
        "  padding: 2px 16px;"
        "}"

        "#lightenRecipeRoot QPushButton {"
        "  min-height: 26px; padding: 2px 10px;"
        "  background-color: #184c86; color: #e8eef6;"
        "  border: 1px solid #2f82da; border-radius: 4px;"
        "  font-size: 12px;"
        "}"
        "#lightenRecipeRoot QPushButton:hover { background-color: #1f5ea2; }"
        "#lightenRecipeRoot QPushButton:pressed { background-color: #17477b; }"
        "#lightenRecipeRoot QPushButton#dangerBtn {"
        "  background-color: #3a1f24; color: #ffcdd2;"
        "  border: 2px solid #e53935;"
        "}"
        "#lightenRecipeRoot QPushButton#dangerBtn:hover { background-color: #4a252c; }"

        "#lightenRecipeRoot QPushButton#recipeToolIconBtn {"
        "  min-width: 26px; max-width: 26px; min-height: 26px; max-height: 26px;"
        "  padding: 0px; margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeToolIconBtn:checked {"
        "  background-color: #1a3550; color: #ffffff;"
        "  border: 1px solid #42a5f5; font-weight: bold;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeToolIconBtn:hover:!checked {"
        "  background-color: #263a52; border: 1px solid #6b8cae;"
        "}"

        "#lightenRecipeRoot QPushButton#recipeColorBtn {"
        "  min-height: 22px; max-height: 22px; min-width: 36px;"
        "  background-color: #24364d; color: #dbe4ee;"
        "  border: 1px solid #4a5f79; border-radius: 4px; font-size: 11px;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeColorBtn:checked {"
        "  background-color: #1565c0; color: #ffffff;"
        "  border: 1px solid #42a5f5; font-weight: bold;"
        "}"

        "#lightenRecipeRoot QPushButton#recipeStepBtn {"
        "  min-height: 30px; font-weight: bold;"
        "  background-color: #184c86; color: #ffffff;"
        "  border: 1px solid #2f82da; border-radius: 4px;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeStepBtn:hover { background-color: #1f5ea2; }"

        "#lightenRecipeRoot QPushButton#collapsibleHeader {"
        "  background-color: #2a3544; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "#lightenRecipeRoot QPushButton#collapsibleHeader:hover { background-color: #323e50; }"

        "#lightenRecipeRoot QPushButton#recipeCamTab {"
        "  min-height: 20px; max-height: 20px;"
        "  padding: 0px 10px;"
        "  margin: 0px;"
        "  border-radius: 10px;"
        "  background-color: #1e2d40;"
        "  color: #b8d4f0;"
        "  border: 1px solid #4a6078;"
        "  font-weight: bold;"
        "  font-size: 10px;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeCamTab:checked {"
        "  background-color: #1a3550;"
        "  color: #ffffff;"
        "  border: 1px solid #42a5f5;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeCamTab:hover:!checked {"
        "  background-color: #263a52;"
        "  color: #dbe4ee;"
        "  border: 1px solid #6b8cae;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeCamTab:checked:hover {"
        "  background-color: #1e4a75;"
        "  border: 1px solid #64b5f6;"
        "}"
        "#lightenRecipeRoot QPushButton#recipeCamTab:pressed {"
        "  background-color: #152a3d;"
        "}"

        "#lightenRecipeRoot QLabel#pillActive, #lightenRecipeRoot QLabel#pill {"
        "  color: #dbe4ee; font-size: 12px; padding: 4px 10px; border-radius: 4px;"
        "  background-color: #1f2d40; border: 1px solid #4a5f79;"
        "}"
        "#lightenRecipeRoot QLabel#pillActive {"
        "  background-color: #1a3a5c; border: 1px solid #2f82da; color: #ffffff;"
        "}"

        "#lightenRecipeRoot QFrame#recipeListGroup {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"

        "#lightenRecipeRoot QFrame#recipeListGroup QLineEdit#recipeListSearch {"
        "  background-color: transparent; color: #ffffff;"
        "  border: 1px solid %2; border-radius: 4px;"
        "  padding: 6px 8px; min-height: 20px;"
        "}"
        "#lightenRecipeRoot QFrame#recipeListGroup QLineEdit#recipeListSearch::placeholder {"
        "  color: #7a8699;"
        "}"

        "#lightenRecipeRoot QListWidget#recipeListWidget {"
        "  background-color: transparent; color: #ffffff;"
        "  border: 1px solid %2; border-radius: 4px;"
        "  outline: none; padding: 0px;"
        "}"
        "#lightenRecipeRoot QListWidget#recipeListWidget::item {"
        "  color: #ffffff;"
        "  background-color: transparent;"
        "  border: none;"
        "  border-bottom: 1px solid #334155;"
        "  padding: 0px;"
        "  min-height: 34px;"
        "}"
        "#lightenRecipeRoot QListWidget#recipeListWidget::item:selected {"
        "  background-color: #1b3a5a;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-bottom: 1px solid #334155;"
        "}"
        "#lightenRecipeRoot QListWidget#recipeListWidget::item:hover:!selected {"
        "  background-color: #233548;"
        "}"

        "#lightenRecipeRoot QFrame#recipeListGroup QLabel#recipeListName {"
        "  color: #ffffff; font-size: 12px; background: transparent;"
        "}"
        "#lightenRecipeRoot QFrame#recipeListGroup QLabel#recipeListTag {"
        "  color: #c5ced9; font-size: 11px; background: transparent;"
        "}"

        "#lightenRecipeRoot QTextEdit#logBox {"
        "  background-color: #161c24;"
        "  color: #dbe4ee;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "  font-family: 'Consolas','Microsoft YaHei UI','Microsoft YaHei',monospace;"
        "  font-size: 11px;"
        "}"
        "#lightenRecipeRoot QScrollBar:vertical {"
        "  background: #1a222c; width: 10px; margin: 0;"
        "}"
        "#lightenRecipeRoot QScrollBar::handle:vertical {"
        "  background: #3a4a5d; min-height: 24px; border-radius: 4px;"
        "}"

        "#lightenRecipeRoot QFrame#recipeResultGroup {"
        "  background-color: #1f2d40;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "}"
        "#lightenRecipeRoot QWidget#recipeResultChipArea {"
        "  background-color: transparent;"
        "}"

        "#lightenRecipeRoot QFrame#recipeParamPanel {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"

        "#lightenRecipeRoot QFrame#recipeParamSection {"
        "  background-color: #1f2d40;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "}"
        "#lightenRecipeRoot QWidget#recipeParamSectionsWrap {"
        "  background-color: transparent;"
        "}"

        "#lightenRecipeRoot QWidget#recipeBasicForm {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicResRow {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicForm QLabel#recipeBasicFormLabel {"
        "  color: #9eb8d4; font-size: 11px; background: transparent;"
        "  padding: 0px; margin: 0px;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicForm QLineEdit#recipeBasicPathEdit {"
        "  background-color: transparent;"
        "  color: #e8eef6;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 34px; max-height: 34px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#lightenRecipeRoot QLabel#recipeBasicResMul {"
        "  color: #9eb8d4; font-size: 12px; background: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeBasicIllus {"
        "  color: #9eb8d4; font-size: 13px; background: transparent;"
        "  border: 1px dashed #6d8199; border-radius: 4px;"
        "  min-height: 168px;"
        "}"

        "#lightenRecipeRoot QFrame#recipeStepShell {"
        "  background-color: #1f2d40;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "}"

        "#lightenRecipeRoot QWidget#recipeMidPreviewStep {"
        "  background-color: transparent;"
        "}"

        "#lightenRecipeRoot QLineEdit, #lightenRecipeRoot QComboBox, #lightenRecipeRoot QSpinBox {"
        "  background-color: #1a222c; color: #e8eef6;"
        "  border: 1px solid #3a4a5d; border-radius: 4px; padding: 4px;"
        "}"

        "#lightenRecipeRoot QTableWidget {"
        "  background-color: #1f2d40; color: #ffffff;"
        "  gridline-color: #3a4a5d; border: 1px solid #3a4a5d; border-radius: 4px;"
        "}"
        "#lightenRecipeRoot QHeaderView::section {"
        "  background-color: #2f3b4c; color: #ffffff; padding: 6px;"
        "  border: none; border-right: 1px solid #3a4a5d;"
        "  font-weight: bold; font-size: 11px;"
        "}"

        "#lightenRecipeRoot QWidget#recipeCamSettingsForm {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeCamTabRow,"
        "#lightenRecipeRoot QWidget#recipeCamTabStrip {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeCamRotWrap {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeAlgoThresholdWrap {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QWidget#recipeSpecialDetectWrap,"
        "#lightenRecipeRoot QWidget#recipeOtherWrap {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QFrame#recipeCamTabRule {"
        "  background-color: #3a4a5d;"
        "  border: none; max-height: 1px; min-height: 1px;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable {"
        "  background-color: transparent; alternate-background-color: transparent;"
        "  color: #ffffff; gridline-color: #3a4a5d;"
        "  border-top: 1px solid #3a4a5d; border-left: 1px solid #3a4a5d;"
        "  border-right: none; border-bottom: none;"
        "  border-radius: 2px;"
        "  outline: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable::item {"
        "  background-color: transparent; color: #ffffff;"
        "  padding: 2px 4px; border: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable QHeaderView {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable QHeaderView::section {"
        "  background-color: transparent; color: #ffffff; font-weight: bold;"
        "  border: none; border-right: 1px solid #3a4a5d; border-bottom: 1px solid #3a4a5d;"
        "  padding: 4px 6px;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeAlgoRgbTable {"
        "  background-color: transparent; alternate-background-color: transparent;"
        "  color: #ffffff; gridline-color: #3a4a5d;"
        "  border-top: 1px solid #3a4a5d; border-left: 1px solid #3a4a5d;"
        "  border-right: none; border-bottom: none;"
        "  border-radius: 2px;"
        "  outline: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeAlgoRgbTable::item {"
        "  background-color: transparent; color: #ffffff;"
        "  padding: 2px 4px; border: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeAlgoRgbTable QHeaderView {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeAlgoRgbTable QHeaderView::section {"
        "  background-color: transparent; color: #ffffff; font-weight: bold;"
        "  border: none; border-right: 1px solid #3a4a5d; border-bottom: 1px solid #3a4a5d;"
        "  padding: 4px 6px;"
        "}"
        "#lightenRecipeRoot QSpinBox#recipeCamSpin,"
        "#lightenRecipeRoot QDoubleSpinBox#recipeCamSpin {"
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
        "#lightenRecipeRoot QSpinBox#recipeCamSpin::up-button,"
        "#lightenRecipeRoot QDoubleSpinBox#recipeCamSpin::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 16px; height: 12px;"
        "  border: none; background-color: transparent;"
        "}"
        "#lightenRecipeRoot QSpinBox#recipeCamSpin::down-button,"
        "#lightenRecipeRoot QDoubleSpinBox#recipeCamSpin::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 16px; height: 12px;"
        "  border: none; background-color: transparent;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable QComboBox#recipeCamCombo {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 22px; max-height: 22px;"
        "  combobox-popup: 2;"
        "  font-size: 11px;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable QComboBox#recipeCamCombo:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeCamParamTable QComboBox#recipeCamCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable QComboBox#recipeCamCombo {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 32px; max-height: 32px;"
        "  combobox-popup: 2;"
        "  font-size: 11px;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable QComboBox#recipeCamCombo:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable QComboBox#recipeCamCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"

        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable,"
        "#lightenRecipeRoot QTableWidget#recipeOtherTable {"
        "  background-color: transparent; alternate-background-color: transparent;"
        "  color: #ffffff; gridline-color: #3a4a5d;"
        "  border-top: 1px solid #3a4a5d; border-left: 1px solid #3a4a5d;"
        "  border-right: none; border-bottom: none;"
        "  border-radius: 4px;"
        "  outline: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable::item,"
        "#lightenRecipeRoot QTableWidget#recipeOtherTable::item {"
        "  background-color: transparent; color: #ffffff;"
        "  padding: 2px 4px; border: none;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable QHeaderView,"
        "#lightenRecipeRoot QTableWidget#recipeOtherTable QHeaderView {"
        "  background-color: transparent;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeSpecialDetectTable QHeaderView::section,"
        "#lightenRecipeRoot QTableWidget#recipeOtherTable QHeaderView::section {"
        "  background-color: transparent; color: #ffffff; font-weight: bold;"
        "  border: none; border-right: 1px solid #3a4a5d; border-bottom: 1px solid #3a4a5d;"
        "  padding: 6px 6px;"
        "  font-size: 11px;"
        "}"
        "#lightenRecipeRoot QTableWidget#recipeOtherTable QDoubleSpinBox#recipeCamSpin {"
        "  min-height: 20px; max-height: 20px;"
        "}"

        "#lightenRecipeRoot QFrame#recipeResultChip {"
        "  background-color: transparent;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "  min-height: 30px;"
        "}"
        "#lightenRecipeRoot QLabel#recipeResultChipTitle {"
        "  color: #ffffff; font-weight: bold; font-size: 12px; background: transparent;"
        "}"
        "#lightenRecipeRoot QLabel#recipeResultChipVal {"
        "  color: #e3f2fd; font-size: 11px; background: transparent;"
        "}"
        "#lightenRecipeRoot QLabel#recipeResultChipTotalNum {"
        "  color: #90caf9; font-size: 11px; font-weight: bold; background: transparent;"
        "}"
    ).arg(kRecipePanelBg, kRecipePanelBr));
}

void RecipeConfigView::buildLayout()
{
    const std::vector<std::string> cameraKeys = DeviceConfigReader::loadCameraKeys();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);
    root->addWidget(buildTopBar());

    auto *body = new QWidget(this);
    auto *bl   = new QHBoxLayout(body);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);

    bl->addWidget(buildRecipeList(), 0);
    bl->addWidget(buildCenterPanel(cameraKeys), 1);
    bl->addWidget(buildParamPanel(cameraKeys), 0);

    root->addWidget(body, 1);
}

QWidget *RecipeConfigView::buildTopBar()
{
    auto *bar = new QWidget(this);
    auto *outer = new QVBoxLayout(bar);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *shell = new QFrame(bar);
    shell->setObjectName(QStringLiteral("recipeTopActionShell"));
    shell->setAttribute(Qt::WA_StyledBackground, true);

    auto *vl = new QVBoxLayout(shell);
    vl->setContentsMargins(10, 10, 10, 10);
    vl->setSpacing(8);

    auto *actionTitle = new QLabel(QStringLiteral("配方操作"), shell);
    actionTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    vl->addWidget(actionTitle);

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(8);
    auto assignActionBtn = [&](const char *label, QPushButton **store) {
        auto *btn = new QPushButton(QString::fromUtf8(label), shell);
        btn->setObjectName(std::strcmp(label, "删除") == 0 ? "dangerBtn" : "");
        if (store)
            *store = btn;
        row1->addWidget(btn);
    };
    assignActionBtn("新增", &m_addRecipeBtn);
    assignActionBtn("复制", &m_copyRecipeBtn);
    assignActionBtn("删除", &m_removeRecipeBtn);
    assignActionBtn("设为默认", &m_setDefaultRecipeBtn);
    assignActionBtn("导入", &m_importRecipeBtn);
    assignActionBtn("导出", &m_exportRecipeBtn);
    assignActionBtn("保存配方", &m_saveRecipeBtn);
    row1->addStretch();
    vl->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(10);
    auto makeStatusPill = [&](const QString &text, bool active, QLabel **store) {
        auto *lbl = new QLabel(text, shell);
        lbl->setObjectName(active ? QStringLiteral("pillActive") : QStringLiteral("pill"));
        if (store)
            *store = lbl;
        row2->addWidget(lbl);
    };
    makeStatusPill(QStringLiteral("当前编辑配方: PANEL-A01"), true, &m_statusRecipeLbl);
    makeStatusPill(QStringLiteral("图像质量: 通过(92%)"), true, &m_statusQualityLbl);
    if (!kRecipeShowImageQualityUi && m_statusQualityLbl)
        m_statusQualityLbl->hide();
    makeStatusPill(QStringLiteral("参数状态: 可保存"), false, &m_statusParamLbl);
    row2->addStretch();
    vl->addLayout(row2);

    outer->addWidget(shell);
    return bar;
}

QWidget *RecipeConfigView::buildRecipeList()
{
    auto *box = new QFrame(this);
    box->setObjectName(QStringLiteral("recipeListGroup"));
    box->setAttribute(Qt::WA_StyledBackground, true);
    box->setFixedWidth(340);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *listTitle = new QLabel(QStringLiteral("配方列表区"), box);
    listTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    layout->addWidget(listTitle);

    auto *search = new QLineEdit(box);
    search->setObjectName(QStringLiteral("recipeListSearch"));
    search->setPlaceholderText(QStringLiteral("搜索配方名称"));
    m_recipeSearchEdit = search;
    layout->addWidget(search);

    m_recipeList = new QListWidget(box);
    m_recipeList->setObjectName(QStringLiteral("recipeListWidget"));
    m_recipeList->setFrameShape(QFrame::NoFrame);
    m_recipeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recipeList->setSpacing(0);
    m_recipeList->setUniformItemSizes(true);
    layout->addWidget(m_recipeList, 1);

    m_logEdit = new QTextEdit(box);
    m_logEdit->setReadOnly(true);
    m_logEdit->setObjectName(QStringLiteral("logBox"));
    m_logEdit->setFixedHeight(168);
    m_logEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

QWidget *RecipeConfigView::buildCenterPanel(const std::vector<std::string> &cameraKeys)
{
    auto *shell = new QFrame(this);
    shell->setObjectName(QStringLiteral("recipeCenterShell"));
    auto *layout = new QVBoxLayout(shell);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto *centerTitle = new QLabel(QStringLiteral("调试区域"), shell);
    centerTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    layout->addWidget(centerTitle);

    // —— 预览区头部工具条：单行、压低行高 ——
    auto *toolbar = new QFrame(shell);
    toolbar->setObjectName(QStringLiteral("recipePreviewToolbar"));
    toolbar->setAttribute(Qt::WA_StyledBackground, true);
    auto *row = new QHBoxLayout(toolbar);
    row->setContentsMargins(8, 2, 8, 2);
    row->setSpacing(6);

    auto pinToolbarCtrlHeight = [](QWidget *w) {
        w->setFixedHeight(kRecipePreviewToolbarCtrlH);
        QSizePolicy sp = w->sizePolicy();
        sp.setVerticalPolicy(QSizePolicy::Fixed);
        w->setSizePolicy(sp);
    };

    m_toolbarCamCombo = new PreviewToolBarComboBox(toolbar);
    m_toolbarCamCombo->addItems(cameraDisplayNames(cameraKeys));
    m_toolbarCamCombo->setMinimumWidth(112);
    row->addWidget(m_toolbarCamCombo, 0, Qt::AlignVCenter);

    auto *snapBtn = new ui::ThemedIconButton(toolbar);
    snapBtn->setObjectName(QStringLiteral("recipeToolIconBtn"));
    ui::applyFaIcon(snapBtn, ui::FaIcon::Camera, kRecipePreviewToolbarIconPx);
    snapBtn->setToolTip(QStringLiteral("抓拍 "));
    m_snapBtn = snapBtn;
    row->addWidget(snapBtn, 0, Qt::AlignVCenter);
    connect(snapBtn, &ui::ThemedIconButton::clicked, this, [this]() {
        if (!m_toolbarCamCombo)
            return;

        const QString cameraName = m_toolbarCamCombo->currentText();

        const auto dm = tryGetDeviceManager();
        if (!dm) {
            appendLog(QStringLiteral("设备管理器未就绪，无法抓拍"), QStringLiteral("ERROR"));
            return;
        }

        const auto camera = dm->CameraHandler(cameraName.toStdString());
        if (!camera) {
            appendLog(QStringLiteral("未找到相机 \"%1\"，无法抓拍").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        if (!camera->IsConnected()) {
            appendLog(QStringLiteral("相机 \"%1\" 未连接，无法抓拍").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        const auto state = camera->GetCaptureState();
        if (state != common::device::CaptureState::Idle) {
            QString stateText;
            switch (state) {
            case common::device::CaptureState::Preview:
                stateText = QStringLiteral("预览中");
                break;
            case common::device::CaptureState::Capturing:
                stateText = QStringLiteral("采图中");
                break;
            default:
                stateText = QStringLiteral("非空闲");
                break;
            }
            appendLog(QStringLiteral("相机 \"%1\" 当前状态为 %2，仅空闲时可抓拍").arg(cameraName, stateText),
                      QStringLiteral("WARN"));
            return;
        }

        if (!cam_async::GrabOneImage(cameraName, kRecipePreviewWinId))
            appendLog(QStringLiteral("相机 \"%1\" 抓拍失败").arg(cameraName), QStringLiteral("ERROR"));
    });

    auto *liveBtn = new ui::ThemedIconButton(toolbar);
    liveBtn->setObjectName(QStringLiteral("recipeToolIconBtn"));
    m_liveBtn = liveBtn;
    syncPreviewButtonFromCaptureState();
    row->addWidget(liveBtn, 0, Qt::AlignVCenter);
    connect(liveBtn, &ui::ThemedIconButton::clicked, this, [this]() {
        if (!m_toolbarCamCombo)
            return;

        const QString cameraName = m_toolbarCamCombo->currentText();

        const auto dm = tryGetDeviceManager();
        if (!dm) {
            appendLog(QStringLiteral("设备管理器未就绪，无法切换预览"), QStringLiteral("ERROR"));
            return;
        }

        const auto camera = dm->CameraHandler(cameraName.toStdString());
        if (!camera) {
            appendLog(QStringLiteral("未找到相机 \"%1\"，无法切换预览").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        if (!camera->IsConnected()) {
            appendLog(QStringLiteral("相机 \"%1\" 未连接，无法切换预览").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        const auto state = camera->GetCaptureState();

        if (state == common::device::CaptureState::Idle) {
            if (!cam_async::StartAsyncGrab(cameraName, kRecipePreviewWinId)) {
                appendLog(QStringLiteral("相机 \"%1\" 启动预览失败").arg(cameraName), QStringLiteral("ERROR"));
                syncPreviewButtonFromCaptureState();
            }
        } else if (state == common::device::CaptureState::Preview) {
            QString name = cameraName;
            if (!cam_async::StopAsyncGrab(name)) {
                appendLog(QStringLiteral("相机 \"%1\" 停止预览失败").arg(cameraName), QStringLiteral("ERROR"));
                syncPreviewButtonFromCaptureState();
            }
        }
    });

    auto *saveImageBtn = new ui::ThemedIconButton(toolbar);
    saveImageBtn->setObjectName(QStringLiteral("recipeToolIconBtn"));
    ui::applyFaIcon(saveImageBtn, ui::FaIcon::Save, kRecipePreviewToolbarIconPx);
    saveImageBtn->setToolTip(QStringLiteral("保存图像"));
    row->addWidget(saveImageBtn, 0, Qt::AlignVCenter);
    connect(saveImageBtn, &ui::ThemedIconButton::clicked, this, [this]() {
        if (!m_toolbarCamCombo)
            return;

        const QString cameraName = m_toolbarCamCombo->currentText();

        const auto dm = tryGetDeviceManager();
        if (!dm) {
            appendLog(QStringLiteral("设备管理器未就绪，无法保存图像"), QStringLiteral("ERROR"));
            return;
        }

        const auto camera = dm->CameraHandler(cameraName.toStdString());
        if (!camera) {
            appendLog(QStringLiteral("未找到相机 \"%1\"，无法保存图像").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        if (!camera->IsConnected()) {
            appendLog(QStringLiteral("相机 \"%1\" 未连接，无法保存图像").arg(cameraName), QStringLiteral("ERROR"));
            return;
        }

        const auto state = camera->GetCaptureState();
        if (state != common::device::CaptureState::Idle) {
            QString stateText;
            switch (state) {
            case common::device::CaptureState::Preview:
                stateText = QStringLiteral("预览中");
                break;
            case common::device::CaptureState::Capturing:
                stateText = QStringLiteral("采图中");
                break;
            default:
                stateText = QStringLiteral("非空闲");
                break;
            }
            appendLog(QStringLiteral("相机 \"%1\" 当前状态为 %2，仅空闲时可保存图像").arg(cameraName, stateText),
                      QStringLiteral("WARN"));
            return;
        }

        const QString defaultName = QStringLiteral("image_%1.raw")
                                        .arg(QDateTime::currentDateTime().toString(
                                            QStringLiteral("yyyyMMddHHmmss")));
        QString path = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("保存图像"),
            defaultName,
            QStringLiteral("RAW 图像 (*.raw);;JPEG 图像 (*.jpg *.jpeg);;BMP 图像 (*.bmp)"));
        if (path.isEmpty())
            return;

        const QString lower = path.toLower();
        if (!lower.endsWith(QStringLiteral(".raw")) && !lower.endsWith(QStringLiteral(".jpg"))
            && !lower.endsWith(QStringLiteral(".jpeg")) && !lower.endsWith(QStringLiteral(".bmp"))) {
            path += QStringLiteral(".raw");
        }

        if (!cam_async::GrabOneImageToFile(cameraName, path))
            appendLog(QStringLiteral("图像保存失败"), QStringLiteral("ERROR"));
    });

    auto *crossBtn = new ui::ThemedIconButton(toolbar);
    crossBtn->setObjectName(QStringLiteral("recipeToolIconBtn"));
    ui::applyFaIcon(crossBtn, ui::FaIcon::Crosshairs, kRecipePreviewToolbarIconPx);
    crossBtn->setToolTip(QStringLiteral("绘制十字线"));
    row->addWidget(crossBtn, 0, Qt::AlignVCenter);
    connect(crossBtn, &ui::ThemedIconButton::clicked, this, [this]() {
        if (!m_graphicsView)
            return;
        m_graphicsView->DrawCross(kRecipePreviewWinId);
    });

    row->addSpacing(4);
    auto *colorGroup = new QButtonGroup(toolbar);
    colorGroup->setExclusive(true);
    m_colorBtnGroup = colorGroup;
    const std::pair<const char *, bool> kColorItems[] = {
        {"红", true}, {"绿", false}, {"蓝", false}, {"黑", false}, {"白", false}};
    for (int colorId = static_cast<int>(ColorChannel::Red);
         colorId <= static_cast<int>(ColorChannel::White); ++colorId) {
        const auto &[t, on] = kColorItems[colorId];
        auto *b = new QPushButton(QString::fromUtf8(t), toolbar);
        b->setObjectName(QStringLiteral("recipeColorBtn"));
        b->setCheckable(true);
        b->setChecked(on);
        colorGroup->addButton(b, colorId);
        row->addWidget(b, 0, Qt::AlignVCenter);
    }

    row->addSpacing(6);
    auto *roiSwitchRow = new QWidget(toolbar);
    roiSwitchRow->setObjectName(QStringLiteral("recipeRoiSwitchWrap"));
    roiSwitchRow->setAttribute(Qt::WA_StyledBackground, true);
    roiSwitchRow->setAutoFillBackground(false);
    auto *roiHl       = new QHBoxLayout(roiSwitchRow);
    roiHl->setContentsMargins(0, 0, 0, 0);
    roiHl->setSpacing(6);
    roiHl->setAlignment(Qt::AlignVCenter);
    auto *roiLbl = new QLabel(QStringLiteral("区域定位"), roiSwitchRow);
    roiLbl->setObjectName(QStringLiteral("recipeRoiSwitchLabel"));
    roiHl->addWidget(roiLbl, 0, Qt::AlignVCenter);
    auto *pill = new RecipePillSwitch(true, roiSwitchRow);
    m_roiSwitch = pill;
    roiHl->addWidget(pill, 0, Qt::AlignVCenter);
    row->addWidget(roiSwitchRow, 0, Qt::AlignVCenter);

    auto *roiMode = new PreviewToolBarComboBox(toolbar);
    roiMode->addItems({QStringLiteral("不绘制ROI"), QStringLiteral("手动ROI(矩形)"),
                       QStringLiteral("手动ROI(四点)")});
    roiMode->setMinimumWidth(118);
    m_roiModeCombo = roiMode;
    row->addWidget(roiMode, 0, Qt::AlignVCenter);
    QObject::connect(roiMode, QOverload<int>::of(&QComboBox::activated), this,
                     [this](int index) { applyToolbarRoiMode(index); });

    auto *holdRoiBtn = new QPushButton(QStringLiteral("区域定位保存"), toolbar);
    holdRoiBtn->setObjectName(QStringLiteral("recipeToolbarActionBtn"));
    m_holdRoiBtn = holdRoiBtn;
    row->addWidget(holdRoiBtn, 0, Qt::AlignVCenter);

    auto *viewRoiBtn = new QPushButton(QStringLiteral("查看ROI"), toolbar);
    viewRoiBtn->setObjectName(QStringLiteral("recipeToolbarActionBtn"));
    m_viewRoiBtn = viewRoiBtn;
    row->addWidget(viewRoiBtn, 0, Qt::AlignVCenter);

    const QString popupExtra12 = QStringLiteral(
        " QAbstractItemView, QAbstractItemView::item { font-size: 12px; }"
        " QAbstractItemView::item { min-height: 30px; }");
    if (QStyle *st = QStyleFactory::create(QStringLiteral("Fusion")))
        m_toolbarCamCombo->setStyle(st);
    {
        const QString popupViewStyle12 =
            previewToolbarComboPopupViewStyleSheet() + popupExtra12;
        if (QAbstractItemView *v = m_toolbarCamCombo->view()) {
            v->setStyleSheet(popupViewStyle12);
            if (auto *lv = qobject_cast<QListView *>(v))
                lv->setItemDelegate(new PlainComboPopupDelegate(lv));
        }
    }
    if (QStyle *st = QStyleFactory::create(QStringLiteral("Fusion")))
        roiMode->setStyle(st);
    styleRecipeComboPopup(roiMode, 12, popupExtra12);
    QFont tf = toolbar->font();
    tf.setPixelSize(12);
    tf.setBold(false);
    m_toolbarCamCombo->setFont(tf);
    roiMode->setFont(tf);
    for (QAbstractButton *b : colorGroup->buttons())
        b->setFont(tf);
    roiLbl->setFont(tf);
    QFont tfb = tf;
    tfb.setBold(true);
    holdRoiBtn->setFont(tfb);
    viewRoiBtn->setFont(tfb);

    if (QAbstractItemView *v = m_toolbarCamCombo->view())
        v->setFont(tf);

    roiLbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    pinToolbarCtrlHeight(roiLbl);
    pinToolbarCtrlHeight(roiSwitchRow);
    pinToolbarCtrlHeight(m_toolbarCamCombo);
    pinToolbarCtrlHeight(roiMode);
    pinToolbarCtrlHeight(snapBtn);
    pinToolbarCtrlHeight(liveBtn);
    for (QAbstractButton *b : colorGroup->buttons())
        pinToolbarCtrlHeight(b);
    pinToolbarCtrlHeight(holdRoiBtn);
    pinToolbarCtrlHeight(viewRoiBtn);

    row->addStretch(1);
    layout->addWidget(toolbar);

    // —— 包裹「预览列 + 单步调试」的横向容器：透明底，透出 #recipeCenterShell ——
    auto *mid = new QWidget(shell);
    mid->setObjectName(QStringLiteral("recipeMidPreviewStep"));
    mid->setAttribute(Qt::WA_StyledBackground, true);
    auto *ml  = new QHBoxLayout(mid);
    ml->setContentsMargins(0, 0, 0, 0);
    ml->setSpacing(8);

    // Preview pane (placeholder + optional graphics view in stack)
    m_previewStack = new QStackedWidget(mid);
    auto *placeholderPane = new QWidget(m_previewStack);
    placeholderPane->setAttribute(Qt::WA_StyledBackground, true);
    placeholderPane->setStyleSheet(QStringLiteral("background-color: #000000;"));
    m_previewStack->addWidget(placeholderPane);

    m_graphicsView = PluginManager::getInterface<IGraphicsVew>();
    if (m_graphicsView)
    {
        QWidget *graphicsPane = m_graphicsView->CreateWidget(kRecipePreviewWinId);
        m_graphicsView->SetVisible(kRecipePreviewWinId, true, false);
        m_previewStack->addWidget(graphicsPane);
    }

    m_previewStack->setCurrentWidget(placeholderPane);
    m_previewStack->setMinimumHeight(200);
    ml->addWidget(m_previewStack, 1);

    auto *stepShell = new QFrame(mid);
    stepShell->setObjectName(QStringLiteral("recipeStepShell"));
    stepShell->setAttribute(Qt::WA_StyledBackground, true);
    stepShell->setFixedWidth(140);
    auto *svl = new QVBoxLayout(stepShell);
    svl->setContentsMargins(8, 8, 8, 8);
    svl->setSpacing(6);
    auto *stepTitle = new QLabel(QStringLiteral("单步调试"), stepShell);
    stepTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    svl->addWidget(stepTitle);
    auto addStepBtn = [svl, stepShell](const char *label) {
        auto *b = new QPushButton(QString::fromUtf8(label), stepShell);
        b->setObjectName(QStringLiteral("recipeStepBtn"));
        svl->addWidget(b);
        return b;
    };
    connect(addStepBtn("进料"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepFeedInClicked);
    connect(addStepBtn("出料"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepFeedOutClicked);
    connect(addStepBtn("读取图像"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepReadImageClicked);
    connect(addStepBtn("纯色检测"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepPureColorDetectClicked);
    connect(addStepBtn("串灯检测"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepStringLightDetectClicked);
    connect(addStepBtn("混色检测"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepMixColorDetectClicked);
    connect(addStepBtn("黑屏亮灯"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepBlackScreenLightDetectClicked);
    connect(addStepBtn("Flash检测"), &QPushButton::clicked, this, &RecipeConfigView::slot_stepFlashDetectClicked);
    svl->addStretch(1);
    ml->addWidget(stepShell, 0);

    layout->addWidget(mid, 1);

    // —— 结果输出区：4 列 × 2 行网格，高度随内容（无固定 min-height、无底部 stretch）——
    auto *resBox = new QFrame(shell);
    resBox->setObjectName(QStringLiteral("recipeResultGroup"));
    resBox->setAttribute(Qt::WA_StyledBackground, true);
    auto *rv = new QVBoxLayout(resBox);
    rv->setContentsMargins(12, 8, 12, 10);
    rv->setSpacing(6);
    auto *resTitle = new QLabel(QStringLiteral("结果输出区域"), resBox);
    resTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    rv->addWidget(resTitle);

    auto *chipWrap = new QWidget(resBox);
    chipWrap->setObjectName(QStringLiteral("recipeResultChipArea"));
    chipWrap->setAttribute(Qt::WA_StyledBackground, true);
    auto *grid     = new QGridLayout(chipWrap);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(5);
    for (int c = 0; c < 4; ++c)
        grid->setColumnStretch(c, 1);

    auto fmtRgb = [](int v, const QString &ch) {
        return QStringLiteral("%1:%2").arg(ch).arg(v < 0 ? QStringLiteral("-") : QString::number(v));
    };
    auto makeChip = [&](DetectItemKind kind, int index) {
        auto *cell = new RecipeResultChipFrame(kind, chipWrap);
        cell->setObjectName(QStringLiteral("recipeResultChip"));
        cell->setAttribute(Qt::WA_StyledBackground, true);
        auto *cl = new QHBoxLayout(cell);
        cl->setContentsMargins(6, 1, 6, 1);
        cl->setSpacing(5);
        auto *t = new QLabel(detectItemName(kind), cell);
        t->setObjectName(QStringLiteral("recipeResultChipTitle"));
        cl->addWidget(t);
        auto *v1 = new QLabel(fmtRgb(0, QStringLiteral("R")), cell);
        v1->setObjectName(QStringLiteral("recipeResultChipVal"));
        auto *v2 = new QLabel(fmtRgb(0, QStringLiteral("G")), cell);
        v2->setObjectName(QStringLiteral("recipeResultChipVal"));
        auto *v3 = new QLabel(fmtRgb(0, QStringLiteral("B")), cell);
        v3->setObjectName(QStringLiteral("recipeResultChipVal"));
        cl->addWidget(v1);
        cl->addWidget(v2);
        cl->addWidget(v3);
        cl->addStretch(1);
        auto *totLbl = new QLabel(QStringLiteral("总数:"), cell);
        totLbl->setObjectName(QStringLiteral("recipeResultChipVal"));
        auto *totNum = new QLabel(QStringLiteral("0"), cell);
        totNum->setObjectName(QStringLiteral("recipeResultChipTotalNum"));
        cl->addWidget(totLbl, 0, Qt::AlignVCenter);
        cl->addWidget(totNum, 0, Qt::AlignVCenter);
        RecipeResultChipFrame::makeChildrenMouseTransparent(cell);
        connect(cell, &RecipeResultChipFrame::sig_clicked, this, &RecipeConfigView::slot_resultChipClicked);
        if (index >= 0 && index < static_cast<int>(m_resultChipLabels.size()))
            m_resultChipLabels[static_cast<size_t>(index)] = {v1, v2, v3, totNum};
        return cell;
    };

    // 第 0 行 4 列：死灯 | 暗灯 | 亮灯 | 串色；第 1 行：串死 | 串活 | 混色 | 黑屏亮灯（RGB/总数初始均为 0）
    static const DetectItemKind kResultChipKinds[] = {
        DetectItemKind::DeadLed,        DetectItemKind::DimLed,       DetectItemKind::BrightLed,
        DetectItemKind::ColorVariation, DetectItemKind::ColorMapDead, DetectItemKind::ColorMapLive,
        DetectItemKind::MixedColor,     DetectItemKind::BlackScreenOn,
    };
    int chipIdx = 0;
    for (DetectItemKind kind : kResultChipKinds) {
        grid->addWidget(makeChip(kind, chipIdx), chipIdx / 4, chipIdx % 4);
        ++chipIdx;
    }

    rv->addWidget(chipWrap);
    layout->addWidget(resBox, 0);

    return shell;
}

QWidget *RecipeConfigView::buildParamPanel(const std::vector<std::string> &cameraKeys)
{
    auto *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("recipeParamPanel"));
    panel->setAttribute(Qt::WA_StyledBackground, true);
    panel->setFixedWidth(500);
    auto *outer = new QVBoxLayout(panel);
    outer->setContentsMargins(10, 10, 10, 10);
    outer->setSpacing(8);

    auto *paramTitle = new QLabel(QStringLiteral("参数配置"), panel);
    paramTitle->setObjectName(QStringLiteral("recipeSectionTitle"));
    outer->addWidget(paramTitle);

    auto *scroll = new QScrollArea(panel);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"));

    auto *content = new QWidget;
    content->setObjectName(QStringLiteral("recipeParamSectionsWrap"));
    content->setAttribute(Qt::WA_StyledBackground, true);
    content->setAutoFillBackground(false);
    scroll->viewport()->setAutoFillBackground(false);
    auto *layout  = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(makeCollapsible(QStringLiteral("点亮产品基本信息"),
                                      buildParamBasicInfo(content), content, true));
    layout->addWidget(makeCollapsible(QStringLiteral("点亮产品相机设置"),
                                      buildParamCameraSettings(content, cameraKeys), content,
                                      false));
    layout->addWidget(makeCollapsible(QStringLiteral("算法阈值"),
                                      buildParamAlgoThresholds(content), content, false));
    layout->addWidget(makeCollapsible(QStringLiteral("特殊检测项"),
                                      buildParamSpecialDetection(content), content, false));
    layout->addWidget(makeCollapsible(QStringLiteral("其他"),
                                      buildParamOtherSettings(content), content, false));
    layout->addStretch();
    scroll->setWidget(content);
    outer->addWidget(scroll, 1);
    return panel;
}

QWidget *RecipeConfigView::makeCollapsible(const QString &title, QWidget *content,
                                           QWidget *parent, bool expanded)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("recipeParamSection"));
    frame->setAttribute(Qt::WA_StyledBackground, true);
    auto *wl = new QVBoxLayout(frame);
    wl->setContentsMargins(8, 8, 8, 8);
    wl->setSpacing(6);

    auto *toggleBtn = new QPushButton((expanded ? QStringLiteral("▾  ") : QStringLiteral("▸  "))
                                          + title,
                                      frame);
    toggleBtn->setCheckable(true);
    toggleBtn->setChecked(expanded);
    toggleBtn->setFlat(true);
    toggleBtn->setObjectName(QStringLiteral("collapsibleHeader"));
    toggleBtn->setStyleSheet(QStringLiteral("text-align:left; padding:6px 8px;"));
    wl->addWidget(toggleBtn);
    content->setVisible(expanded);
    wl->addWidget(content, 0, Qt::AlignTop);

    connect(toggleBtn, &QPushButton::toggled, content, &QWidget::setVisible);
    connect(toggleBtn, &QPushButton::toggled, toggleBtn,
            [toggleBtn, title](bool on) {
                toggleBtn->setText((on ? QStringLiteral("▾  ") : QStringLiteral("▸  ")) + title);
            });

    return frame;
}

void RecipeConfigView::connectCameraTabComboSync()
{
    if (!m_toolbarCamCombo || !m_camSettingsTabGroup)
        return;

    connect(m_toolbarCamCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [this](int ix) {
                if (!m_camSettingsTabGroup)
                    return;
                if (QAbstractButton *b = m_camSettingsTabGroup->button(ix)) {
                    if (!b->isChecked())
                        b->setChecked(true);
                }
                cam_async::SetWinId(kRecipePreviewWinId);
            });

    connect(m_camSettingsTabGroup,
            static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
            this,
            [this](QAbstractButton *btn) {
                if (!m_toolbarCamCombo || !m_camSettingsTabGroup)
                    return;
                const int id = m_camSettingsTabGroup->id(btn);
                if (id < 0 || id >= m_toolbarCamCombo->count())
                    return;
                if (m_toolbarCamCombo->currentIndex() != id)
                    m_toolbarCamCombo->setCurrentIndex(id);
            });
}

QWidget *RecipeConfigView::buildParamBasicInfo(QWidget *parent)
{
    auto *w = new QWidget(parent);
    w->setObjectName(QStringLiteral("recipeBasicForm"));
    w->setAttribute(Qt::WA_StyledBackground, true);

    auto *vl = new QVBoxLayout(w);
    vl->setContentsMargins(10, 2, 10, 8);
    vl->setSpacing(0);

    auto addLabel = [w, vl](const QString &txt) {
        auto *lb = new QLabel(txt, w);
        lb->setObjectName(QStringLiteral("recipeBasicFormLabel"));
        vl->addWidget(lb);
    };

    addLabel(QStringLiteral("配方名称"));
    vl->addSpacing(3);
    auto *nameEdit = new QLineEdit(QStringLiteral("PANEL-A01"), w);
    nameEdit->setObjectName(QStringLiteral("recipeBasicPathEdit"));
    m_basicNameEdit = nameEdit;
    vl->addWidget(nameEdit);
    vl->addSpacing(10);

    addLabel(QStringLiteral("分辨率"));
    vl->addSpacing(3);
    auto *rrl = new QHBoxLayout;
    rrl->setContentsMargins(0, 0, 0, 0);
    rrl->setSpacing(8);
    m_basicWidthSpin = makeRecipeIntSpin(w, 1, 9999);
    m_basicWidthSpin->setValue(1920);
    m_basicHeightSpin = makeRecipeIntSpin(w, 1, 9999);
    m_basicHeightSpin->setValue(1080);
    auto *mul = new QLabel(QStringLiteral("×"), w);
    mul->setObjectName(QStringLiteral("recipeBasicResMul"));
    rrl->addWidget(m_basicWidthSpin, 1);
    rrl->addWidget(mul, 0, Qt::AlignVCenter);
    rrl->addWidget(m_basicHeightSpin, 1);
    vl->addLayout(rrl);
    vl->addSpacing(10);

    addLabel(QStringLiteral("坐标系"));
    vl->addSpacing(3);
    const QString coordPopupExtra = QStringLiteral(
        " QAbstractItemView, QAbstractItemView::item { font-size: 12px; }"
        " QAbstractItemView::item { min-height: 32px; }");
    auto *coordSel = makeRecipeTableCombo(
        kRecipeBasicCoordComboH, w,
        {QStringLiteral("① 左上原点：X→右，Y→下"), QStringLiteral("② 右上原点：X→下，Y→左"),
         QStringLiteral("③ 左下原点：X→左，Y→上"), QStringLiteral("④ 右下原点：X→上，Y→右"),
         QStringLiteral("⑤ 左上原点：X→下，Y→右"), QStringLiteral("⑥ 右上原点：X→左，Y→下"),
         QStringLiteral("⑦ 左下原点：X→上，Y→左"), QStringLiteral("⑧ 右下原点：X→右，Y→上")},
        12, coordPopupExtra);
    coordSel->setCurrentIndex(0);
    m_basicCoordCombo = coordSel;
    vl->addWidget(coordSel);
    vl->addSpacing(10);

    auto *illus = new RecipeCoordIllustration(w);
    illus->setCoordIndex(coordSel->currentIndex());
    QObject::connect(coordSel, QOverload<int>::of(&QComboBox::currentIndexChanged), illus,
                     [illus](int i) { illus->setCoordIndex(i); });
    vl->addWidget(illus);

    return w;
}

QTableWidget *RecipeConfigView::buildCameraSettingsTable(QWidget *parent)
{
    constexpr int kRows = 8;
    auto *t = new QTableWidget(kRows, 4, parent);
    t->setObjectName(QStringLiteral("recipeCamParamTable"));
    t->setAttribute(Qt::WA_StyledBackground, true);
    initRecipeGridTable(t);
    t->setHorizontalHeaderLabels({QStringLiteral("参数"), QStringLiteral("R"),
                                  QStringLiteral("G"), QStringLiteral("B")});
    for (int c = 0; c < 4; ++c)
        t->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
    t->verticalHeader()->setDefaultSectionSize(40);

    const QStringList labels = {
        QStringLiteral("纯色曝光(ms)"), QStringLiteral("纯色灰阶"),
        QStringLiteral("串灯曝光(ms)"), QStringLiteral("串灯灰阶"),
        QStringLiteral("白色曝光(ms)"), QStringLiteral("黑色曝光(ms)"),
        QStringLiteral("图像质量分析项"),
        QStringLiteral("旋转角度"),
    };

    auto setLabelCell = [t](int row, const QString &txt) {
        t->setItem(row, 0, recipeTableLabelItem(txt));
    };

    auto addRgbRow = [t, &setLabelCell, this](int row, const QString &label, QSpinBox **r, QSpinBox **g,
                                             QSpinBox **b) {
        setLabelCell(row, label);
        QSpinBox *cols[3] = {nullptr, nullptr, nullptr};
        for (int c = 0; c < 3; ++c) {
            cols[c] = makeRecipeIntSpin(t, 0, 9999);
            t->setCellWidget(row, c + 1, cols[c]);
        }
        *r = cols[0];
        *g = cols[1];
        *b = cols[2];
    };

    addRgbRow(0, labels[0], &m_camSettingsWidgets.pureExposureR, &m_camSettingsWidgets.pureExposureG,
              &m_camSettingsWidgets.pureExposureB);
    addRgbRow(1, labels[1], &m_camSettingsWidgets.pureGrayR, &m_camSettingsWidgets.pureGrayG,
              &m_camSettingsWidgets.pureGrayB);
    addRgbRow(2, labels[2], &m_camSettingsWidgets.seriesExposureR, &m_camSettingsWidgets.seriesExposureG,
              &m_camSettingsWidgets.seriesExposureB);
    addRgbRow(3, labels[3], &m_camSettingsWidgets.seriesGrayR, &m_camSettingsWidgets.seriesGrayG,
              &m_camSettingsWidgets.seriesGrayB);

    setLabelCell(4, labels[4]);
    t->setSpan(4, 1, 1, 3);
    m_camSettingsWidgets.whiteExposure = makeRecipeIntSpin(t, 0, 9999);
    t->setCellWidget(4, 1, m_camSettingsWidgets.whiteExposure);

    setLabelCell(5, labels[5]);
    t->setSpan(5, 1, 1, 3);
    m_camSettingsWidgets.blackExposure = makeRecipeIntSpin(t, 0, 9999);
    t->setCellWidget(5, 1, m_camSettingsWidgets.blackExposure);

    setLabelCell(6, labels[6]);
    m_camSettingsWidgets.qualityR = new RecipeQualityLabel(t);
    m_camSettingsWidgets.qualityG = new RecipeQualityLabel(t);
    m_camSettingsWidgets.qualityB = new RecipeQualityLabel(t);
    t->setCellWidget(6, 1, m_camSettingsWidgets.qualityR);
    t->setCellWidget(6, 2, m_camSettingsWidgets.qualityG);
    t->setCellWidget(6, 3, m_camSettingsWidgets.qualityB);
    if (!kRecipeShowImageQualityUi)
        t->setRowHidden(6, true);

    setLabelCell(7, labels[7]);
    t->setSpan(7, 1, 1, 3);
    auto *rotWrap = new QWidget(t);
    rotWrap->setObjectName(QStringLiteral("recipeCamRotWrap"));
    rotWrap->setAttribute(Qt::WA_StyledBackground, true);
    auto *rotVl = new QVBoxLayout(rotWrap);
    rotVl->setContentsMargins(6, 6, 6, 6);
    rotVl->setSpacing(0);
    auto *rotSel = makeRecipeTableCombo(
        kRecipePreviewToolbarCtrlH, rotWrap,
        {QStringLiteral("0"), QStringLiteral("90"), QStringLiteral("180"), QStringLiteral("270")});
    rotSel->setMinimumWidth(168);
    rotVl->addStretch();
    rotVl->addWidget(rotSel, 0, Qt::AlignCenter);
    rotVl->addStretch();
    t->setCellWidget(7, 1, rotWrap);
    m_camSettingsWidgets.rotation = rotSel;

    const int visibleDataRows = kRecipeShowImageQualityUi ? kRows : kRows - 1;
    setRecipeTableFixedHeight(t, visibleDataRows);
    t->setMinimumWidth(400);

    return t;
}

QWidget *RecipeConfigView::buildParamCameraSettings(QWidget *parent,
                                                    const std::vector<std::string> &cameraKeys)
{
    auto *w      = new QWidget(parent);
    w->setObjectName(QStringLiteral("recipeCamSettingsForm"));
    w->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(8, 6, 8, 8);
    layout->setSpacing(8);

    auto *camGroup = new QButtonGroup(w);
    camGroup->setExclusive(true);
    m_camSettingsTabGroup = camGroup;

    auto *tabBlock = new QWidget(w);
    tabBlock->setObjectName(QStringLiteral("recipeCamTabStrip"));
    tabBlock->setAttribute(Qt::WA_StyledBackground, true);
    auto *tabBl = new QVBoxLayout(tabBlock);
    tabBl->setContentsMargins(0, 0, 0, 0);
    tabBl->setSpacing(0);

    auto *tabRow = new QWidget(tabBlock);
    tabRow->setObjectName(QStringLiteral("recipeCamTabRow"));
    tabRow->setAttribute(Qt::WA_StyledBackground, true);
    tabRow->setMinimumHeight(RecipeCamTabButton::kTabH + 8);
    auto *tabRl = new QHBoxLayout(tabRow);
    tabRl->setContentsMargins(0, 2, 0, 2);
    tabRl->setSpacing(4);
    int camTabId = 0;
    const QStringList camTabNames = cameraDisplayNames(cameraKeys);
    bool firstTab = true;
    for (const QString &name : camTabNames) {
        auto *b = new RecipeCamTabButton(name, tabRow);
        b->setChecked(firstTab);
        firstTab = false;
        camGroup->addButton(b, camTabId++);
        tabRl->addWidget(b);
    }
    tabRl->addStretch();
    tabBl->addWidget(tabRow);

    auto *camTabRule = new QFrame(tabBlock);
    camTabRule->setObjectName(QStringLiteral("recipeCamTabRule"));
    camTabRule->setFixedHeight(1);
    camTabRule->setFrameShape(QFrame::NoFrame);
    tabBl->addWidget(camTabRule);
    layout->addWidget(tabBlock);

    layout->addWidget(buildCameraSettingsTable(w));

    connectCameraTabComboSync();

    return w;
}

QWidget *RecipeConfigView::buildParamAlgoThresholds(QWidget *parent)
{
    auto *w = new QWidget(parent);
    w->setObjectName(QStringLiteral("recipeAlgoThresholdWrap"));
    w->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(4, 4, 4, 4);

    const QStringList labels = {
        QStringLiteral("死灯阈值"), QStringLiteral("亮灯阈值"), QStringLiteral("暗灯阈值"),
        QStringLiteral("串灯阈值"), QStringLiteral("串色R"),   QStringLiteral("串色G"),
        QStringLiteral("串色B"),   QStringLiteral("混色初筛"), QStringLiteral("混色筛选")};

    const int nRows = labels.size();
    auto *table     = new QTableWidget(nRows, 4, w);
    table->setObjectName(QStringLiteral("recipeAlgoRgbTable"));
    table->setAttribute(Qt::WA_StyledBackground, true);
    initRecipeGridTable(table);
    table->setHorizontalHeaderLabels({QStringLiteral("参数"), QStringLiteral("R"),
                                      QStringLiteral("G"), QStringLiteral("B")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(40);

    for (int r = 0; r < nRows; ++r) {
        table->setItem(r, 0, recipeTableLabelItem(labels[r]));
        QDoubleSpinBox *cols[3] = {nullptr, nullptr, nullptr};
        for (int c = 0; c < 3; ++c) {
            cols[c] = makeRecipeDoubleSpin(table, 0.0, 9999.99);
            table->setCellWidget(r, c + 1, cols[c]);
        }
        if (r < RecipeAlgoThresholdWidgets::kRowCount) {
            m_algoWidgets.rows[r].r = cols[0];
            m_algoWidgets.rows[r].g = cols[1];
            m_algoWidgets.rows[r].b = cols[2];
        }
    }

    setRecipeTableFixedHeight(table, nRows);
    layout->addWidget(table);

    auto *extraLayout = new QFormLayout(w);
    extraLayout->setSpacing(4);
    auto addExtraSpin = [&](const char *label, QDoubleSpinBox **store) {
        auto *s = makeRecipeDoubleSpin(w, 0.0, 9999.99);
        if (store)
            *store = s;
        extraLayout->addRow(QString::fromUtf8(label), s);
    };
    addExtraSpin("黑屏阈值", &m_algoWidgets.blackScreenThresh);
    addExtraSpin("黑屏亮灯阈值", &m_algoWidgets.blackScreenBrightThresh);
    addExtraSpin("坏点阈值", &m_algoWidgets.badPixelThresh);
    layout->addLayout(extraLayout);
    return w;
}

QWidget *RecipeConfigView::buildParamSpecialDetection(QWidget *parent)
{
    auto *w = new QWidget(parent);
    w->setObjectName(QStringLiteral("recipeSpecialDetectWrap"));
    w->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(4, 4, 4, 4);

    constexpr int kSpecRows = RecipeSpecialDetectWidgets::kRowCount;
    auto *table = new QTableWidget(kSpecRows, 4, w);
    table->setObjectName(QStringLiteral("recipeSpecialDetectTable"));
    table->setAttribute(Qt::WA_StyledBackground, true);
    initRecipeGridTable(table);
    table->setHorizontalHeaderLabels(
        {QStringLiteral("项"), QStringLiteral("开关"), QStringLiteral("灯点个数阈值"),
         QStringLiteral("处理方式")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table->horizontalHeader()->resizeSection(1, 88);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(38);

    const QStringList specNames = {QStringLiteral("大面积灭点"), QStringLiteral("非边缘缺陷"),
                                   QStringLiteral("线不良"), QStringLiteral("区块不良")};

    for (int r = 0; r < kSpecRows; ++r) {
        table->setItem(r, 0, recipeTableLabelItem(specNames[r]));

        m_specialDetectWidgets.rows[r].switchCell = makeSpecialDetectSwitchCell(true, table);
        table->setCellWidget(r, 1, m_specialDetectWidgets.rows[r].switchCell);

        m_specialDetectWidgets.rows[r].lampThreshold = makeRecipeIntSpin(table, 0, 9999);
        table->setCellWidget(r, 2, m_specialDetectWidgets.rows[r].lampThreshold);

        auto *sel = makeRecipeTableCombo(kRecipeSpecTableComboH, table,
                                         {QStringLiteral("自动下料"), QStringLiteral("报警")});
        table->setCellWidget(r, 3, sel);
        m_specialDetectWidgets.rows[r].action = sel;
    }

    setRecipeTableFixedHeight(table, kSpecRows);

    layout->addWidget(table);
    return w;
}

QWidget *RecipeConfigView::buildParamOtherSettings(QWidget *parent)
{
    auto *w = new QWidget(parent);
    w->setObjectName(QStringLiteral("recipeOtherWrap"));
    w->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(4, 4, 4, 4);

    constexpr int kOtherRowH = 44;
    auto *table = new QTableWidget(1, 2, w);
    table->setObjectName(QStringLiteral("recipeOtherTable"));
    table->setAttribute(Qt::WA_StyledBackground, true);
    initRecipeGridTable(table);
    table->setHorizontalHeaderLabels({QStringLiteral("项"), QStringLiteral("值")});
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setFixedHeight(kOtherRowH);
    table->verticalHeader()->setDefaultSectionSize(kOtherRowH);

    table->setItem(0, 0, recipeTableLabelItem(QStringLiteral("轨道宽度(mm)")));

    auto *valWrap = new QWidget(table);
    valWrap->setAttribute(Qt::WA_TranslucentBackground, true);
    auto *hl = new QHBoxLayout(valWrap);
    hl->setContentsMargins(4, 2, 4, 2);
    hl->setSpacing(0);
    m_otherTrackWidthSpin = makeRecipeDoubleSpin(valWrap, 0.0, 999999.0);
    hl->addWidget(m_otherTrackWidthSpin, 1);
    table->setCellWidget(0, 1, valWrap);

    // 与表头+内容行对齐即可；余量过大会在视口底部留出一条与底边框叠在一起的线
    table->setFixedHeight(kOtherRowH + kOtherRowH + 2 * table->frameWidth() + 2);

    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addWidget(table, 0, Qt::AlignTop);
    return w;
}

void RecipeConfigView::setupViewModel()
{
    if (m_viewModel)
        return;

    m_viewModel = new RecipeConfigViewModel(this);

    setupBindings();

    connect(m_viewModel, &RecipeConfigViewModel::sig_recipesChanged, this, &RecipeConfigView::refreshRecipeList);
    connect(m_viewModel, &RecipeConfigViewModel::sig_statusBarChanged, this, &RecipeConfigView::applyStatusBarToUi);
    connect(m_viewModel, &RecipeConfigViewModel::sig_statusChanged, this, [this]() {
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
    connect(m_viewModel, &RecipeConfigViewModel::sig_recipeSelected, this,
            &RecipeConfigView::resetToolbarCameraOnRecipeSwitch);

    const QString defaultId = RecipeRepository::defaultRecipeId();
    if (!defaultId.isEmpty())
        m_viewModel->slot_selectRecipe(defaultId);
    else
        resetToolbarCameraOnRecipeSwitch();
    refreshRecipeList();
    applyStatusBarToUi();

    if (m_recipeSearchEdit) {
        connect(m_recipeSearchEdit, &QLineEdit::textChanged, m_viewModel,
                &RecipeConfigViewModel::slot_applySearch);
    }

    if (m_recipeList) {
        connect(m_recipeList, &QListWidget::currentRowChanged, this, [this](int row) {
            if (!m_viewModel || row < 0 || row >= m_viewModel->recipes().size())
                return;
            const QString recipeId = m_viewModel->recipes().at(row).id;
            if (recipeId != m_viewModel->currentRecipeId())
                m_viewModel->slot_selectRecipe(recipeId);
        });
    }
}

void RecipeConfigView::setupBindings()
{
    if (!m_viewModel)
        return;

    bindTopBarCommands();

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;

    if (m_basicNameEdit) {
        DataBinding::bindStructField<RD>(
            m_viewModel, prop,
            [](const RD &d) { return d.id; },
            [](RD &d, const QString &v) {
                const QString id = sanitizeRecipeId(v);
                d.id         = id;
                d.basic.name = id;
            },
            m_basicNameEdit);
    }
    if (m_basicWidthSpin) {
        DataBinding::bindStructField<RD>(
            m_viewModel, prop,
            [](const RD &d) { return d.basic.width; },
            [](RD &d, int v) { d.basic.width = v; },
            m_basicWidthSpin);
    }
    if (m_basicHeightSpin) {
        DataBinding::bindStructField<RD>(
            m_viewModel, prop,
            [](const RD &d) { return d.basic.height; },
            [](RD &d, int v) { d.basic.height = v; },
            m_basicHeightSpin);
    }
    if (m_basicCoordCombo) {
        bindStructComboIndex(
            m_viewModel, prop,
            [](const RD &d) { return d.basic.coordSystemIndex; },
            [](RD &d, int v) { d.basic.coordSystemIndex = v; },
            m_basicCoordCombo);
    }

    if (m_toolbarCamCombo) {
        bindStructComboIndex(
            m_viewModel, prop,
            [this](const RD &) { return m_viewModel->activeCamera(); },
            [this](RD &d, int v) {
                m_viewModel->setActiveCamera(v);
                findOrCreateDebugEntry(d.debugs, static_cast<CameraIndex>(v),
                                       static_cast<ColorChannel>(m_viewModel->activeColorChannel()));
            },
            m_toolbarCamCombo);
        QObject::connect(m_toolbarCamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         m_viewModel, [this](int index) {
                             if (index >= 0)
                                 m_viewModel->setActiveCamera(index);
                         });
    }

    if (m_colorBtnGroup) {
        bindStructButtonGroupIndex(
            m_viewModel, prop,
            [this](const RD &) { return m_viewModel->activeColorChannel(); },
            [this](RD &d, int v) {
                m_viewModel->setActiveColorChannel(v);
                findOrCreateDebugEntry(d.debugs, static_cast<CameraIndex>(m_viewModel->activeCamera()),
                                       static_cast<ColorChannel>(v));
            },
            m_colorBtnGroup);
    }

    if (RecipePillSwitch *pill = dynamic_cast<RecipePillSwitch *>(m_roiSwitch)) {
        auto getRoiEnabled = [this](const RD &d) -> bool {
            const RecipeDebugToolbar *entry = findDebugEntry(
                d.debugs, static_cast<CameraIndex>(m_viewModel->activeCamera()),
                static_cast<ColorChannel>(m_viewModel->activeColorChannel()));
            return entry ? entry->roiEnabled : true;
        };
        auto setRoiEnabled = [this](RD &d, bool v) {
            findOrCreateDebugEntry(d.debugs, static_cast<CameraIndex>(m_viewModel->activeCamera()),
                                   static_cast<ColorChannel>(m_viewModel->activeColorChannel()))
                .roiEnabled = v;
        };

        pill->setChecked(getRoiEnabled(m_viewModel->document()));
        syncRoiModeComboEnabled(pill->isChecked());

        QObject::connect(m_viewModel, &ViewModel::propertyChanged, pill,
                         [pill, prop, getRoiEnabled, this](const QString &name, const QVariant &val) {
                             if (name == prop) {
                                 QSignalBlocker blocker(pill);
                                 pill->setChecked(getRoiEnabled(val.value<RD>()));
                                 syncRoiModeComboEnabled(pill->isChecked());
                             }
                         });
        pill->setOnToggled([this, prop, setRoiEnabled](bool checked) {
            RD doc = m_viewModel->getProperty(prop).value<RD>();
            setRoiEnabled(doc, checked);
            m_viewModel->setProperty(prop, QVariant::fromValue(doc));
            syncRoiModeComboEnabled(checked);

            constexpr int kNoDrawRoiMode = 0;
            if (m_roiModeCombo && m_roiModeCombo->currentIndex() != kNoDrawRoiMode)
                m_roiModeCombo->setCurrentIndex(kNoDrawRoiMode);
            applyToolbarRoiMode(kNoDrawRoiMode);
        });

        auto refreshRoiEnabledUi = [this, getRoiEnabled]() {
            if (RecipePillSwitch *p = dynamic_cast<RecipePillSwitch *>(m_roiSwitch)) {
                QSignalBlocker blocker(p);
                p->setChecked(getRoiEnabled(m_viewModel->document()));
                syncRoiModeComboEnabled(p->isChecked());
            }
        };
        connect(m_viewModel, &RecipeConfigViewModel::sig_activeCameraChanged, this, refreshRoiEnabledUi);
        connect(m_viewModel, &RecipeConfigViewModel::sig_activeColorChannelChanged, this,
                refreshRoiEnabledUi);
    }

    if (m_toolbarCamCombo) {
        QObject::connect(m_toolbarCamCombo,
                         QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                         &RecipeConfigView::onToolbarCameraChanged);
    }

    if (m_holdRoiBtn)
        connect(m_holdRoiBtn, &QPushButton::clicked, this, &RecipeConfigView::slot_holdRoiSaveClicked);

    if (m_viewRoiBtn)
        connect(m_viewRoiBtn, &QPushButton::clicked, this, &RecipeConfigView::slot_viewRoiClicked);

    bindCameraSettings();
    bindAlgoThresholds();
    bindSpecialDetection();
    bindOtherSettings();

    connect(m_viewModel, &ViewModel::propertyChanged, this,
            [this](const QString &name, const QVariant &) {
                if (name != RecipeConfigViewModel::PROP_DOCUMENT)
                    return;
                applyCameraSettingsToUi();
                applyAlgoThresholdsToUi();
                applySpecialDetectionToUi();
                applyOtherSettingsToUi();
            });

    applyAlgoThresholdsToUi();
    applySpecialDetectionToUi();
    applyOtherSettingsToUi();
}

namespace {

bool perCameraHasNonZeroLockedFields(const PerCameraParams &p)
{
    auto rgbNonZero = [](const RgbTriple &t) {
        return t.r != 0.0 || t.g != 0.0 || t.b != 0.0;
    };
    return rgbNonZero(p.seriesExposureMs) || rgbNonZero(p.seriesGrayLevel) || p.whiteExposureMs != 0
           || p.blackExposureMs != 0;
}

void zeroPerCameraLockedFields(PerCameraParams &p)
{
    p.seriesExposureMs  = {};
    p.seriesGrayLevel   = {};
    p.whiteExposureMs   = 0;
    p.blackExposureMs   = 0;
}

} // namespace

void RecipeConfigView::applyCameraSettingsEditability(bool isFirstCamera)
{
    auto setSpinEnabled = [](QSpinBox *spin, bool enabled) {
        if (spin)
            spin->setEnabled(enabled);
    };

    const bool lockedEditable = isFirstCamera;
    setSpinEnabled(m_camSettingsWidgets.seriesExposureR, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.seriesExposureG, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.seriesExposureB, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.seriesGrayR, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.seriesGrayG, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.seriesGrayB, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.whiteExposure, lockedEditable);
    setSpinEnabled(m_camSettingsWidgets.blackExposure, lockedEditable);
}

void RecipeConfigView::applyCameraSettingsToUi()
{
    if (!m_viewModel)
        return;

    const int ci             = m_viewModel->activeCamera();
    const bool isFirstCamera = (ci == 0);

    if (!isFirstCamera && ci >= 0) {
        RecipeDocument doc = m_viewModel->document();
        if (ci < doc.cameras.size() && perCameraHasNonZeroLockedFields(doc.cameras[ci])) {
            zeroPerCameraLockedFields(doc.cameras[ci]);
            m_viewModel->setProperty(RecipeConfigViewModel::PROP_DOCUMENT, QVariant::fromValue(doc));
        }
    }

    const RecipeDocument doc = m_viewModel->document();
    if (ci < 0 || ci >= doc.cameras.size())
        return;

    const PerCameraParams &p = doc.cameras.at(ci);

    auto rotationToComboIndex = [](int deg) {
        if (deg == 90)
            return 1;
        if (deg == 180)
            return 2;
        if (deg == 270)
            return 3;
        return 0;
    };

    m_syncingCameraSettings = true;

    auto setSpin = [](QSpinBox *spin, double v) {
        if (!spin)
            return;
        QSignalBlocker blocker(spin);
        spin->setValue(static_cast<int>(v));
    };

    setSpin(m_camSettingsWidgets.pureExposureR, p.pureExposureMs.r);
    setSpin(m_camSettingsWidgets.pureExposureG, p.pureExposureMs.g);
    setSpin(m_camSettingsWidgets.pureExposureB, p.pureExposureMs.b);
    setSpin(m_camSettingsWidgets.pureGrayR, p.pureGrayLevel.r);
    setSpin(m_camSettingsWidgets.pureGrayG, p.pureGrayLevel.g);
    setSpin(m_camSettingsWidgets.pureGrayB, p.pureGrayLevel.b);
    setSpin(m_camSettingsWidgets.seriesExposureR, p.seriesExposureMs.r);
    setSpin(m_camSettingsWidgets.seriesExposureG, p.seriesExposureMs.g);
    setSpin(m_camSettingsWidgets.seriesExposureB, p.seriesExposureMs.b);
    setSpin(m_camSettingsWidgets.seriesGrayR, p.seriesGrayLevel.r);
    setSpin(m_camSettingsWidgets.seriesGrayG, p.seriesGrayLevel.g);
    setSpin(m_camSettingsWidgets.seriesGrayB, p.seriesGrayLevel.b);
    setSpin(m_camSettingsWidgets.whiteExposure, p.whiteExposureMs);
    setSpin(m_camSettingsWidgets.blackExposure, p.blackExposureMs);

    if (kRecipeShowImageQualityUi) {
        if (auto *ql = dynamic_cast<RecipeQualityLabel *>(m_camSettingsWidgets.qualityR))
            ql->setQualityOk(p.qualityOkR);
        if (auto *ql = dynamic_cast<RecipeQualityLabel *>(m_camSettingsWidgets.qualityG))
            ql->setQualityOk(p.qualityNgG);
        if (auto *ql = dynamic_cast<RecipeQualityLabel *>(m_camSettingsWidgets.qualityB))
            ql->setQualityOk(p.qualityOkB);
    }

    if (m_camSettingsWidgets.rotation) {
        QSignalBlocker blocker(m_camSettingsWidgets.rotation);
        m_camSettingsWidgets.rotation->setCurrentIndex(rotationToComboIndex(p.rotationDeg));
    }

    m_syncingCameraSettings = false;
    applyCameraSettingsEditability(isFirstCamera);
}

void RecipeConfigView::bindCameraSettings()
{
    if (!m_viewModel)
        return;

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;

    auto ensureCameras = [](RD &doc) {
        if (doc.cameras.size() < kCameraCount)
            doc.cameras.resize(kCameraCount);
    };

    auto camIndex = [this]() { return m_viewModel ? m_viewModel->activeCamera() : 0; };

    auto bindPerCamSpin = [&](QSpinBox *spin, std::function<double(const PerCameraParams &)> get,
                              std::function<void(PerCameraParams &, double)> set) {
        if (!spin)
            return;
        QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                         [this, prop, get, set, ensureCameras, camIndex](int v) {
                             if (m_syncingCameraSettings || !m_viewModel)
                                 return;
                             RD doc = m_viewModel->document();
                             ensureCameras(doc);
                             const int ci = camIndex();
                             if (ci < 0 || ci >= kCameraCount)
                                 return;
                             set(doc.cameras[ci], static_cast<double>(v));
                             m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                         });
    };

    bindPerCamSpin(m_camSettingsWidgets.pureExposureR,
                   [](const PerCameraParams &p) { return p.pureExposureMs.r; },
                   [](PerCameraParams &p, double v) { p.pureExposureMs.r = v; });
    bindPerCamSpin(m_camSettingsWidgets.pureExposureG,
                   [](const PerCameraParams &p) { return p.pureExposureMs.g; },
                   [](PerCameraParams &p, double v) { p.pureExposureMs.g = v; });
    bindPerCamSpin(m_camSettingsWidgets.pureExposureB,
                   [](const PerCameraParams &p) { return p.pureExposureMs.b; },
                   [](PerCameraParams &p, double v) { p.pureExposureMs.b = v; });
    bindPerCamSpin(m_camSettingsWidgets.pureGrayR,
                   [](const PerCameraParams &p) { return p.pureGrayLevel.r; },
                   [](PerCameraParams &p, double v) { p.pureGrayLevel.r = v; });
    bindPerCamSpin(m_camSettingsWidgets.pureGrayG,
                   [](const PerCameraParams &p) { return p.pureGrayLevel.g; },
                   [](PerCameraParams &p, double v) { p.pureGrayLevel.g = v; });
    bindPerCamSpin(m_camSettingsWidgets.pureGrayB,
                   [](const PerCameraParams &p) { return p.pureGrayLevel.b; },
                   [](PerCameraParams &p, double v) { p.pureGrayLevel.b = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesExposureR,
                   [](const PerCameraParams &p) { return p.seriesExposureMs.r; },
                   [](PerCameraParams &p, double v) { p.seriesExposureMs.r = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesExposureG,
                   [](const PerCameraParams &p) { return p.seriesExposureMs.g; },
                   [](PerCameraParams &p, double v) { p.seriesExposureMs.g = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesExposureB,
                   [](const PerCameraParams &p) { return p.seriesExposureMs.b; },
                   [](PerCameraParams &p, double v) { p.seriesExposureMs.b = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesGrayR,
                   [](const PerCameraParams &p) { return p.seriesGrayLevel.r; },
                   [](PerCameraParams &p, double v) { p.seriesGrayLevel.r = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesGrayG,
                   [](const PerCameraParams &p) { return p.seriesGrayLevel.g; },
                   [](PerCameraParams &p, double v) { p.seriesGrayLevel.g = v; });
    bindPerCamSpin(m_camSettingsWidgets.seriesGrayB,
                   [](const PerCameraParams &p) { return p.seriesGrayLevel.b; },
                   [](PerCameraParams &p, double v) { p.seriesGrayLevel.b = v; });
    bindPerCamSpin(m_camSettingsWidgets.whiteExposure,
                   [](const PerCameraParams &p) { return p.whiteExposureMs; },
                   [](PerCameraParams &p, double v) { p.whiteExposureMs = static_cast<int>(v); });
    bindPerCamSpin(m_camSettingsWidgets.blackExposure,
                   [](const PerCameraParams &p) { return p.blackExposureMs; },
                   [](PerCameraParams &p, double v) { p.blackExposureMs = static_cast<int>(v); });

    auto bindQualityToggle = [&](QLabel *label, std::function<bool(const PerCameraParams &)> get,
                                 std::function<void(PerCameraParams &, bool)> set) {
        auto *ql = dynamic_cast<RecipeQualityLabel *>(label);
        if (!ql)
            return;
        ql->setOnToggled([this, prop, ql, get, set, ensureCameras, camIndex]() {
            if (m_syncingCameraSettings || !m_viewModel)
                return;
            RD doc = m_viewModel->document();
            ensureCameras(doc);
            const int ci = camIndex();
            if (ci < 0 || ci >= kCameraCount)
                return;
            const bool next = !get(doc.cameras[ci]);
            set(doc.cameras[ci], next);
            ql->setQualityOk(next);
            m_viewModel->setProperty(prop, QVariant::fromValue(doc));
        });
    };

    if (kRecipeShowImageQualityUi) {
        bindQualityToggle(m_camSettingsWidgets.qualityR,
                          [](const PerCameraParams &p) { return p.qualityOkR; },
                          [](PerCameraParams &p, bool v) { p.qualityOkR = v; });
        bindQualityToggle(m_camSettingsWidgets.qualityG,
                          [](const PerCameraParams &p) { return p.qualityNgG; },
                          [](PerCameraParams &p, bool v) { p.qualityNgG = v; });
        bindQualityToggle(m_camSettingsWidgets.qualityB,
                          [](const PerCameraParams &p) { return p.qualityOkB; },
                          [](PerCameraParams &p, bool v) { p.qualityOkB = v; });
    }

    if (m_camSettingsWidgets.rotation) {
        QObject::connect(m_camSettingsWidgets.rotation,
                         QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                         [this, prop, ensureCameras, camIndex](int index) {
                             if (m_syncingCameraSettings || !m_viewModel || index < 0)
                                 return;
                             static const int kRotDeg[] = {0, 90, 180, 270};
                             const int        deg       = (index < 4) ? kRotDeg[index] : 0;
                             RD               doc       = m_viewModel->document();
                             ensureCameras(doc);
                             const int ci = camIndex();
                             if (ci < 0 || ci >= kCameraCount)
                                 return;
                             doc.cameras[ci].rotationDeg = deg;
                             m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                         });
    }

    connect(m_viewModel, &RecipeConfigViewModel::sig_activeCameraChanged, this,
            &RecipeConfigView::applyCameraSettingsToUi);

    applyCameraSettingsToUi();
}

namespace {

AlgoThresholdRow &mutableAlgoRowAt(RecipeDocument &doc, int index)
{
    const QString name = algoThresholdNameAt(index);
    for (AlgoThresholdRow &row : doc.algo.rows) {
        if (row.name == name)
            return row;
    }
    AlgoThresholdRow created;
    created.name = name;
    doc.algo.rows.append(created);
    return doc.algo.rows.last();
}

void ensureSpecialItems(RecipeDocument &doc)
{
    static const QStringList kNames = {
        QStringLiteral("大面积灭点"), QStringLiteral("非边缘缺陷"),
        QStringLiteral("线不良"),     QStringLiteral("区块不良")};
    while (doc.specialItems.size() < RecipeSpecialDetectWidgets::kRowCount) {
        const int i = doc.specialItems.size();
        doc.specialItems.append(
            {kNames.value(i), true, 0, QStringLiteral("报警")});
    }
}

SpecialDetectItem specialItemAt(const RecipeDocument &doc, int row)
{
    if (row >= 0 && row < doc.specialItems.size())
        return doc.specialItems.at(row);
    return {};
}

SpecialDetectItem &mutableSpecialItemAt(RecipeDocument &doc, int row)
{
    ensureSpecialItems(doc);
    return doc.specialItems[row];
}

RecipePillSwitch *pillFromSwitchCell(QWidget *cell)
{
    if (!cell)
        return nullptr;
    // Qt6 qobject_cast/findChild both require Q_OBJECT on the target class;
    // RecipePillSwitch is file-local so MOC cannot see it.  Use dynamic_cast instead.
    for (auto *child : cell->children()) {
        if (auto *sw = dynamic_cast<RecipePillSwitch *>(child))
            return sw;
    }
    return nullptr;
}

} // namespace

void RecipeConfigView::applyAlgoThresholdsToUi()
{
    if (!m_viewModel)
        return;

    const RecipeDocument doc = m_viewModel->document();
    m_syncingExtraParams     = true;

    auto setDoubleSpin = [](QDoubleSpinBox *spin, double v) {
        if (!spin)
            return;
        QSignalBlocker blocker(spin);
        spin->setValue(v);
    };

    for (int r = 0; r < RecipeAlgoThresholdWidgets::kRowCount; ++r) {
        const RgbTriple rgb = algoRowValuesAt(doc.algo, r);
        setDoubleSpin(m_algoWidgets.rows[r].r, rgb.r);
        setDoubleSpin(m_algoWidgets.rows[r].g, rgb.g);
        setDoubleSpin(m_algoWidgets.rows[r].b, rgb.b);
    }

    setDoubleSpin(m_algoWidgets.blackScreenThresh, doc.algo.blackScreenThresh);
    setDoubleSpin(m_algoWidgets.blackScreenBrightThresh, doc.algo.blackScreenBrightThresh);
    setDoubleSpin(m_algoWidgets.badPixelThresh, doc.algo.badPixelThresh);

    m_syncingExtraParams = false;
}

void RecipeConfigView::bindAlgoThresholds()
{
    if (!m_viewModel)
        return;

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;

    auto bindAlgoDouble = [&](QDoubleSpinBox *spin,
                              std::function<double(const AlgoThresholds &)> get,
                              std::function<void(AlgoThresholds &, double)> set) {
        if (!spin)
            return;
        QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                         [this, prop, get, set](double v) {
                             if (m_syncingExtraParams || !m_viewModel)
                                 return;
                             RD doc = m_viewModel->document();
                             set(doc.algo, v);
                             m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                         });
    };

    auto bindAlgoRgb = [&](int row, QDoubleSpinBox *spin,
                           std::function<double &(RgbTriple &)> member) {
        if (!spin)
            return;
        QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                         [this, prop, row, member](double v) {
                             if (m_syncingExtraParams || !m_viewModel)
                                 return;
                             RD doc = m_viewModel->document();
                             member(mutableAlgoRowAt(doc, row).values) = v;
                             m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                         });
    };

    for (int r = 0; r < RecipeAlgoThresholdWidgets::kRowCount; ++r) {
        bindAlgoRgb(r, m_algoWidgets.rows[r].r,
                    [](RgbTriple &t) -> double & { return t.r; });
        bindAlgoRgb(r, m_algoWidgets.rows[r].g,
                    [](RgbTriple &t) -> double & { return t.g; });
        bindAlgoRgb(r, m_algoWidgets.rows[r].b,
                    [](RgbTriple &t) -> double & { return t.b; });
    }

    bindAlgoDouble(m_algoWidgets.blackScreenThresh,
                   [](const AlgoThresholds &a) { return a.blackScreenThresh; },
                   [](AlgoThresholds &a, double v) { a.blackScreenThresh = v; });
    bindAlgoDouble(m_algoWidgets.blackScreenBrightThresh,
                   [](const AlgoThresholds &a) { return a.blackScreenBrightThresh; },
                   [](AlgoThresholds &a, double v) { a.blackScreenBrightThresh = v; });
    bindAlgoDouble(m_algoWidgets.badPixelThresh,
                   [](const AlgoThresholds &a) { return a.badPixelThresh; },
                   [](AlgoThresholds &a, double v) { a.badPixelThresh = v; });
}

void RecipeConfigView::applySpecialDetectionToUi()
{
    if (!m_viewModel)
        return;

    const RecipeDocument doc = m_viewModel->document();
    m_syncingExtraParams     = true;

    for (int r = 0; r < RecipeSpecialDetectWidgets::kRowCount; ++r) {
        const SpecialDetectItem item = specialItemAt(doc, r);
        if (RecipePillSwitch *sw = pillFromSwitchCell(m_specialDetectWidgets.rows[r].switchCell))
            sw->setChecked(item.enabled);
        if (QSpinBox *spin = m_specialDetectWidgets.rows[r].lampThreshold) {
            QSignalBlocker blocker(spin);
            spin->setValue(item.lampThreshold);
        }
        if (QComboBox *combo = m_specialDetectWidgets.rows[r].action) {
            QSignalBlocker blocker(combo);
            combo->setCurrentText(item.action);
        }
    }

    m_syncingExtraParams = false;
}

void RecipeConfigView::bindSpecialDetection()
{
    if (!m_viewModel)
        return;

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;

    for (int r = 0; r < RecipeSpecialDetectWidgets::kRowCount; ++r) {
        if (RecipePillSwitch *sw = pillFromSwitchCell(m_specialDetectWidgets.rows[r].switchCell)) {
            bindRecipePillSwitch(
                m_viewModel, prop,
                [r](const RD &d) { return specialItemAt(d, r).enabled; },
                [r](RD &d, bool v) { mutableSpecialItemAt(d, r).enabled = v; },
                sw);
        }

        if (QSpinBox *spin = m_specialDetectWidgets.rows[r].lampThreshold) {
            QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                             [this, prop, r](int v) {
                                 if (m_syncingExtraParams || !m_viewModel)
                                     return;
                                 RD doc = m_viewModel->document();
                                 mutableSpecialItemAt(doc, r).lampThreshold = v;
                                 m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                             });
        }

        if (QComboBox *combo = m_specialDetectWidgets.rows[r].action) {
            QObject::connect(combo, &QComboBox::currentTextChanged, this,
                             [this, prop, r](const QString &text) {
                                 if (m_syncingExtraParams || !m_viewModel)
                                     return;
                                 RD doc = m_viewModel->document();
                                 mutableSpecialItemAt(doc, r).action = text;
                                 m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                             });
        }
    }
}

void RecipeConfigView::applyOtherSettingsToUi()
{
    if (!m_viewModel || !m_otherTrackWidthSpin)
        return;

    m_syncingExtraParams = true;
    QSignalBlocker blocker(m_otherTrackWidthSpin);
    m_otherTrackWidthSpin->setValue(m_viewModel->document().other.trackWidthMm);
    m_syncingExtraParams = false;
}

void RecipeConfigView::bindOtherSettings()
{
    if (!m_viewModel || !m_otherTrackWidthSpin)
        return;

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;

    QObject::connect(m_otherTrackWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     this, [this, prop](double v) {
                         if (m_syncingExtraParams || !m_viewModel)
                             return;
                         RD doc = m_viewModel->document();
                         doc.other.trackWidthMm = v;
                         m_viewModel->setProperty(prop, QVariant::fromValue(doc));
                     });
}

void RecipeConfigView::applyStatusBarToUi()
{
    if (!m_viewModel)
        return;

    const RecipeStatusBar bar = m_viewModel->statusBar();
    if (m_statusRecipeLbl)
        m_statusRecipeLbl->setText(QStringLiteral("当前配方: %1").arg(bar.currentRecipeName));
    if (kRecipeShowImageQualityUi && m_statusQualityLbl) {
        m_statusQualityLbl->setText(QStringLiteral("图像质量: %1").arg(bar.imageQualityText));
        m_statusQualityLbl->setObjectName(bar.imageQualityOk ? QStringLiteral("pillActive")
                                                             : QStringLiteral("pill"));
        m_statusQualityLbl->style()->unpolish(m_statusQualityLbl);
        m_statusQualityLbl->style()->polish(m_statusQualityLbl);
    }
    if (m_statusParamLbl) {
        m_statusParamLbl->setText(QStringLiteral("参数状态: %1").arg(bar.paramStatusText));
        m_statusParamLbl->setObjectName(bar.canSave ? QStringLiteral("pillActive")
                                                    : QStringLiteral("pill"));
        m_statusParamLbl->style()->unpolish(m_statusParamLbl);
        m_statusParamLbl->style()->polish(m_statusParamLbl);
    }
}

void RecipeConfigView::bindTopBarCommands()
{
    if (!m_viewModel)
        return;
    if (m_addRecipeBtn) {
        connect(m_addRecipeBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            QString name;
            if (!promptRecipeName(&name, QStringLiteral("新配方")))
                return;
            m_viewModel->slot_addRecipe(name);
        });
    }
    if (m_copyRecipeBtn) {
        syncCopyRecipeButtonEnabled();
        connect(m_viewModel, &RecipeConfigViewModel::sig_recipesChanged, this,
                &RecipeConfigView::syncCopyRecipeButtonEnabled);
        connect(m_viewModel, &ViewModel::propertyChanged, this,
                [this](const QString &name, const QVariant &) {
                    if (name == RecipeConfigViewModel::PROP_DOCUMENT)
                        syncCopyRecipeButtonEnabled();
                });
        connect(m_copyRecipeBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            if (!isRecipeListSelectionValid()) {
                m_viewModel->setStatus(QStringLiteral("请先选择有效配方"));
                return;
            }
            const QString defaultName =
                m_viewModel->document().id + QStringLiteral(" (副本)");
            QString name;
            if (!promptRecipeName(&name, defaultName, QStringLiteral("复制配方")))
                return;
            m_viewModel->slot_copyRecipe(name);
        });
    }
    if (m_removeRecipeBtn) {
        syncRemoveRecipeButtonEnabled();
        connect(m_viewModel, &RecipeConfigViewModel::sig_recipesChanged, this,
                &RecipeConfigView::syncRemoveRecipeButtonEnabled);
        connect(m_viewModel, &ViewModel::propertyChanged, this,
                [this](const QString &name, const QVariant &) {
                    if (name == RecipeConfigViewModel::PROP_DOCUMENT)
                        syncRemoveRecipeButtonEnabled();
                });
        connect(m_removeRecipeBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel || !m_viewModel->canRemoveRecipe())
                return;
            if (!confirmRemoveRecipe())
                return;
            m_viewModel->slot_removeRecipe();
        });
    }
    if (m_importRecipeBtn) {
        connect(m_importRecipeBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            const QStringList files = QFileDialog::getOpenFileNames(
                this, QStringLiteral("选择要导入的配方 JSON 文件"), QString(),
                QStringLiteral("JSON 文件 (*.json);;所有文件 (*.*)"));
            if (files.isEmpty())
                return;
            m_viewModel->slot_importRecipesFromFiles(files);
        });
    }
    if (m_exportRecipeBtn) {
        connect(m_exportRecipeBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            const QString dir =
                QFileDialog::getExistingDirectory(this, QStringLiteral("选择导出路径"));
            if (dir.isEmpty())
                return;
            m_viewModel->slot_exportAllRecipes(dir);
        });
    }
    if (m_setDefaultRecipeBtn)
        DataBinding::bindCommand(m_viewModel->setDefaultCommand(), m_setDefaultRecipeBtn);
    if (m_saveRecipeBtn) {
        DataBinding::bindCommand(m_viewModel->saveCommand(), m_saveRecipeBtn, [this]() {
            capturePreviewRoiToDocument();
        });
    }

    connect(m_viewModel, &RecipeConfigViewModel::sig_saveSucceeded, this, &RecipeConfigView::showSaveSuccessTip);
}

void RecipeConfigView::capturePreviewRoiToDocument()
{
    if (!m_viewModel)
        return;
    QList<double> roi;
    E_ItemType itemType = E_TYPE_NONE;
    if (m_graphicsView)
        m_graphicsView->GetRoi(kRecipePreviewWinId, itemType, roi);
    Q_UNUSED(itemType);
    m_viewModel->slot_setCurrentRoiPoints(roi);
}

void RecipeConfigView::resetToolbarCameraOnRecipeSwitch()
{
    if (!m_toolbarCamCombo || !m_viewModel || m_toolbarCamCombo->count() <= 0)
        return;

    constexpr int kFirstCamera = 0;
    m_viewModel->setActiveCamera(kFirstCamera);

    if (m_toolbarCamCombo->currentIndex() != kFirstCamera) {
        m_toolbarCamCombo->setCurrentIndex(kFirstCamera);
        return;
    }

    using RD = RecipeDocument;
    const QString prop = RecipeConfigViewModel::PROP_DOCUMENT;
    if (m_viewModel->hasProperty(prop)) {
        RD doc = m_viewModel->getProperty(prop).value<RD>();
        findOrCreateDebugEntry(doc.debugs, static_cast<CameraIndex>(kFirstCamera),
                               static_cast<ColorChannel>(m_viewModel->activeColorChannel()));
        m_viewModel->setProperty(prop, QVariant::fromValue(doc));
    }

    if (m_camSettingsTabGroup) {
        if (QAbstractButton *b = m_camSettingsTabGroup->button(kFirstCamera)) {
            if (!b->isChecked())
                b->setChecked(true);
        }
    }

    onToolbarCameraChanged(kFirstCamera);
}

void RecipeConfigView::onToolbarCameraChanged(int index)
{
    if (index < 0)
        return;

    constexpr int kPlaceholderPage     = 0;
    constexpr int kDefaultRoiModeCombo = 0; ///< 不绘制ROI

    if (m_previewStack)
        m_previewStack->setCurrentIndex(kPlaceholderPage);

    syncPreviewButtonFromCaptureState();

    if (!m_roiModeCombo)
        return;

    if (m_roiModeCombo->currentIndex() != kDefaultRoiModeCombo)
        m_roiModeCombo->setCurrentIndex(kDefaultRoiModeCombo);

    applyToolbarRoiMode(kDefaultRoiModeCombo);
}

void RecipeConfigView::syncRoiModeComboEnabled(bool roiEnabled)
{
    if (m_roiModeCombo)
        m_roiModeCombo->setEnabled(roiEnabled);
}

void RecipeConfigView::applyToolbarRoiMode(int index)
{
    if (!m_graphicsView)
        return;
    switch (index) {
    case 0:
        m_graphicsView->DrawClean(kRecipePreviewWinId);
        break;
    case 1:
        m_graphicsView->DrawRect(kRecipePreviewWinId);
        break;
    case 2:
        m_graphicsView->DrawPolygon(kRecipePreviewWinId);
        break;
    default:
        break;
    }
}

void RecipeConfigView::slot_viewRoiClicked()
{
    if (!m_graphicsView || !m_viewModel)
        return;

    const int savedModeIndex = m_roiModeCombo ? m_roiModeCombo->currentIndex() : 0;

    constexpr int kNoDrawRoiMode = 0;
    if (m_roiModeCombo && m_roiModeCombo->currentIndex() != kNoDrawRoiMode)
        m_roiModeCombo->setCurrentIndex(kNoDrawRoiMode);
    applyToolbarRoiMode(kNoDrawRoiMode);

    const RecipeDebugToolbar toolbar = m_viewModel->debugToolbar();
    const E_ItemType type = (savedModeIndex <= 1) ? E_TYPE_RECT : E_TYPE_POLYGON;
    m_graphicsView->SetRoi(kRecipePreviewWinId, type, toolbar.rois);
}

void RecipeConfigView::slot_holdRoiSaveClicked()
{
    if (!m_viewModel)
        return;

    bool roiEnabled = false;
    if (RecipePillSwitch *pill = dynamic_cast<RecipePillSwitch *>(m_roiSwitch))
        roiEnabled = pill->isChecked();

    QList<double> roi;
    if (roiEnabled && m_graphicsView) {
        E_ItemType itemType = E_TYPE_NONE;
        m_graphicsView->GetRoi(kRecipePreviewWinId, itemType, roi);
    }
    m_viewModel->slot_saveCurrentRoiPoints(roiEnabled, roi);
}

void RecipeConfigView::showPreviewImage(const QImage &image)
{
    if (!m_previewStack)
        return;

    constexpr int kPlaceholderPage = 0;
    constexpr int kGraphicsPage    = 1;

    if (!image.isNull() && m_graphicsView && m_previewStack->count() > kGraphicsPage) {
        m_graphicsView->SetImage(kRecipePreviewWinId, image);
        m_previewStack->setCurrentIndex(kGraphicsPage);
    } else {
        m_previewStack->setCurrentIndex(kPlaceholderPage);
    }
}

void RecipeConfigView::applyResultChipLabels(DetectItemKind kind, int r, int g, int b)
{
    const int slot = resultChipSlotIndex(kind);
    if (slot < 0 || slot >= static_cast<int>(m_resultChipLabels.size()))
        return;

    const RecipeResultChipLabels &labels = m_resultChipLabels[static_cast<size_t>(slot)];
    const auto fmtRgb = [](int v, const QString &ch) {
        return QStringLiteral("%1:%2").arg(ch).arg(v < 0 ? QStringLiteral("-") : QString::number(v));
    };
    if (labels.r)
        labels.r->setText(fmtRgb(r, QStringLiteral("R")));
    if (labels.g)
        labels.g->setText(fmtRgb(g, QStringLiteral("G")));
    if (labels.b)
        labels.b->setText(fmtRgb(b, QStringLiteral("B")));
    if (labels.total)
        labels.total->setText(QString::number(r + g + b));
}

void RecipeConfigView::slot_debug_pure_color_defect_count(const QVariantList &args)
{
    if (args.size() < 3)
        return;

    const auto kind = static_cast<DetectItemKind>(args.at(0).toInt());
    const int colorIdx = args.at(1).toInt();
    const int count    = args.at(2).toInt();
    if (colorIdx < 0 || colorIdx > 2)
        return;

    int r = 0;
    int g = 0;
    int b = 0;
    if (m_viewModel) {
        RecipeDocument doc = m_viewModel->document();
        DefectResultChip *target = nullptr;
        for (DefectResultChip &chip : doc.debugResultChips) {
            if (chip.item == kind) {
                target = &chip;
                break;
            }
        }
        if (!target) {
            DefectResultChip chip;
            chip.item = kind;
            doc.debugResultChips.append(chip);
            target = &doc.debugResultChips.last();
        }
        if (colorIdx == 0)
            target->r = count;
        else if (colorIdx == 1)
            target->g = count;
        else
            target->b = count;
        target->total = target->r + target->g + target->b;
        r             = target->r;
        g             = target->g;
        b             = target->b;
        m_viewModel->setProperty(RecipeConfigViewModel::PROP_DOCUMENT, QVariant::fromValue(doc));
    }
    applyResultChipLabels(kind, r, g, b);
}

bool RecipeConfigView::tryFillStepDebugCameraContext(QString *cameraName,
                                                     QString *cameraPath,
                                                     int *xResolution,
                                                     int *yResolution,
                                                     int *elemSize) const
{
    if (!cameraName || !cameraPath || !xResolution || !yResolution || !elemSize || !m_toolbarCamCombo) {
        return false;
    }

    const QString name = m_toolbarCamCombo->currentText();
    if (name.isEmpty()) {
        const_cast<RecipeConfigView *>(this)->appendLog(QStringLiteral("请先选择相机"),
                                                        QStringLiteral("WARN"));
        return false;
    }

    const int camIndex = m_toolbarCamCombo->currentIndex();
    const std::vector<std::string> cameraPaths = DeviceConfigReader::loadCameraPaths();
    QString path;
    if (camIndex >= 0 && camIndex < static_cast<int>(cameraPaths.size()))
        path = QString::fromStdString(cameraPaths[static_cast<size_t>(camIndex)]);
    if (path.isEmpty()) {
        const_cast<RecipeConfigView *>(this)->appendLog(QStringLiteral("当前相机未配置输出路径"),
                                                        QStringLiteral("WARN"));
        return false;
    }

    CameraImageConfig camImageConfig;
    if (!DeviceConfigReader::loadCameraImageConfig(name, &camImageConfig)) {
        const_cast<RecipeConfigView *>(this)->appendLog(
            QStringLiteral("当前相机未配置有效的图像分辨率"), QStringLiteral("WARN"));
        return false;
    }

    const auto format = imageFormatFromPixelFormatString(camImageConfig.pixelFormat);
    if (camImageConfig.xResolution <= 0 || camImageConfig.yResolution <= 0
        || format == ImageFormat::UNKNOWN) {
        const_cast<RecipeConfigView *>(this)->appendLog(
            QStringLiteral("当前相机未配置有效的图像分辨率"), QStringLiteral("WARN"));
        return false;
    }

    const int es = elemSizeFromImageFormat(format);
    if (es <= 0) {
        const_cast<RecipeConfigView *>(this)->appendLog(
            QStringLiteral("当前相机未配置有效的像素格式"), QStringLiteral("WARN"));
        return false;
    }

    *cameraName  = name;
    *cameraPath  = path;
    *xResolution = camImageConfig.xResolution;
    *yResolution = camImageConfig.yResolution;
    *elemSize    = es;
    return true;
}

void RecipeConfigView::syncToolbarColorChannelFromUi()
{
    if (!m_viewModel || !m_colorBtnGroup)
        return;
    const int colorId = m_colorBtnGroup->checkedId();
    if (colorId < static_cast<int>(ColorChannel::Red)
        || colorId > static_cast<int>(ColorChannel::White))
        return;
    if (m_viewModel->activeColorChannel() != colorId)
        m_viewModel->setActiveColorChannel(colorId);
}

void RecipeConfigView::triggerStepDebugPureColorDetect()
{
    if (!m_viewModel || !m_toolbarCamCombo) {
        return;
    }
    syncToolbarColorChannelFromUi();

    const auto colorChannel = static_cast<ColorChannel>(m_viewModel->activeColorChannel());
    if (colorChannel == ColorChannel::Black || colorChannel == ColorChannel::White) {
        QMessageBox::warning(
            this,
            QStringLiteral("纯色检测"),
            QStringLiteral("请先选择「红」「绿」「蓝」中的一种颜色通道，黑/白通道不支持纯色检测。"));
        return;
    }

    QString cameraName;
    QString cameraPath;
    int xResolution = 0;
    int yResolution = 0;
    int elemSize    = 0;
    if (!tryFillStepDebugCameraContext(&cameraName, &cameraPath, &xResolution, &yResolution, &elemSize)) {
        return;
    }

    RecipeStepDebugPureColorRequest request;
    request.recipe       = m_viewModel->document();
    request.cameraName   = cameraName;
    request.cameraPath   = cameraPath;
    request.xResolution  = xResolution;
    request.yResolution  = yResolution;
    request.elemSize     = elemSize;
    request.colorChannel = colorChannel;
    request.rawImagePath = m_stepDebugMainRawPath;
    request.debugToolbar = m_viewModel->debugToolbar();

    const int cameraIndex = cameraIndexFromName(cameraName);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < request.recipe.cameras.size())
                                    ? request.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};
    const size_t colorIdx = colorChannelIndex(colorChannel);
    const std::vector<std::string> softScreenKeys = DeviceConfigReader::loadSoftScreenKeys();
    request.softDisplay.colorChannel = colorChannel;
    request.softDisplay.panelWidth   = request.recipe.basic.width;
    request.softDisplay.panelHeight  = request.recipe.basic.height;
    request.softDisplay.brightness =
        static_cast<int>(rgbChannelValue(cam.pureGrayLevel, colorIdx));
    request.softDisplay.dispDelayMs =
        SystemConfigRepository::load().generalSettings.switchDelayMs;
    request.softDisplay.softScreenKey =
        softScreenKeys.empty() ? QStringLiteral("winScreen")
                               : QString::fromStdString(softScreenKeys.front());

    m_viewModel->setStatus(QStringLiteral("纯色检测进行中…"));
    QPointer<RecipeConfigView> self(this);

    std::thread([self, request = std::move(request)]() mutable {
        const RecipeStepDebugPureColorResult result = runRecipeStepDebugPureColor(request);
        QMetaObject::invokeMethod(
            qApp,
            [self, result]() {
                if (!self || !self->m_viewModel)
                    return;

                if (result.err) {
                    const QString msg = QStringLiteral("纯色检测失败:%1")
                                            .arg(QString::fromStdString(result.err->Message()));
                    self->m_viewModel->setStatus(msg);
                    return;
                }

                self->m_stepDebugMainRawPath.clear();
                const QString msg =
                    QStringLiteral("纯色检测完成，结果已保存至 %1").arg(result.outputDir);
                self->m_viewModel->setStatus(msg);
            },
            Qt::QueuedConnection);
    }).detach();
}

void RecipeConfigView::triggerStepDebugStringLightDetect()
{
    if (!m_viewModel || !m_toolbarCamCombo) {
        return;
    }
    syncToolbarColorChannelFromUi();

    const auto colorChannel = static_cast<ColorChannel>(m_viewModel->activeColorChannel());
    if (colorChannel == ColorChannel::Black || colorChannel == ColorChannel::White) {
        QMessageBox::warning(
            this,
            QStringLiteral("串灯检测"),
            QStringLiteral("请先选择「红」「绿」「蓝」中的一种颜色通道，黑/白通道不支持串灯检测。"));
        return;
    }

    QString cameraName;
    QString cameraPath;
    int xResolution = 0;
    int yResolution = 0;
    int elemSize    = 0;
    if (!tryFillStepDebugCameraContext(&cameraName, &cameraPath, &xResolution, &yResolution, &elemSize)) {
        return;
    }

    const QString contourPath = stepDebugPureContourFilePath(cameraPath, colorChannel);
    if (!QFileInfo::exists(contourPath)) {
        QMessageBox::warning(this,
                             QStringLiteral("串灯检测"),
                             QStringLiteral("请先完成纯色检测。"));
        return;
    }

    QString leftObliqueRawPath;
    QString rightObliqueRawPath;
    if (!m_stepDebugMainRawPath.isEmpty()) {
        leftObliqueRawPath = m_stepDebugMainRawPath;

        const QString initialDir = QFileInfo(leftObliqueRawPath).absolutePath();
        const QString rawFilter  = QStringLiteral("RAW 图像 (*.raw);;所有文件 (*.*)");

        rightObliqueRawPath = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择右斜图像"),
            initialDir,
            rawFilter);
        if (rightObliqueRawPath.isEmpty())
            return;
    }

    RecipeStepDebugStringLightRequest request;
    request.recipe              = m_viewModel->document();
    request.cameraName          = cameraName;
    request.cameraPath          = cameraPath;
    request.xResolution         = xResolution;
    request.yResolution         = yResolution;
    request.elemSize            = elemSize;
    request.colorChannel        = colorChannel;
    request.leftObliqueRawPath  = leftObliqueRawPath;
    request.rightObliqueRawPath = rightObliqueRawPath;
    request.debugToolbar        = m_viewModel->debugToolbar();

    const int cameraIndex = cameraIndexFromName(cameraName);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < request.recipe.cameras.size())
                                    ? request.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};
    const size_t colorIdx = colorChannelIndex(colorChannel);
    const std::vector<std::string> softScreenKeys = DeviceConfigReader::loadSoftScreenKeys();
    request.softDisplay.colorChannel = colorChannel;
    request.softDisplay.panelWidth   = request.recipe.basic.width;
    request.softDisplay.panelHeight  = request.recipe.basic.height;
    request.softDisplay.brightness =
        static_cast<int>(rgbChannelValue(cam.seriesGrayLevel, colorIdx));
    request.softDisplay.dispDelayMs =
        SystemConfigRepository::load().generalSettings.switchDelayMs;
    request.softDisplay.softScreenKey =
        softScreenKeys.empty() ? QStringLiteral("winScreen")
                               : QString::fromStdString(softScreenKeys.front());

    m_viewModel->setStatus(QStringLiteral("串灯检测进行中…"));
    QPointer<RecipeConfigView> self(this);

    std::thread([self, request = std::move(request)]() mutable {
        const RecipeStepDebugStringLightResult result = runRecipeStepDebugStringLight(request);
        QMetaObject::invokeMethod(
            qApp,
            [self, result]() {
                if (!self || !self->m_viewModel)
                    return;

                if (result.err) {
                    const QString msg = QStringLiteral("串灯检测失败:%1")
                                            .arg(QString::fromStdString(result.err->Message()));
                    self->m_viewModel->setStatus(msg);
                    return;
                }

                self->m_stepDebugMainRawPath.clear();
                const QString msg =
                    QStringLiteral("串灯检测完成，结果已保存至 %1").arg(result.outputDir);
                self->m_viewModel->setStatus(msg);
            },
            Qt::QueuedConnection);
    }).detach();
}

/**
 * 单步调试前重置：结果输出区 8 个芯片 R/G/B/总数归零（document.debugResultChips + UI），
 * 并删除 exe/sample/{当前相机}/ 下除 *.contour 以外的所有文件（保留目录与 contour 文件）。
 */
void RecipeConfigView::resetStepDebugResultOutput()
{
    static const DetectItemKind kResultChipKinds[] = {
        DetectItemKind::DeadLed,        DetectItemKind::DimLed,       DetectItemKind::BrightLed,
        DetectItemKind::ColorVariation, DetectItemKind::ColorMapDead, DetectItemKind::ColorMapLive,
        DetectItemKind::MixedColor,     DetectItemKind::BlackScreenOn,
    };
    for (DetectItemKind kind : kResultChipKinds)
        applyResultChipLabels(kind, 0, 0, 0);

    if (m_viewModel) {
        RecipeDocument doc = m_viewModel->document();
        for (DefectResultChip &chip : doc.debugResultChips) {
            chip.r     = 0;
            chip.g     = 0;
            chip.b     = 0;
            chip.total = 0;
        }
        m_viewModel->setProperty(RecipeConfigViewModel::PROP_DOCUMENT, QVariant::fromValue(doc));
    }

    if (!m_toolbarCamCombo)
        return;

    const int camIndex = m_toolbarCamCombo->currentIndex();
    const std::vector<std::string> cameraPaths = DeviceConfigReader::loadCameraPaths();
    QString cameraDirName;
    if (camIndex >= 0 && camIndex < static_cast<int>(cameraPaths.size()))
        cameraDirName = QString::fromStdString(cameraPaths[static_cast<size_t>(camIndex)]);
    if (cameraDirName.isEmpty())
        cameraDirName = m_toolbarCamCombo->currentText();
    if (cameraDirName.isEmpty())
        return;

    const QString sampleBase =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("sample"));
    QDir cameraSampleDir(QDir(sampleBase).filePath(cameraDirName));
    if (!cameraSampleDir.exists())
        return;

    const QFileInfoList entries =
        cameraSampleDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden);
    for (const QFileInfo &info : entries) {
        if (info.suffix().compare(QStringLiteral("contour"), Qt::CaseInsensitive) == 0)
            continue;
        QFile::remove(info.absoluteFilePath());
    }
}

QString RecipeConfigView::resultChipSampleImagePath(DetectItemKind kind) const
{
    if (!m_viewModel || !m_toolbarCamCombo)
        return {};

    const int camIndex = m_toolbarCamCombo->currentIndex();
    const std::vector<std::string> cameraPaths = DeviceConfigReader::loadCameraPaths();
    QString cameraPath;
    if (camIndex >= 0 && camIndex < static_cast<int>(cameraPaths.size()))
        cameraPath = QString::fromStdString(cameraPaths[static_cast<size_t>(camIndex)]);
    if (cameraPath.isEmpty())
        cameraPath = m_toolbarCamCombo->currentText();
    if (cameraPath.isEmpty())
        return {};

    const auto colorChannel = static_cast<ColorChannel>(m_viewModel->activeColorChannel());
    if (colorChannel != ColorChannel::Red && colorChannel != ColorChannel::Green
        && colorChannel != ColorChannel::Blue)
        return {};

    const char *fileSuffix = nullptr;
    switch (kind) {
    case DetectItemKind::DeadLed:
        fileSuffix = "_dead.jpg";
        break;
    case DetectItemKind::DimLed:
        fileSuffix = "_dark.jpg";
        break;
    case DetectItemKind::BrightLed:
        fileSuffix = "_bright.jpg";
        break;
    case DetectItemKind::ColorVariation:
        fileSuffix = "_color.jpg";
        break;
    case DetectItemKind::ColorMapDead:
        fileSuffix = "_string_dead.jpg";
        break;
    case DetectItemKind::ColorMapLive:
        fileSuffix = "_string_live.jpg";
        break;
    default:
        return {};
    }

    const QString prefix = QString::fromLatin1(
        common::algorithm::PureDetectColorToString(toPureDetectColor(colorChannel)));
    const QString fileName = prefix + QString::fromLatin1(fileSuffix);
    return QDir(sampleCameraOutputDirPath(cameraPath)).filePath(fileName);
}

void RecipeConfigView::slot_resultChipClicked(DetectItemKind kind)
{
    QImage previewImage;
    const QString path = resultChipSampleImagePath(kind);
    if (!path.isEmpty())
        previewImage = QImage(path);

    if (m_graphicsView)
        m_graphicsView->DrawClean(kRecipePreviewWinId);
    showPreviewImage(previewImage);
}

/** 选择 RAW 文件，后台线程解码后经 CAM_ASYNC_EVENT_CAMERA_IMG 显示，失败后写日志 */
void RecipeConfigView::loadRawImageFromDialog(QString &fileName)
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择 RAW 图像"),
        fileName.isEmpty() ? QString() : QFileInfo(fileName).absolutePath(),
        QStringLiteral("RAW 图像 (*.raw);;所有文件 (*.*)"));
    if (path.isEmpty())
        return;

    if (!m_toolbarCamCombo || m_toolbarCamCombo->currentText().isEmpty()) {
        appendLog(QStringLiteral("请先选择相机"), QStringLiteral("WARN"));
        return;
    }

    const QString cameraName = m_toolbarCamCombo->currentText();
    QPointer<RecipeConfigView> self(this);
    QString *const outPath = &fileName;

    std::thread([self, path, cameraName, outPath]() {
        QString error;
        const QImage image = loadRawFileToQImage(path, cameraName, &error);
        QMetaObject::invokeMethod(
            qApp,
            [self, outPath, path, cameraName, image, error]() {
                if (!self)
                    return;
                if (image.isNull()) {
                    self->appendLog(error.isEmpty()
                                        ? QStringLiteral("读取 RAW 图像失败: %1").arg(path)
                                        : error,
                                    QStringLiteral("ERROR"));
                    return;
                }
                if (outPath)
                    *outPath = path;
                MessageCenter::Center::Notify(CAM_ASYNC_EVENT_CAMERA_IMG,
                                              kRecipePreviewWinId,
                                              cameraName,
                                              image);
            },
            Qt::QueuedConnection);
    }).detach();
}

void RecipeConfigView::slot_stepFeedInClicked()
{
    appendLog(QStringLiteral("执行单步调试: 进料"), QStringLiteral("INFO"));
}

void RecipeConfigView::slot_stepFeedOutClicked()
{
    appendLog(QStringLiteral("执行单步调试: 出料"), QStringLiteral("INFO"));
}

void RecipeConfigView::slot_stepReadImageClicked()
{
    resetStepDebugResultOutput();
    loadRawImageFromDialog(m_stepDebugMainRawPath);
}

void RecipeConfigView::slot_stepPureColorDetectClicked()
{
    if (!m_viewModel || !isRecipeListSelectionValid()) {
        appendLog(QStringLiteral("请先选择配方"), QStringLiteral("WARN"));
        return;
    }
    resetStepDebugResultOutput();
    appendLog(QStringLiteral("执行单步调试: 纯色检测"), QStringLiteral("INFO"));
    triggerStepDebugPureColorDetect();
}

void RecipeConfigView::slot_stepStringLightDetectClicked()
{
    if (!m_viewModel || !isRecipeListSelectionValid()) {
        appendLog(QStringLiteral("请先选择配方"), QStringLiteral("WARN"));
        return;
    }
    resetStepDebugResultOutput();
    appendLog(QStringLiteral("执行单步调试: 串灯检测"), QStringLiteral("INFO"));
    triggerStepDebugStringLightDetect();
}

void RecipeConfigView::slot_stepMixColorDetectClicked()
{
    if (!m_viewModel || !isRecipeListSelectionValid()) {
        appendLog(QStringLiteral("请先选择配方"), QStringLiteral("WARN"));
        return;
    }
    appendLog(QStringLiteral("执行单步调试: 混色检测"), QStringLiteral("INFO"));
}

void RecipeConfigView::slot_stepBlackScreenLightDetectClicked()
{
    if (!m_viewModel || !isRecipeListSelectionValid()) {
        appendLog(QStringLiteral("请先选择配方"), QStringLiteral("WARN"));
        return;
    }
    appendLog(QStringLiteral("执行单步调试: 黑屏亮灯"), QStringLiteral("INFO"));
}

void RecipeConfigView::slot_stepFlashDetectClicked()
{
    if (!m_viewModel || !isRecipeListSelectionValid()) {
        appendLog(QStringLiteral("请先选择配方"), QStringLiteral("WARN"));
        return;
    }
    appendLog(QStringLiteral("执行单步调试: Flash检测"), QStringLiteral("INFO"));
}

void RecipeConfigView::appendLog(const QString &text, const QString &level)
{
    if (!m_logEdit || text.isEmpty())
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

void RecipeConfigView::showSaveSuccessTip()
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

bool RecipeConfigView::isRecipeListSelectionValid() const
{
    if (!m_viewModel || !m_recipeList)
        return false;

    const int row = m_recipeList->currentRow();
    if (row < 0 || row >= m_recipeList->count())
        return false;

    const QVector<RecipeListItem> recipes = m_viewModel->recipes();
    if (row >= recipes.size())
        return false;

    const QString selectedId = recipes.at(row).id;
    const QString currentId  = m_viewModel->currentRecipeId();
    return !selectedId.isEmpty() && !currentId.isEmpty() && selectedId == currentId
           && !m_viewModel->document().id.isEmpty();
}

bool RecipeConfigView::promptRecipeName(QString *outName, const QString &defaultName,
                                        const QString &dialogTitle) const
{
    if (!outName)
        return false;

    ui::ThemedPanelDialog dlg(const_cast<RecipeConfigView *>(this)->window());
    dlg.setModal(true);
    dlg.setTitleText(dialogTitle);
    dlg.setMinimumPanelWidth(320);

    auto *body = new QWidget;
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(6);

    auto *nameLabel = new QLabel(QStringLiteral("配方名称:"), body);
    nameLabel->setObjectName(QStringLiteral("themedFormLabel"));

    auto *nameEdit = ui::makeLineEdit(body);
    nameEdit->setText(defaultName);
    nameEdit->setMinimumWidth(260);
    nameEdit->setMaximumWidth(340);
    nameEdit->setMinimumHeight(30);

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();
    auto *cancelBtn =
        ui::makePushButton(QStringLiteral("取消"), ui::PushButtonVariant::Secondary, body);
    auto *confirmBtn =
        ui::makePushButton(QStringLiteral("确定"), ui::PushButtonVariant::Primary, body);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);

    bodyLayout->addWidget(nameLabel);
    bodyLayout->addWidget(nameEdit);
    bodyLayout->addLayout(btnRow);

    dlg.setBodyWidget(body);
    dlg.finalizeLayoutSize();

    QObject::connect(cancelBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(confirmBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(nameEdit, &ui::ThemedLineEdit::returnPressed, &dlg, &QDialog::accept);

    nameEdit->setFocus();
    nameEdit->selectAll();

    if (dlg.exec() != QDialog::Accepted)
        return false;

    *outName = nameEdit->text().trimmed();
    return true;
}

void RecipeConfigView::syncCopyRecipeButtonEnabled()
{
    if (m_copyRecipeBtn)
        m_copyRecipeBtn->setEnabled(isRecipeListSelectionValid());
}

bool RecipeConfigView::confirmRemoveRecipe() const
{
    QMessageBox box(const_cast<RecipeConfigView *>(this));
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("删除配方"));
    box.setText(QStringLiteral("删除后无法恢复，是否继续删除？"));
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    if (QAbstractButton *ok = box.button(QMessageBox::Ok))
        ok->setText(QStringLiteral("确定"));
    if (QAbstractButton *cancel = box.button(QMessageBox::Cancel))
        cancel->setText(QStringLiteral("取消"));
    return box.exec() == QMessageBox::Ok;
}

void RecipeConfigView::syncRemoveRecipeButtonEnabled()
{
    if (m_removeRecipeBtn && m_viewModel)
        m_removeRecipeBtn->setEnabled(m_viewModel->canRemoveRecipe());
}

namespace {

int recipeListRowForId(const QVector<RecipeListItem> &recipes, const QString &recipeId)
{
    if (recipeId.isEmpty())
        return -1;
    for (int i = 0; i < recipes.size(); ++i) {
        if (recipes.at(i).id == recipeId)
            return i;
    }
    return -1;
}

} // namespace

void RecipeConfigView::refreshRecipeList()
{
    if (!m_viewModel || !m_recipeList)
        return;

    const QVector<RecipeListItem> recipes = m_viewModel->recipes();
    const QString currentId               = m_viewModel->currentRecipeId();

    m_recipeList->clear();
    for (int i = 0; i < recipes.size(); ++i) {
        const auto &recipe = recipes.at(i);
        auto *row          = new QWidget(m_recipeList);
        row->setAttribute(Qt::WA_TranslucentBackground, true);
        row->setAutoFillBackground(false);
        auto *hl = new QHBoxLayout(row);
        hl->setContentsMargins(10, 6, 10, 6);
        hl->setSpacing(6);
        auto *nameLbl = new QLabel(recipe.name, row);
        nameLbl->setObjectName(QStringLiteral("recipeListName"));
        nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        hl->addWidget(nameLbl, 1);
        auto *tagLbl = new QLabel(recipeListTagText(recipe.tag), row);
        tagLbl->setObjectName(QStringLiteral("recipeListTag"));
        hl->addWidget(tagLbl, 0, Qt::AlignRight | Qt::AlignVCenter);

        auto *item = new QListWidgetItem(m_recipeList);
        item->setData(Qt::UserRole, recipe.id);
        m_recipeList->addItem(item);
        m_recipeList->setItemWidget(item, row);
        row->adjustSize();
        item->setSizeHint(row->sizeHint().expandedTo(QSize(0, 34)));
    }

    int selectedRow = recipeListRowForId(recipes, currentId);
    QString selectId  = currentId;

    if (selectedRow < 0 && !recipes.isEmpty()) {
        const QString defaultId = RecipeRepository::defaultRecipeId();
        selectedRow             = recipeListRowForId(recipes, defaultId);
        if (selectedRow >= 0)
            selectId = defaultId;
        else {
            selectedRow = 0;
            selectId    = recipes.first().id;
        }
    }

    if (selectedRow >= 0) {
        QSignalBlocker blocker(m_recipeList);
        m_recipeList->setCurrentRow(selectedRow);
        if (!selectId.isEmpty() && selectId != m_viewModel->currentRecipeId())
            m_viewModel->slot_selectRecipe(selectId);
    }

    syncCopyRecipeButtonEnabled();
    syncRemoveRecipeButtonEnabled();
}

} // namespace lighten
