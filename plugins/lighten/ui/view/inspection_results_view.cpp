/**
 * @file inspection_results_view.cpp
 * @brief InspectionResultsView 筛选栏、统计行与分页表格实现
 */

#include "inspection_results_view.h"

#include "inspection_results_view_model.h"
#include "inspection_results_model.h"
#include "lighten_types.h"
#include "repository/filter_utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTime>
#include <QTime>
#include <QDate>
#include <QTimeEdit>
#include <QFrame>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCalendarWidget>
#include <QToolButton>
#include <QMouseEvent>
#include <QShowEvent>
#include <QAbstractSlider>
#include <QHeaderView>
#include <QPointer>
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QWidget>
#include <QListView>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyleOptionComboBox>
#include <QStyle>
#include <QStyleFactory>
#include <QTimer>
#include <QEvent>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPolygon>
#include <functional>

/** 检测结果页本地 UI 辅助类（下拉、日期时间、表格着色等，仅供本编译单元） */
namespace {

/** 下拉列表只画文字，去掉当前项前的勾选/指示（与 recipe_config_view::PlainComboPopupDelegate 一致） */
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

/** 预览工具条式下拉：弹出宽度与框一致、底边距 2px；箭头区与 recipe_config_view::PreviewToolBarComboBox 一致 */
class PreviewToolBarComboBox : public QComboBox
{
public:
    explicit PreviewToolBarComboBox(QWidget *parent = nullptr)
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
        const QRect r = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
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

/** 与 recipe_config_view::RecipeCamTableComboBox 一致：固定高度 34px 对应基本信息「坐标系」下拉 */
class InspectionBasicFormComboBox : public PreviewToolBarComboBox
{
public:
    explicit InspectionBasicFormComboBox(int pixelH, QWidget *parent = nullptr)
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

/** 历史数据分页「每页条数」：列表在输入框上方，列表底边与输入框顶边相距 2px */
class InspectionPagerSizeComboBox final : public InspectionBasicFormComboBox
{
public:
    explicit InspectionPagerSizeComboBox(int pixelH, QWidget *parent = nullptr)
        : InspectionBasicFormComboBox(pixelH, parent)
    {
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
            const QPoint comboTopLeft = mapToGlobal(QPoint(0, 0));
            const int popupH = popup->height();
            const int y = comboTopLeft.y() - 2 - popupH;
            popup->move(comboTopLeft.x(), y);
            if (QLayout *ly = popup->layout())
                ly->setContentsMargins(0, 0, 0, 0);
            popup->setFixedWidth(w);
        });
    }
};

/** 弹窗内同时选择日期（日历）与时间；无边框，对齐锚点控件底部 */
class DateTimePickDialog final : public QDialog
{
public:
    explicit DateTimePickDialog(const QDateTime &initial, QWidget *anchor, QWidget *parent = nullptr)
        : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
        , m_anchor(anchor)
        , m_cal(new QCalendarWidget(this))
        , m_timeEd(new QTimeEdit(this))
        , m_monthLabel(new QLabel(this))
        , m_btnPrev(new QToolButton(this))
        , m_btnNext(new QToolButton(this))
    {
        setModal(true);
        setMinimumWidth(300);
        if (QStyle *fus = QStyleFactory::create(QStringLiteral("Fusion")))
            setStyle(fus);

        m_cal->setGridVisible(true);
        m_cal->setNavigationBarVisible(false);
        m_cal->setCurrentPage(initial.date().year(), initial.date().month());
        m_cal->setSelectedDate(initial.date());

        m_btnPrev->setText(QStringLiteral("◀"));
        m_btnNext->setText(QStringLiteral("▶"));
        m_btnPrev->setCursor(Qt::PointingHandCursor);
        m_btnNext->setCursor(Qt::PointingHandCursor);
        m_btnPrev->setFocusPolicy(Qt::NoFocus);
        m_btnNext->setFocusPolicy(Qt::NoFocus);
        m_btnPrev->setFixedSize(30, 30);
        m_btnNext->setFixedSize(30, 30);
        m_btnPrev->setStyleSheet(QStringLiteral(
            "QToolButton { border: none; background: transparent; color: #e8eef6; font-size: 14px; }"
            "QToolButton:hover { color: #64b5f6; }"));
        m_btnNext->setStyleSheet(m_btnPrev->styleSheet());

        m_monthLabel->setAlignment(Qt::AlignCenter);
        m_monthLabel->setMinimumWidth(200);
        m_monthLabel->setStyleSheet(QStringLiteral(
            "color: #e8eef6; font-size: 14px; font-weight: bold; background: transparent;"
            "padding: 4px 8px;"));

        auto *navRow = new QHBoxLayout();
        navRow->setContentsMargins(0, 0, 0, 0);
        navRow->setSpacing(4);
        navRow->addWidget(m_btnPrev, 0);
        navRow->addWidget(m_monthLabel, 1);
        navRow->addWidget(m_btnNext, 0);

        auto *timeLabel = new QLabel(QStringLiteral("时间"), this);
        timeLabel->setStyleSheet(QStringLiteral("color:#9eb8d4; font-size:12px; background:transparent;"));
        m_timeEd->setDisplayFormat(QStringLiteral("HH:mm:ss"));
        m_timeEd->setTime(initial.time());
        m_timeEd->setWrapping(false);

        auto *vl = new QVBoxLayout(this);
        vl->setContentsMargins(12, 10, 12, 12);
        vl->setSpacing(8);
        vl->addLayout(navRow);
        vl->addWidget(m_cal);
        vl->addWidget(timeLabel);
        vl->addWidget(m_timeEd);

        auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        vl->addWidget(box);
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(m_cal, &QCalendarWidget::currentPageChanged, this, &DateTimePickDialog::syncMonthLabel);
        connect(m_btnPrev, &QToolButton::clicked, this, &DateTimePickDialog::showPrevMonth);
        connect(m_btnNext, &QToolButton::clicked, this, &DateTimePickDialog::showNextMonth);
        syncMonthLabel();

        setObjectName(QStringLiteral("dateTimePickPopup"));
        setStyleSheet(QStringLiteral(
            "#dateTimePickPopup {"
            "  background-color: #1a2233;"
            "  border: 1px solid #3a4a5d;"
            "  border-radius: 6px;"
            "}"
            "QCalendarWidget { background-color: #232d3d; color: #e8eef6; }"
            "QCalendarWidget QAbstractItemView:enabled { color: #e8eef6; selection-background-color: #2a4a7a;"
            "  selection-color: #ffffff; }"
            "QTimeEdit { background-color: #152a3d; color: #ffffff; border: 1px solid #3a4a5d; border-radius: 4px;"
            "  padding: 4px 8px; min-height: 28px; font-size: 12px; }"
            "QDialogButtonBox QPushButton {"
            "  min-height: 26px; padding: 2px 14px;"
            "  background-color: #184c86; color: #e8eef6;"
            "  border: 1px solid #2f82da; border-radius: 4px; font-size: 12px;"
            "}"));
    }

