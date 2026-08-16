/**
 * @file themed_list_widget.cpp
 * @brief ThemedListWidget 悬停追踪、行宽同步与主题 QSS 刷新
 */

#include "list/themed_list_widget.h"

#include "list/list_widget_style.h"

#include "theme/theme_refresh.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QFrame>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QResizeEvent>

namespace ui {

namespace {

/** 配方行内可着色区域，优先 recipeListRowContent 子控件 */
QWidget *rowContentWidget(QWidget *rowWidget)
{
    if (!rowWidget)
        return nullptr;
    return rowWidget->findChild<QWidget *>(QStringLiteral("recipeListRowContent"));
}

} // namespace

ThemedListWidget::ThemedListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setObjectName(QStringLiteral("themedListWidget"));
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setUniformItemSizes(true);
    setMouseTracking(true);
    setSpacing(2);
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);

    connect(this, &QListWidget::itemEntered, this, [this](QListWidgetItem *item) {
        if (m_hoveredItem == item)
            return;
        auto *prev    = m_hoveredItem;
        m_hoveredItem = item;
        refreshRowItem(prev);
        refreshRowItem(m_hoveredItem);
    });

    connect(this, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                refreshRowItem(previous);
                refreshRowItem(current);
            });

    connect(model(), &QAbstractItemModel::rowsInserted, this, [this]() { refreshAllRows(); });

    connect(model(), &QAbstractItemModel::rowsAboutToBeRemoved, this,
            [this](const QModelIndex &parent, int first, int last) {
                Q_UNUSED(parent);
                if (!m_hoveredItem)
                    return;
                for (int row = first; row <= last; ++row) {
                    if (item(row) == m_hoveredItem) {
                        m_hoveredItem = nullptr;
                        break;
                    }
                }
            });

    bindTheme();
    refreshStyle();
}

void ThemedListWidget::setItemWidgetRows(bool enabled)
{
    if (m_itemWidgetRows == enabled)
        return;
    m_itemWidgetRows = enabled;
    setSpacing(enabled ? 0 : 2);
    refreshStyle();
    refreshAllRows();
}

void ThemedListWidget::applyRowWidgetStyle(QWidget *rowWidget, ListRowVisualState state)
{
    if (!rowWidget)
        return;

    QWidget *content = rowContentWidget(rowWidget);
    if (!content)
        content = rowWidget;

    content->setAttribute(Qt::WA_StyledBackground, true);
    content->setAutoFillBackground(true);
    content->setStyleSheet(buildListRowContentStylesheet(state));
}

ListRowVisualState ThemedListWidget::rowVisualState(QListWidgetItem *item) const
{
    if (!item)
        return ListRowVisualState::Normal;
    if (item == currentItem())
        return ListRowVisualState::Selected;
    if (item == m_hoveredItem)
        return ListRowVisualState::Hovered;
    return ListRowVisualState::Normal;
}

bool ThemedListWidget::containsItem(QListWidgetItem *item) const
{
    if (!item)
        return false;
    for (int i = 0; i < count(); ++i) {
        if (this->item(i) == item)
            return true;
    }
    return false;
}

void ThemedListWidget::refreshRowItem(QListWidgetItem *item)
{
    if (!containsItem(item))
        return;

    if (QWidget *rowWidget = itemWidget(item)) {
        // setItemWidget 不会替代 QListWidgetItem 的 delegate 绘制；须清掉 item 底色并关闭 ::item 伪类
        item->setBackground(Qt::NoBrush);
        applyRowWidgetStyle(rowWidget, rowVisualState(item));
        return;
    }

    const ListRowVisualState state = rowVisualState(item);
    item->setBackground(listPlainItemBackground(state));
    item->setForeground(listPlainItemForeground(state));
    if (viewport())
        viewport()->update(visualItemRect(item));
}

void ThemedListWidget::clear()
{
    m_hoveredItem = nullptr;
    QListWidget::clear();
}

void ThemedListWidget::refreshAllRows()
{
    for (int i = 0; i < count(); ++i)
        refreshRowItem(item(i));
}

void ThemedListWidget::syncRowWidths()
{
    const int viewW = viewport()->width();
    if (viewW <= 0)
        return;

    for (int i = 0; i < count(); ++i) {
        auto *item = this->item(i);
        if (!item)
            continue;
        auto *rowWidget = itemWidget(item);
        if (!rowWidget)
            continue;

        const QRect itemRect = visualItemRect(item);
        const int     w      = qMax(viewW, itemRect.width());
        const int     h      = qMax(35, rowWidget->sizeHint().height());
        rowWidget->setFixedSize(w, h);
        item->setSizeHint(QSize(w, h));
    }
}

void ThemedListWidget::updateHoveredItemAt(const QPoint &viewportPos)
{
    QListWidgetItem *item = itemAt(viewportPos);
    if (m_hoveredItem == item)
        return;

    auto *prev    = m_hoveredItem;
    m_hoveredItem = item;
    refreshRowItem(prev);
    refreshRowItem(m_hoveredItem);
}

void ThemedListWidget::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        refreshStyle();
        refreshAllRows();
    });
}

void ThemedListWidget::refreshStyle()
{
    setStyleSheet(buildListWidgetStylesheet({}, m_itemWidgetRows));
}

void ThemedListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    syncRowWidths();
    refreshAllRows();
}

bool ThemedListWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != viewport())
        return QListWidget::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseMove:
        updateHoveredItemAt(static_cast<QMouseEvent *>(event)->pos());
        break;
    case QEvent::Leave:
        if (m_hoveredItem) {
            auto *prev    = m_hoveredItem;
            m_hoveredItem = nullptr;
            refreshRowItem(prev);
        }
        break;
    default:
        break;
    }

    return QListWidget::eventFilter(watched, event);
}

void ThemedListWidget::changeEvent(QEvent *event)
{
    QListWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        refreshStyle();
}

ThemedListWidget *makeListWidget(QWidget *parent)
{
    return new ThemedListWidget(parent);
}

} // namespace ui
