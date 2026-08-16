#pragma once

/**
 * @file themed_list_widget.h
 * @brief 主题 QListWidget：动态 QSS，颜色跟随 ThemeTokens
 */

#include "list/list_widget_style.h"

#include <QListWidget>
#include "component_global.h"

class QEvent;
class QListWidgetItem;
class QMouseEvent;
class QObject;
class QResizeEvent;
class QWidget;

namespace ui {

/**
 * @brief 主题化 QListWidget，行背景由 itemWidget 自绘而非 ::item 伪类
 *
 * 悬停态通过 viewport eventFilter + itemEntered 维护；
 * clear / 行删除时会清空 m_hoveredItem，避免列表重建后 Leave 等事件访问悬空 item。
 * 选中/悬停/常态样式见 list_widget_style.h。
 */
class UI_COMPONENT_EXPORT ThemedListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ThemedListWidget(QWidget *parent = nullptr);

    /**
     * setItemWidget 行模式：true 时关闭 ::item 的 hover/selected 底色，
     * 仅由 itemWidget + applyRowWidgetStyle 着色（配方列表等自定义行）。
     * 纯文本 item 列表保持默认 false。
     */
    void setItemWidgetRows(bool enabled);
    bool itemWidgetRows() const { return m_itemWidgetRows; }

    /** 对 itemWidget 行内内容区应用行态 QSS */
    static void applyRowWidgetStyle(QWidget *rowWidget, ListRowVisualState state);

    /** 清空列表并重置悬停追踪（配方页 setRecipes 等全量刷新路径） */
    void clear();

    /** 遍历所有 item 刷新行背景 */
    void refreshAllRows();
    /** 将各行 itemWidget 宽度对齐 viewport，避免横向滚动 */
    void syncRowWidths();

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void refreshStyle();

private:
    void bindTheme();
    /** item 是否仍为本列表当前持有的行（不访问 item 成员，仅指针比对） */
    bool containsItem(QListWidgetItem *item) const;
    ListRowVisualState rowVisualState(QListWidgetItem *item) const;
    void refreshRowItem(QListWidgetItem *item);
    void updateHoveredItemAt(const QPoint &viewportPos);

    QListWidgetItem *m_hoveredItem   = nullptr;
    bool             m_itemWidgetRows = false;
};

/** 工厂：创建并配置默认 objectName 的 ThemedListWidget */
UI_COMPONENT_EXPORT ThemedListWidget *makeListWidget(QWidget *parent = nullptr);

} // namespace ui