    QDateTime pickedDateTime() const { return QDateTime(m_cal->selectedDate(), m_timeEd->time()); }

protected:
    void showEvent(QShowEvent *event) override
    {
        QDialog::showEvent(event);
        if (!m_anchor)
            return;
        adjustSize();
        const QPoint p = m_anchor->mapToGlobal(QPoint(0, m_anchor->height()));
        move(p.x(), p.y() + 2);
    }

private:
    void syncMonthLabel()
    {
        m_monthLabel->setText(QStringLiteral("%1年%2月")
                                  .arg(m_cal->yearShown())
                                  .arg(m_cal->monthShown(), 2, 10, QLatin1Char('0')));
    }

    void showPrevMonth()
    {
        const QDate cur(m_cal->yearShown(), m_cal->monthShown(), 1);
        const QDate prev = cur.addMonths(-1);
        m_cal->setCurrentPage(prev.year(), prev.month());
    }

    void showNextMonth()
    {
        const QDate cur(m_cal->yearShown(), m_cal->monthShown(), 1);
        const QDate next = cur.addMonths(1);
        m_cal->setCurrentPage(next.year(), next.month());
    }

    QWidget *m_anchor;
    QCalendarWidget *m_cal;
    QTimeEdit *m_timeEd;
    QLabel *m_monthLabel;
    QToolButton *m_btnPrev;
    QToolButton *m_btnNext;
};

/** 单行展示日期时间，点击行或右侧按钮在弹窗中同时选日期与时间 */
class InspectionUnifiedDateTimeField final : public QFrame
{
public:
    explicit InspectionUnifiedDateTimeField(QWidget *parent = nullptr)
        : QFrame(parent)
        , m_edit(new QLineEdit(this))
        , m_btn(new QToolButton(this))
    {
        setObjectName(QStringLiteral("inspectionUnifiedDateTime"));
        setAttribute(Qt::WA_StyledBackground, true);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setFixedSize(228, 34);

        auto *hl = new QHBoxLayout(this);
        hl->setContentsMargins(6, 0, 0, 0);
        hl->setSpacing(0);

        m_edit->setObjectName(QStringLiteral("inspectionUnifiedDateTimeEdit"));
        m_edit->setReadOnly(true);
        m_edit->setFrame(false);
        m_edit->installEventFilter(this);

        m_btn->setObjectName(QStringLiteral("inspectionUnifiedDateTimeBtn"));
        m_btn->setText(QStringLiteral("▾"));
        m_btn->setAutoRaise(true);
        m_btn->setCursor(Qt::PointingHandCursor);
        m_btn->setFocusPolicy(Qt::NoFocus);
        m_btn->setFixedWidth(28);

        hl->addWidget(m_edit, 1);
        hl->addWidget(m_btn, 0);

        connect(m_btn, &QToolButton::clicked, this, &InspectionUnifiedDateTimeField::openPicker);
    }

    void setDateTime(const QDateTime &dt)
    {
        m_dt = dt.isValid() ? dt : QDateTime::currentDateTime();
        m_edit->setText(m_dt.toString(QStringLiteral("yyyy/MM/dd HH:mm:ss")));
    }

    QDateTime dateTime() const { return m_dt; }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o == m_edit && e->type() == QEvent::MouseButtonRelease) {
            const auto *me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton)
                openPicker();
        }
        return QFrame::eventFilter(o, e);
    }

private:
    void openPicker()
    {
        DateTimePickDialog dlg(m_dt, this, this);
        if (dlg.exec() == QDialog::Accepted)
            setDateTime(dlg.pickedDateTime());
    }

    QLineEdit *m_edit;
    QToolButton *m_btn;
    QDateTime m_dt;
};

/** 与 recipe_config_view::previewToolbarComboPopupViewStyleSheet 一致（弹出层为独立窗口） */
static QString inspectionFilterComboPopupViewStyleSheet()
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

static void applyRecipeLikeComboPopup(QComboBox *cb)
{
    if (!cb)
        return;
    if (QAbstractItemView *cv = cb->view()) {
        cv->setStyleSheet(inspectionFilterComboPopupViewStyleSheet()
                          + QStringLiteral(" QAbstractItemView, QAbstractItemView::item { font-size: 12px; }"
                                           " QAbstractItemView::item { min-height: 32px; }"));
        if (auto *lv = qobject_cast<QListView *>(cv))
            lv->setItemDelegate(new PlainComboPopupDelegate(lv));
    }
    QFont tf = cb->font();
    tf.setPixelSize(12);
    tf.setBold(false);
    cb->setFont(tf);
    if (QAbstractItemView *cv = cb->view())
        cv->setFont(tf);
}

/** 与 recipe_config_view 基本信息「坐标系」下拉行高一致 */
static constexpr int kInspectionRecipeBasicComboH = 34;
/** 分页条内控件高度：页码、导航按钮与「每页」下拉及包裹容器一致（略大于纯 28px 文本，避免下拉显矮） */
static constexpr int kInspectionPagerBarCtrlH = 32;

/** 分页条可见页码（1-based）；-1 表示省略号「…」 */
QVector<int> inspectionPagerVisiblePages(int currentZeroBased, int totalPages)
{
    QVector<int> out;
    if (totalPages <= 0)
        return out;

    const int current = currentZeroBased + 1;
    const int delta   = 2;
    QVector<int> range;
    int last = 0;

    for (int i = 1; i <= totalPages; ++i) {
        if (i == 1 || i == totalPages
            || (i >= current - delta && i <= current + delta))
            range.append(i);
    }

    for (const int i : range) {
        if (last > 0) {
            if (i - last == 2)
                out.append(last + 1);
            else if (i - last != 1)
                out.append(-1);
        }
        out.append(i);
        last = i;
    }
    return out;
}

void clearPagerLayout(QHBoxLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }
}

