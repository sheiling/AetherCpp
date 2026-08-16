/**
 * @file themed_combo_box.cpp
 * @brief ThemedComboBox 实现：popup 委托行高、list QSS、minimumSizeHint 约束
 */

#include "controls/combo/themed_combo_box.h"

#include "controls/combo/combo_box_style.h"
#include "controls/spinbox/spinbox_paint_utils.h"

#include "theme/theme_refresh.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QListView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionComboBox>
#include <QStyleOptionViewItem>

namespace ui {

namespace {

constexpr int kComboPopupItemRowHeight = 36;

class ComboPopupItemDelegate final : public QStyledItemDelegate
{
public:
    explicit ComboPopupItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.setHeight(kComboPopupItemRowHeight);
        return sz;
    }
};

void setupComboPopupList(QAbstractItemView *list, QObject *owner)
{
    if (!list)
        return;

    list->setItemDelegate(new ComboPopupItemDelegate(owner));
    if (auto *lv = qobject_cast<QListView *>(list))
        lv->setUniformItemSizes(true);
}

} // namespace

ThemedComboBox::ThemedComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setMinimumContentsLength(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(0);
    bindTheme();
    refreshStyle();

    if (QAbstractItemView *list = view())
        setupComboPopupList(list, this);
}

QSize ThemedComboBox::minimumSizeHint() const
{
    QSize hint = QComboBox::minimumSizeHint();
    // 固定宽度时勿按最长下拉项撑开，否则闭合态文字绘制区域错位（仅见左侧细条）
    if (width() > 0)
        hint.setWidth(width());
    const int maxW = maximumWidth();
    if (maxW > 0 && maxW < QWIDGETSIZE_MAX)
        hint.setWidth(qMin(hint.width(), maxW));
    if (minimumHeight() > 0)
        hint.setHeight(qMax(hint.height(), minimumHeight()));
    return hint;
}

QSize ThemedComboBox::sizeHint() const
{
    QSize hint = QComboBox::sizeHint();
    if (width() > 0)
        hint.setWidth(width());
    const int maxW = maximumWidth();
    if (maxW > 0 && maxW < QWIDGETSIZE_MAX)
        hint.setWidth(qMin(hint.width(), maxW));
    if (minimumHeight() > 0)
        hint.setHeight(qMax(hint.height(), minimumHeight()));
    return hint;
}

void ThemedComboBox::showPopup()
{
    QComboBox::showPopup();

    QAbstractItemView *list = view();
    if (!list)
        return;

    list->setStyleSheet(buildComboBoxListViewStylesheet());

    QWidget *popup = list->window();
    if (!popup)
        return;

    list->setMinimumWidth(0);
    const int listContentW = list->sizeHintForColumn(0);
    const int popupW       = qMax(width(), listContentW + 28);
    popup->setMinimumWidth(popupW);
    popup->resize(popupW, popup->height());
}

void ThemedComboBox::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedComboBox::refreshStyle()
{
    setStyleSheet(buildComboBoxStylesheet());
    if (QAbstractItemView *list = view())
        list->setStyleSheet(buildComboBoxListViewStylesheet());
}

void ThemedComboBox::paintEvent(QPaintEvent *event)
{
    QComboBox::paintEvent(event);

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    const QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                    QStyle::SC_ComboBoxArrow, this);
    if (!arrowRect.isValid())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const bool popupOpen = (opt.state & QStyle::State_On) != 0;
    paintTriangleArrow(p, arrowRect,
                       popupOpen ? TriangleDirection::Up : TriangleDirection::Down,
                       isEnabled());
}

void ThemedComboBox::changeEvent(QEvent *event)
{
    QComboBox::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
}

ThemedComboBox *makeComboBox(QWidget *parent)
{
    return new ThemedComboBox(parent);
}

} // namespace ui
