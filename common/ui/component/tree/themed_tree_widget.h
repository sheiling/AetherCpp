#pragma once

/**
 * @file themed_tree_widget.h
 * @brief 按模块分组的通用树形选择控件（模块为父节点，条目为叶子）
 */

#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

class QTreeWidget;

#include "component_global.h"

namespace ui {

/**
 * @brief 主题化模块分组树
 *
 * 用于运控轴目录、IO 目录等「模块 → 条目名」场景。
 * 仅叶子节点可选；选中时发出 itemSelected(moduleName, itemName)。
 */
class UI_COMPONENT_EXPORT ThemedTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ThemedTreeWidget(QWidget *parent = nullptr);

    /** 重建树：key 为模块名，value 为条目名列表 */
    void setGroupedItems(const QMap<QString, QStringList> &groups);

    /** 按条目名选中对应叶子（模块名未知时遍历匹配） */
    void selectItem(const QString &itemName);

    QString currentModuleName() const { return m_currentModule; }
    QString currentItemName() const { return m_currentItem; }

signals:
    void itemSelected(const QString &moduleName, const QString &itemName);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void bindTheme();
    void applyTheme();
    void retranslateTree();
    void onCurrentItemChanged();

    QTreeWidget *m_tree           = nullptr;
    QString      m_currentModule;
    QString      m_currentItem;
};

} // namespace ui