void rebuildPagerPageButtons(QWidget *wrap, int currentZeroBased, int totalPages,
                             const std::function<void(int)> &onPageClicked)
{
    auto *layout = qobject_cast<QHBoxLayout *>(wrap->layout());
    if (!layout)
        return;

    clearPagerLayout(layout);

    for (const int page : inspectionPagerVisiblePages(currentZeroBased, totalPages)) {
        if (page < 0) {
            auto *ell = new QLabel(QStringLiteral("..."), wrap);
            ell->setObjectName(QStringLiteral("inspectionPagerEllipsis"));
            ell->setFixedHeight(kInspectionPagerBarCtrlH);
            ell->setFixedWidth(24);
            ell->setAlignment(Qt::AlignCenter);
            layout->addWidget(ell);
            continue;
        }

        auto *btn = new QPushButton(QString::number(page), wrap);
        btn->setObjectName(QStringLiteral("inspectionPagerPageBtn"));
        btn->setCheckable(true);
        btn->setChecked(page - 1 == currentZeroBased);
        btn->setFixedSize(kInspectionPagerBarCtrlH, kInspectionPagerBarCtrlH);
        QObject::connect(btn, &QPushButton::clicked, btn, [onPageClicked, page]() {
            onPageClicked(page - 1);
        });
        layout->addWidget(btn);
    }
}

constexpr int kColCount            = 27;
constexpr int kColInspectionResult = 2;
constexpr int kColBlackScreenLight = 5;
constexpr int kColDefectRgbFirst   = 6;
constexpr int kColDefectRgbLast    = kColDefectRgbFirst + lighten::kDefectRgbCount - 1;
constexpr int kColFlashResult      = kColDefectRgbLast + 1;
constexpr int kColSpecial          = kColFlashResult + 1;
constexpr int kColTime             = kColSpecial + 1;

// Column widths（检测时间列偏窄，余量由「特殊检测项」Stretch 吸收）
const int kColumnWidths[kColCount] = {
    120, 132, 96, 88, 88, 88,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    100, 200, 150,
};

InspectionUnifiedDateTimeField *asDateTimeField(QWidget *widget)
{
    return static_cast<InspectionUnifiedDateTimeField *>(widget);
}

QColor resultColor(const QString &s)
{
    if (s == QLatin1String("OK"))
        return QColor(QStringLiteral("#52c41a"));
    if (s == QLatin1String("NG"))
        return QColor(QStringLiteral("#ff4d4f"));
    if (s == QLatin1String("RW"))
        return QColor(QStringLiteral("#fadb14"));
    return QColor();
}

QTableWidgetItem *makeCell(const QString &text, const QColor &color = {}, Qt::Alignment hAlign = Qt::AlignHCenter)
{
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignVCenter | hAlign);
    const QColor fg = color.isValid() ? color : QColor(QStringLiteral("#e8eef6"));
    item->setForeground(QBrush(fg));
    if (color.isValid()) {
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    return item;
}

void fillTableRow(QTableWidget *table, int row, const lighten::InspectionHistoryRecord &d)
{
    const QString resultText = lighten::inspectionResultText(d.result);
    int c = 0;
    table->setItem(row, c++, makeCell(d.model, {}, Qt::AlignLeft));
    table->setItem(row, c++, makeCell(d.serialNumber, {}, Qt::AlignLeft));
    table->setItem(row, c++, makeCell(resultText, resultColor(resultText)));
    table->setItem(row, c++, makeCell(QString::number(d.inspectTimes)));
    table->setItem(row, c++, makeCell(QString::number(d.rwCount)));
    table->setItem(row, c++, makeCell(QString::number(d.blackScreenLightCount)));
    for (int i = 0; i < lighten::kDefectRgbCount; ++i)
        table->setItem(row, c++, makeCell(QString::number(d.defectCounts[i])));
    const QString flashText = lighten::flashResultText(d.flashResult);
    table->setItem(row, c++, makeCell(flashText, resultColor(flashText)));
    table->setItem(row, c++, makeCell(d.specialDetect, {}, Qt::AlignLeft));
    table->setItem(row, c++, makeCell(d.inspectedAt));
}

/** Fusion + QSS 时 item 前景色常被样式忽略，对状态列用 delegate 强制按语义色绘制 */
class InspectionHistoryTableDelegate final : public QStyledItemDelegate
{
public:
    explicit InspectionHistoryTableDelegate(QTableWidget *table)
        : QStyledItemDelegate(table)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!index.isValid()
            || (index.column() != kColInspectionResult && index.column() != kColFlashResult)) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const QString t = index.data(Qt::DisplayRole).toString();
        const QColor c = resultColor(t);
        if (!c.isValid()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        opt.palette.setColor(QPalette::Text, c);
        opt.palette.setColor(QPalette::HighlightedText, c);
        QFont f = opt.font;
        f.setBold(true);
        opt.font = f;

        QStyledItemDelegate::paint(painter, opt, index);
    }
};

/** @brief 双层分组合并表头（置于表格上方，与横向滚动同步） */
class InspectionHistoryGroupHeaderWidget final : public QWidget
{
public:
    explicit InspectionHistoryGroupHeaderWidget(QTableWidget *table, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_table(table)
    {
        setFixedHeight(56);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAttribute(Qt::WA_StyledBackground, true);
        if (!m_table)
            return;
        if (QScrollBar *hs = m_table->horizontalScrollBar()) {
            connect(hs, &QAbstractSlider::valueChanged, this, [this](int) { update(); });
        }
        if (QHeaderView *hh = m_table->horizontalHeader()) {
            connect(hh, &QHeaderView::sectionResized, this, [this](int, int, int) { update(); });
            connect(hh, &QHeaderView::geometriesChanged, this, [this] { update(); });
        }
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        update();
    }

    void paintEvent(QPaintEvent *event) override
    {
        QTableWidget *tw = m_table.data();
        if (!tw || tw->columnCount() <= 0) {
            QWidget::paintEvent(event);
            return;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);

        const QRect bounds(0, 0, width(), height());
        if (bounds.width() <= 0 || bounds.height() <= 0) {
            QWidget::paintEvent(event);
            return;
        }

        const int scroll = tw->horizontalScrollBar() ? tw->horizontalScrollBar()->value() : 0;

        auto sectionLeft = [&](int logical) -> int {
            int x = -scroll;
            for (int j = 0; j < logical; ++j)
                x += tw->columnWidth(j);
            return x;
        };
        auto sectionRight = [&](int logical) -> int {
            return sectionLeft(logical) + tw->columnWidth(logical);
        };

        const QColor bgTop(QStringLiteral("#2f3b4d"));
        const QColor bgBot(QStringLiteral("#1e2736"));
        const QColor bgMerged(QStringLiteral("#2a3545"));
        const QColor line(QStringLiteral("#35475c"));
        const QColor textPri(QStringLiteral("#e8eef6"));
        const QColor textSec(QStringLiteral("#9aa4b5"));

        const int topH = height() / 2;
        const int botH = height() - topH;
        const int last = tw->columnCount() - 1;

        painter.setClipRect(event->rect());
        painter.fillRect(bounds, bgBot);

        for (int i = 0; i <= last; ++i) {
            const int x1 = sectionLeft(i);
            const int x2 = sectionRight(i);
            if (x2 <= x1 || x2 < bounds.left() || x1 > bounds.right())
                continue;

            const bool mergedBasic = (i >= 0 && i <= kColBlackScreenLight);
            const bool mergedTail = (i >= kColFlashResult && i <= kColTime);
            const bool merged = mergedBasic || mergedTail;

            if (merged) {
                const QRect fullRect(x1, 0, x2 - x1, height());
                painter.fillRect(fullRect, bgMerged);
                painter.setPen(line);
                painter.drawRect(fullRect.adjusted(0, 0, -1, -1));

                static const QString kMergedBasicLabels[] = {
                    QStringLiteral("产品型号"), QStringLiteral("灯板编号"), QStringLiteral("检测结果"),
                    QStringLiteral("检测次数"), QStringLiteral("返工次数"), QStringLiteral("黑屏亮灯"),
                };
                static const QString kMergedTailLabels[] = {
                    QStringLiteral("Flash结果"), QStringLiteral("特殊检测项"), QStringLiteral("检测时间"),
                };
                const QString txt = mergedBasic ? kMergedBasicLabels[i]
                                                : kMergedTailLabels[i - kColFlashResult];

                painter.setPen(textPri);
                QFont f = painter.font();
                f.setPointSize(11);
                f.setBold(true);
                painter.setFont(f);
                painter.drawText(fullRect, Qt::AlignCenter, txt);
                continue;
            }

            if (i >= kColDefectRgbFirst && i <= kColDefectRgbLast
                && (i - kColDefectRgbFirst) % 3 == 0) {
                const int g = (i - kColDefectRgbFirst) / 3;
                static const QString kGroups[6] = {
                    QStringLiteral("死灯"), QStringLiteral("暗灯"), QStringLiteral("亮灯"),
                    QStringLiteral("串色"), QStringLiteral("串灯"), QStringLiteral("混色"),
                };
                const int gx1 = sectionLeft(i);
                const int gx2 = sectionRight(i + 2);
                const QRect topRect(gx1, 0, gx2 - gx1, topH);
                painter.fillRect(topRect, bgTop);
                painter.setPen(line);
                painter.drawRect(topRect.adjusted(0, 0, -1, -1));
                painter.setPen(textPri);
                QFont f = painter.font();
                f.setPointSize(11);
                f.setBold(true);
                painter.setFont(f);
                painter.drawText(topRect, Qt::AlignCenter, kGroups[g]);
            }

            const QRect botRect(x1, topH, x2 - x1, botH);
            painter.fillRect(botRect, bgBot);
            painter.setPen(line);
            painter.drawRect(botRect.adjusted(0, 0, -1, -1));
            const int rgbIdx = (i - kColDefectRgbFirst) % 3;
            static const char *kRgb[3] = {"R", "G", "B"};
            painter.setPen(textSec);
            QFont f2 = painter.font();
            f2.setPointSize(10);
            painter.setFont(f2);
            painter.drawText(botRect, Qt::AlignCenter, QString::fromLatin1(kRgb[rgbIdx]));

            if (i >= kColDefectRgbFirst && i <= kColDefectRgbLast
                && (i - kColDefectRgbFirst) % 3 == 2) {
                painter.setPen(line);
                painter.drawLine(x2 - 1, 0, x2 - 1, height() - 1);
            }
        }

        if (last >= kColDefectRgbLast) {
            painter.setPen(line);
            const int xL = sectionLeft(kColDefectRgbFirst);
            const int xR = sectionRight(kColDefectRgbLast);
            painter.drawLine(xL, topH, xR, topH);
        }

        painter.setPen(line);
        painter.drawLine(bounds.left(), height() - 1, bounds.right(), height() - 1);
    }

private:
    QPointer<QTableWidget> m_table;
};

} // namespace

/** @brief InspectionResultsView 成员实现（筛选、统计 KPI、分页宽表与数据绑定） */
namespace lighten {

InspectionResultsView::InspectionResultsView(QWidget *parent)
    : PageWidget(parent)
{
    setObjectName(QStringLiteral("lightenInspectionRoot"));
    setPageId(QStringLiteral("lighten.inspection_results"));
    setPageTitle(QStringLiteral("检测结果"));
}

void InspectionResultsView::onLoad()
{
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(QStringLiteral(
        "#lightenInspectionRoot { background-color:#1a2233; }"

        "#lightenInspectionRoot QFrame#filterPanel, #lightenInspectionRoot QFrame#dataPanel {"
        "  background-color:#232d3d; border:1px solid #2d3a4d; border-radius:4px; }"

        "#lightenInspectionRoot QLabel#sectionTitle { color:#e8eef6; font-size:14px; font-weight:bold;"
        "  background-color: transparent; }"

        /* 与 recipe_config_view::applyRecipeTheme 中 QWidget#recipeBasicForm 基本信息段一致 */
        "#lightenInspectionRoot QWidget#recipeBasicForm {"
        "  background-color: transparent;"
        "}"
        "#lightenInspectionRoot QFrame#filterPanel QLabel#recipeBasicFormLabel,"
        "#lightenInspectionRoot QFrame#filterPanel QLabel#recipeBasicResMul,"
        "#lightenInspectionRoot QFrame#filterPanel QLabel#sectionTitle,"
        "#lightenInspectionRoot QWidget#recipeBasicForm QLabel#recipeBasicFormLabel {"
        "  color: #9eb8d4; font-size: 11px; background-color: transparent; background: transparent;"
        "  padding: 0px; margin: 0px; border: none;"
        "}"
        "#lightenInspectionRoot QFrame#filterPanel QLabel#recipeBasicResMul,"
        "#lightenInspectionRoot QWidget#recipeBasicForm QLabel#recipeBasicResMul {"
        "  color: #9eb8d4; font-size: 12px; background-color: transparent; background: transparent;"
        "  border: none;"
        "}"
        "#lightenInspectionRoot QFrame#filterPanel QWidget#inspectionModelSnStrip,"
        "#lightenInspectionRoot QFrame#filterPanel QWidget#inspectionTimeStrip {"
        "  background-color: transparent;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QLineEdit#recipeBasicPathEdit {"
        "  background-color: transparent;"
        "  color: #e8eef6;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QLineEdit#recipeBasicPathEdit::placeholder {"
        "  color: #7a8699;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QWidget#inspectionModelSnStrip QLineEdit {"
        "  background-color: #152a3d;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo {"
        "  background-color: #152a3d; color: #ffffff;"
        "  border: 1px solid #3a4a5d; border-radius: 5px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 6px; padding-right: 6px;"
        "  margin: 0px;"
        "  min-height: 34px; max-height: 34px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QComboBox#recipeCamCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 14px;"
        "  background: transparent;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QFrame#inspectionUnifiedDateTime {"
        "  background-color: #152a3d;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 5px;"
        "  margin: 0px;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QFrame#inspectionUnifiedDateTime:hover {"
        "  border: 1px solid #556c85;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QLineEdit#inspectionUnifiedDateTimeEdit {"
        "  background: transparent;"
        "  border: none;"
        "  color: #ffffff;"
        "  font-size: 12px;"
        "  padding: 0px;"
        "  margin: 0px;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QToolButton#inspectionUnifiedDateTimeBtn {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8eef6;"
        "  font-size: 14px;"
        "  padding: 0px;"
        "  margin: 0px;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QToolButton#inspectionUnifiedDateTimeBtn:hover {"
        "  color: #64b5f6;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QPushButton {"
        "  min-height: 26px; padding: 2px 10px;"
        "  background-color: #184c86; color: #e8eef6;"
        "  border: 1px solid #2f82da; border-radius: 4px;"
        "  font-size: 12px;"
        "}"
        "#lightenInspectionRoot QWidget#recipeBasicForm QPushButton:hover { background-color: #1f5ea2; }"
        "#lightenInspectionRoot QWidget#recipeBasicForm QPushButton:pressed { background-color: #17477b; }"

        "#lightenInspectionRoot QFrame#dataPanel QPushButton#primaryBtn {"
        "  min-height:26px; padding:2px 10px;"
        "  background-color:#184c86; color:#e8eef6;"
        "  border:1px solid #2f82da; border-radius:4px; font-size:12px; }"
        "#lightenInspectionRoot QFrame#dataPanel QPushButton#primaryBtn:hover { background-color:#1f5ea2; }"
        "#lightenInspectionRoot QFrame#dataPanel QPushButton#primaryBtn:pressed { background-color:#17477b; }"

        /* 历史数据表分页条（对齐企业微信截图：深蓝底、亮蓝描边、浅字） */
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar {"
        "  background-color: #1a2634;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QLabel#inspectionPagerTotal,"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QLabel#inspectionPagerEllipsis,"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QWidget#inspectionPagerSizeWrap QLabel {"
        "  color: #e8eef6;"
        "  font-size: 12px;"
        "  background: transparent;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QWidget#inspectionPagerSizeWrap {"
        "  min-height: 32px; max-height: 32px;"
        "  padding: 0px; margin: 0px;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerNavBtn {"
        "  background-color: rgba(24, 144, 255, 0.28);"
        "  color: #ffffff;"
        "  border: 1px solid #40a9ff;"
        "  border-radius: 4px;"
        "  padding: 0px 10px;"
        "  min-height: 32px;"
        "  max-height: 32px;"
        "  font-size: 12px;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerNavBtn:hover {"
        "  background-color: rgba(24, 144, 255, 0.36);"
        "  border-color: #69c0ff;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerNavBtn:pressed {"
        "  background-color: rgba(24, 144, 255, 0.22);"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerPageBtn {"
        "  background-color: transparent;"
        "  color: #ffffff;"
        "  border: 1px solid #2d3a4d;"
        "  border-radius: 4px;"
        "  padding: 0px;"
        "  min-height: 32px;"
        "  max-height: 32px;"
        "  min-width: 32px;"
        "  font-size: 12px;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerPageBtn:hover {"
        "  background-color: rgba(24, 144, 255, 0.12);"
        "  border-color: #40a9ff;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerBar QPushButton#inspectionPagerPageBtn:checked {"
        "  background-color: rgba(24, 144, 255, 0.28);"
        "  color: #ffffff;"
        "  border: 1px solid #40a9ff;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerSizeWrap QComboBox#inspectionPagerSizeCombo {"
        "  background-color: #152a3d;"
        "  color: #ffffff;"
        "  border: 1px solid #2d3a4d;"
        "  border-radius: 4px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 8px; padding-right: 8px;"
        "  margin: 0px;"
        "  min-height: 32px; max-height: 32px;"
        "  combobox-popup: 2;"
        "  font-size: 12px;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerSizeWrap QComboBox#inspectionPagerSizeCombo:hover {"
        "  border-color: #40a9ff;"
        "}"
        "#lightenInspectionRoot QFrame#dataPanel QWidget#inspectionPagerSizeWrap QComboBox#inspectionPagerSizeCombo::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  border: none; width: 18px;"
        "  background: transparent;"
        "}"

        "#lightenInspectionRoot QFrame#dataPanel QTableWidget {"
        "  background-color:#1e2736; alternate-background-color:#222c3c;"
        "  gridline-color:#35475c; border:1px solid #2d3a4d; border-radius:0px;"
        "  font-size: 12px; outline: none; }"
        "#lightenInspectionRoot QFrame#dataPanel QTableWidget::item {"
        "  padding: 5px 8px; border: none; }"
        "#lightenInspectionRoot QFrame#dataPanel QTableWidget::item:selected { background-color:#2a4a7a; }"
        "#lightenInspectionRoot QScrollBar:vertical, #lightenInspectionRoot QScrollBar:horizontal {"
        "  background:#1a2233; border:none; }"
        "#lightenInspectionRoot QScrollBar::handle { background:#3a4a5d; min-height:24px; min-width:24px; border-radius:3px; }"
        "#lightenInspectionRoot QFrame#kpiCard {"
        "  background-color:#232d3d; border:1px solid #2d3a4d; border-radius:4px; }"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(10);

    buildFilterCard(root);
    buildStatsRow(root);
    buildDataTable(root);

    if (!m_viewModel)
        m_viewModel = new InspectionResultsViewModel(this);
    setupBindings();
}

void InspectionResultsView::buildFilterCard(QVBoxLayout *root)
{
    auto *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("filterPanel"));
    auto *outer = new QVBoxLayout(panel);
    outer->setContentsMargins(12, 10, 12, 10);
    outer->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("历史筛选"), panel);
    title->setObjectName(QStringLiteral("sectionTitle"));
    title->setAutoFillBackground(false);
    title->setAttribute(Qt::WA_TranslucentBackground, true);
    {
        QPalette pal = title->palette();
        pal.setColor(QPalette::Window, Qt::transparent);
        pal.setColor(QPalette::Base, Qt::transparent);
        title->setPalette(pal);
    }
    outer->addWidget(title);

    auto *basicForm = new QWidget(panel);
    basicForm->setObjectName(QStringLiteral("recipeBasicForm"));
    basicForm->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QHBoxLayout(basicForm);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    const auto transparentSurface = [](QWidget *w) {
        w->setAutoFillBackground(false);
        w->setAttribute(Qt::WA_TranslucentBackground, true);
    };
    const auto makeFilterLabel = [&](QWidget *parent, const QString &text,
                                     const QString &objectName) {
        auto *lb = new QLabel(text, parent);
        lb->setObjectName(objectName);
        transparentSurface(lb);
        QPalette pal = lb->palette();
        pal.setColor(QPalette::Window, Qt::transparent);
        pal.setColor(QPalette::Base, Qt::transparent);
        lb->setPalette(pal);
        return lb;
    };

    auto addField = [&](const QString &labelText, QWidget *editor, int labelToEditorGap = 6) {
        auto *cell = new QWidget(basicForm);
        transparentSurface(cell);
        cell->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        auto *hl = new QHBoxLayout(cell);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(labelToEditorGap);
        hl->addWidget(makeFilterLabel(cell, labelText, QStringLiteral("recipeBasicFormLabel")));
        hl->addWidget(editor);
        layout->addWidget(cell);
    };

    /* 产品型号 + 灯板编号：同一行容器、组间更紧，标签与控件 4px */
    auto *modelSnStrip = new QWidget(basicForm);
    modelSnStrip->setObjectName(QStringLiteral("inspectionModelSnStrip"));
    transparentSurface(modelSnStrip);
    modelSnStrip->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    auto *modelSnLay = new QHBoxLayout(modelSnStrip);
    modelSnLay->setContentsMargins(0, 0, 0, 0);
    modelSnLay->setSpacing(8);

    static constexpr int kModelSnControlW = 168;

    auto addCompactCell = [&](const QString &labelText, QWidget *editor) {
        auto *cell = new QWidget(modelSnStrip);
        transparentSurface(cell);
        cell->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        auto *hl = new QHBoxLayout(cell);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(4);
        hl->addWidget(makeFilterLabel(cell, labelText, QStringLiteral("recipeBasicFormLabel")));
        hl->addWidget(editor);
        modelSnLay->addWidget(cell);
    };

    auto *modelSel = new InspectionBasicFormComboBox(kInspectionRecipeBasicComboH, modelSnStrip);
    modelSel->setObjectName(QStringLiteral("recipeCamCombo"));
    modelSel->addItems({QStringLiteral("全部"), QStringLiteral("A01"), QStringLiteral("A02"), QStringLiteral("B01"),
                        QStringLiteral("B03"), QStringLiteral("C05"), QStringLiteral("D07")});
    modelSel->setCurrentIndex(0);
    modelSel->setFixedWidth(kModelSnControlW);
    addCompactCell(QStringLiteral("产品型号"), modelSel);

    auto *snEdit = new QLineEdit(modelSnStrip);
    snEdit->setObjectName(QStringLiteral("recipeBasicPathEdit"));
    snEdit->setPlaceholderText(QStringLiteral("请输入"));
    snEdit->setFixedWidth(kModelSnControlW);
    addCompactCell(QStringLiteral("灯板编号"), snEdit);

    layout->addWidget(modelSnStrip);

    auto *resultSel = new InspectionBasicFormComboBox(kInspectionRecipeBasicComboH, basicForm);
    resultSel->setObjectName(QStringLiteral("recipeCamCombo"));
    resultSel->addItems({QStringLiteral("全部"), QStringLiteral("OK"), QStringLiteral("NG"), QStringLiteral("RW")});
    resultSel->setFixedWidth(120);
    addField(QStringLiteral("检测结果"), resultSel);

    auto *defectSel = new InspectionBasicFormComboBox(kInspectionRecipeBasicComboH, basicForm);
    defectSel->setObjectName(QStringLiteral("recipeCamCombo"));
    defectSel->addItems(lighten::repo_filter::defectTypeFilterOptions());
    defectSel->setFixedWidth(128);
    addField(QStringLiteral("缺陷类型"), defectSel);

    auto *flashSel = new InspectionBasicFormComboBox(kInspectionRecipeBasicComboH, basicForm);
    flashSel->setObjectName(QStringLiteral("recipeCamCombo"));
    flashSel->addItems(lighten::repo_filter::flashResultFilterOptions());
    flashSel->setFixedWidth(120);
    addField(QStringLiteral("Flash结果"), flashSel);

    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime fromInit(now.date(), QTime(0, 0));

    auto *fromDt = new InspectionUnifiedDateTimeField(basicForm);
    fromDt->setDateTime(fromInit);
    auto *toDt = new InspectionUnifiedDateTimeField(basicForm);
    toDt->setDateTime(now);

    auto *timeWrap = new QWidget(basicForm);
    timeWrap->setObjectName(QStringLiteral("inspectionTimeStrip"));
    transparentSurface(timeWrap);
    auto *timeLay = new QHBoxLayout(timeWrap);
    timeLay->setContentsMargins(0, 0, 0, 0);
    timeLay->setSpacing(6);
    timeLay->addWidget(
        makeFilterLabel(timeWrap, QStringLiteral("检测时间"), QStringLiteral("recipeBasicFormLabel")));
    timeLay->addWidget(fromDt);
    timeLay->addWidget(
        makeFilterLabel(timeWrap, QStringLiteral("—"), QStringLiteral("recipeBasicResMul")));
    timeLay->addWidget(toDt);
    layout->addWidget(timeWrap);

    layout->addSpacing(6);
    auto *btnQuery = new QPushButton(QStringLiteral("查询"), basicForm);
    auto *btnReset = new QPushButton(QStringLiteral("重置"), basicForm);
    auto *btnExport = new QPushButton(QStringLiteral("导出表格"), basicForm);
    layout->addWidget(btnQuery);
    layout->addWidget(btnReset);
    layout->addWidget(btnExport);

    layout->addStretch(1);

    applyRecipeLikeComboPopup(modelSel);
    applyRecipeLikeComboPopup(resultSel);
    applyRecipeLikeComboPopup(defectSel);
    applyRecipeLikeComboPopup(flashSel);

    m_modelSel   = modelSel;
    m_snEdit     = snEdit;
    m_resultSel  = resultSel;
    m_defectSel  = defectSel;
    m_flashSel   = flashSel;
    m_fromDt     = fromDt;
    m_toDt       = toDt;
    m_btnQuery   = btnQuery;
    m_btnReset   = btnReset;
    m_btnExport  = btnExport;

    outer->addWidget(basicForm);
    root->addWidget(panel);
}

void InspectionResultsView::buildStatsRow(QVBoxLayout *root)
{
    auto *bar = new QWidget(this);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    struct StatEntry {
        QString label;
        QString value;
    };
    const QVector<StatEntry> stats = {
        {QStringLiteral("直通率"), QStringLiteral("0.00%")},
        {QStringLiteral("平均坏点"), QStringLiteral("0.0")},
        {QStringLiteral("OK数量"), QStringLiteral("0")},
        {QStringLiteral("NG数量"), QStringLiteral("0")},
        {QStringLiteral("RW数量"), QStringLiteral("0")},
    };

    for (const auto &s : stats) {
        auto *card = new QFrame(bar);
        card->setObjectName(QStringLiteral("kpiCard"));
        card->setFrameShape(QFrame::NoFrame);
        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(14, 10, 14, 10);
        vl->setSpacing(4);

        auto *lbl = new QLabel(s.label, card);
        lbl->setStyleSheet(QStringLiteral("color:#9aa4b5; font-size:12px;"));
        auto *val = new QLabel(s.value, card);
        QFont f = val->font();
        f.setPointSize(15);
        f.setBold(true);
        val->setFont(f);
        val->setStyleSheet(QStringLiteral("color:#ffffff;"));

        vl->addWidget(lbl);
        vl->addWidget(val);
        layout->addWidget(card, 1);
        m_stats.append(val);
    }
    root->addWidget(bar);
}

void InspectionResultsView::buildDataTable(QVBoxLayout *root)
{
    auto *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("dataPanel"));
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("历史数据"), panel);
    title->setObjectName(QStringLiteral("sectionTitle"));
    layout->addWidget(title);

    auto *tableWrap = new QWidget(panel);
    auto *twLayout = new QVBoxLayout(tableWrap);
    twLayout->setContentsMargins(0, 0, 0, 0);
    twLayout->setSpacing(0);

    m_table = new QTableWidget(tableWrap);
    m_table->setObjectName(QStringLiteral("inspectionHistoryTable"));
    m_table->setColumnCount(kColCount);
    m_table->setRowCount(0);

    m_table->horizontalHeader()->setVisible(false);
    m_table->horizontalHeader()->setMinimumSectionSize(36);
    for (int i = 0; i < kColCount; ++i) {
        const QHeaderView::ResizeMode mode = (i == kColSpecial) ? QHeaderView::Stretch : QHeaderView::Interactive;
        m_table->horizontalHeader()->setSectionResizeMode(i, mode);
        m_table->setColumnWidth(i, kColumnWidths[i]);
    }

    m_table->setItemDelegate(new InspectionHistoryTableDelegate(m_table));

    auto *groupHeader = new InspectionHistoryGroupHeaderWidget(m_table, tableWrap);
    twLayout->addWidget(groupHeader);
    twLayout->addWidget(m_table, 1);

    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(36);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->setWordWrap(false);
    m_table->setTextElideMode(Qt::ElideNone);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(tableWrap, 1);

    auto *pgBar = new QWidget(panel);
    pgBar->setObjectName(QStringLiteral("inspectionPagerBar"));
    pgBar->setAttribute(Qt::WA_StyledBackground, true);
    auto *pgLayout = new QHBoxLayout(pgBar);
    pgLayout->setContentsMargins(8, 10, 8, 10);
    pgLayout->setSpacing(10);

    pgLayout->addStretch(1);

    m_totalLbl = new QLabel(QStringLiteral("共 0 条"), pgBar);
    m_totalLbl->setObjectName(QStringLiteral("inspectionPagerTotal"));
    pgLayout->addWidget(m_totalLbl);

    m_prevBtn = new QPushButton(QStringLiteral("上一页"), pgBar);
    m_prevBtn->setObjectName(QStringLiteral("inspectionPagerNavBtn"));
    m_prevBtn->setFixedHeight(kInspectionPagerBarCtrlH);
    pgLayout->addWidget(m_prevBtn);

    m_pageNumsWrap = new QWidget(pgBar);
    m_pageNumsWrap->setObjectName(QStringLiteral("inspectionPagerNumsWrap"));
    auto *numsLay = new QHBoxLayout(m_pageNumsWrap);
    numsLay->setContentsMargins(0, 0, 0, 0);
    numsLay->setSpacing(4);
    pgLayout->addWidget(m_pageNumsWrap);

    m_nextBtn = new QPushButton(QStringLiteral("下一页"), pgBar);
    m_nextBtn->setObjectName(QStringLiteral("inspectionPagerNavBtn"));
    m_nextBtn->setFixedHeight(kInspectionPagerBarCtrlH);
    pgLayout->addWidget(m_nextBtn);

    pgLayout->addSpacing(6);

    auto *pageCtrlWrap = new QWidget(pgBar);
    pageCtrlWrap->setObjectName(QStringLiteral("inspectionPagerSizeWrap"));
    pageCtrlWrap->setAttribute(Qt::WA_StyledBackground, true);
    pageCtrlWrap->setFixedHeight(kInspectionPagerBarCtrlH);
    pageCtrlWrap->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    auto *pageCtrlLay = new QHBoxLayout(pageCtrlWrap);
    pageCtrlLay->setContentsMargins(0, 0, 0, 0);
    pageCtrlLay->setSpacing(2);
    pageCtrlLay->setAlignment(Qt::AlignVCenter);

    auto *perLbl = new QLabel(QStringLiteral("每页"), pageCtrlWrap);
    perLbl->setFixedHeight(kInspectionPagerBarCtrlH);
    perLbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    perLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    pageCtrlLay->addWidget(perLbl);

    m_pageSizeSel = new InspectionPagerSizeComboBox(kInspectionPagerBarCtrlH, pageCtrlWrap);
    m_pageSizeSel->addItems({QStringLiteral("10"), QStringLiteral("20"), QStringLiteral("50")});
    m_pageSizeSel->setCurrentIndex(1);
    m_pageSizeSel->setObjectName(QStringLiteral("inspectionPagerSizeCombo"));
    m_pageSizeSel->setFixedWidth(60);
    m_pageSizeSel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pageCtrlLay->addWidget(m_pageSizeSel);
    applyRecipeLikeComboPopup(m_pageSizeSel);

    auto *suffix = new QLabel(QStringLiteral("条"), pageCtrlWrap);
    suffix->setFixedHeight(kInspectionPagerBarCtrlH);
    suffix->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    suffix->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    pageCtrlLay->addWidget(suffix);

    pgLayout->addWidget(pageCtrlWrap);

    layout->addWidget(pgBar);
    root->addWidget(panel, 1);
}

InspectionFilter InspectionResultsView::readFilterFromUi() const
{
    InspectionFilter f = InspectionFilter::defaultToday();
    if (m_modelSel)
        f.productModel = m_modelSel->currentText();
    if (m_snEdit)
        f.boardSerial = m_snEdit->text();
    if (m_resultSel)
        f.resultFilter = m_resultSel->currentText();
    if (m_defectSel)
        f.defectType = m_defectSel->currentText();
    if (m_flashSel)
        f.flashResultFilter = m_flashSel->currentText();
    if (m_fromDt)
        f.timeFrom = asDateTimeField(m_fromDt)->dateTime();
    if (m_toDt)
        f.timeTo = asDateTimeField(m_toDt)->dateTime();
    return f;
}

void InspectionResultsView::applyFilterToUi(const InspectionFilter &filter)
{
    if (m_modelSel) {
        const int idx = m_modelSel->findText(filter.productModel);
        m_modelSel->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (m_snEdit)
        m_snEdit->setText(filter.boardSerial);
    if (m_resultSel) {
        const int idx = m_resultSel->findText(filter.resultFilter);
        m_resultSel->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (m_defectSel) {
        const int idx = m_defectSel->findText(filter.defectType);
        m_defectSel->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (m_flashSel) {
        const int idx = m_flashSel->findText(filter.flashResultFilter);
        m_flashSel->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (m_fromDt)
        asDateTimeField(m_fromDt)->setDateTime(filter.timeFrom);
    if (m_toDt)
        asDateTimeField(m_toDt)->setDateTime(filter.timeTo);
}

void InspectionResultsView::refreshTable()
{
    if (!m_table || !m_viewModel)
        return;

    const auto rows = m_viewModel->page().rows;
    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r)
        fillTableRow(m_table, r, rows[r]);
}

void InspectionResultsView::refreshStats()
{
    if (!m_viewModel || m_stats.size() < 5)
        return;

    const InspectionStats s = m_viewModel->stats();
    m_stats[0]->setText(QStringLiteral("%1%").arg(s.passRatePercent, 0, 'f', 2));
    m_stats[1]->setText(QString::number(s.avgDefectCount, 'f', 1));
    m_stats[2]->setText(QString::number(s.okCount));
    m_stats[3]->setText(QString::number(s.ngCount));
    m_stats[4]->setText(QString::number(s.rwCount));
}

void InspectionResultsView::refreshPager()
{
    if (!m_viewModel)
        return;

    const InspectionPage p = m_viewModel->page();
    if (m_totalLbl)
        m_totalLbl->setText(QStringLiteral("共 %1 条").arg(p.totalCount));
    if (m_pageNumsWrap && m_viewModel) {
        rebuildPagerPageButtons(m_pageNumsWrap, p.pageIndex, p.totalPages,
                                [this](int pageIndex) { m_viewModel->slot_goToPage(pageIndex); });
    }
    if (m_prevBtn)
        m_prevBtn->setEnabled(m_viewModel->pageIndex() > 0);
    if (m_nextBtn)
        m_nextBtn->setEnabled(m_viewModel->pageIndex() + 1 < p.totalPages);
}

void InspectionResultsView::showExportSuccessTip()
{
    if (m_exportSuccessTipDlg)
        m_exportSuccessTipDlg->close();

    auto *tipDlg = new QDialog(this, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    tipDlg->setAttribute(Qt::WA_DeleteOnClose);
    tipDlg->setFixedSize(280, 60);
    m_exportSuccessTipDlg = tipDlg;

    auto *opacityFx = new QGraphicsOpacityEffect(tipDlg);
    opacityFx->setOpacity(1.0);
    tipDlg->setGraphicsEffect(opacityFx);

    auto *tipLayout = new QVBoxLayout(tipDlg);
    tipLayout->setContentsMargins(0, 0, 0, 0);
    auto *tipLabel = new QLabel(QStringLiteral("导出成功"), tipDlg);
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
    connect(tipDlg, &QObject::destroyed, this, [this]() { m_exportSuccessTipDlg = nullptr; });
    QTimer::singleShot(1000, tipDlg, [fadeAnim]() { fadeAnim->start(); });

    tipDlg->show();
}

void InspectionResultsView::promptExportTable()
{
    if (!m_viewModel)
        return;

    m_viewModel->setFilter(readFilterFromUi());
    const QString defaultName = QStringLiteral("result_%1.csv")
                                    .arg(QDateTime::currentDateTime().toString(
                                        QStringLiteral("yyyyMMddHHmmss")));
    QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("导出表格"),
        defaultName,
        QStringLiteral("CSV 文件 (*.csv);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    if (!path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive))
        path += QStringLiteral(".csv");
    m_viewModel->slot_exportTable(path);
}

void InspectionResultsView::setupBindings()
{
    if (!m_viewModel)
        return;

    connect(m_btnQuery, &QPushButton::clicked, this, [this]() {
        m_viewModel->setFilter(readFilterFromUi());
        m_viewModel->slot_query();
    });
    connect(m_btnReset, &QPushButton::clicked, m_viewModel, &InspectionResultsViewModel::slot_resetFilter);
    connect(m_btnExport, &QPushButton::clicked, this, &InspectionResultsView::promptExportTable);
    connect(m_viewModel, &InspectionResultsViewModel::sig_exportRequested, this,
            &InspectionResultsView::promptExportTable);
    connect(m_viewModel, &InspectionResultsViewModel::sig_exportSucceeded, this,
            &InspectionResultsView::showExportSuccessTip);

    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        m_viewModel->slot_goToPage(m_viewModel->pageIndex() - 1);
    });
    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        m_viewModel->slot_goToPage(m_viewModel->pageIndex() + 1);
    });

    connect(m_pageSizeSel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (!m_viewModel || !m_pageSizeSel)
            return;
        m_viewModel->setPageSize(m_pageSizeSel->currentText().toInt());
        m_viewModel->slot_query();
    });

    connect(m_viewModel, &InspectionResultsViewModel::sig_pageChanged, this, [this]() {
        refreshTable();
        refreshPager();
    });
    connect(m_viewModel, &InspectionResultsViewModel::sig_statsChanged, this, &InspectionResultsView::refreshStats);
    connect(m_viewModel, &InspectionResultsViewModel::sig_filterChanged, this, [this]() {
        applyFilterToUi(m_viewModel->filter());
    });

    applyFilterToUi(m_viewModel->filter());
    m_viewModel->slot_query();
}

} // namespace lighten
