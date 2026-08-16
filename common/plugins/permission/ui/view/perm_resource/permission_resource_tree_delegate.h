#pragma once

#include <QStyledItemDelegate>

/// 权限资源树行：无连接线，展开箭头 + 名称 + key（action 行仅 key + 名称）
class PermissionResourceTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    static constexpr int kExpandToggleW = 24;

    enum ResourceTreeRole {
        ResourceIdRole    = Qt::UserRole + 10,
        ResourceTypeRole  = Qt::UserRole + 11,
        ResourceKeyRole   = Qt::UserRole + 12,
        HasChildrenRole   = Qt::UserRole + 13,
        ResourceEnabledRole = Qt::UserRole + 14,
    };

    explicit PermissionResourceTreeDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    static void applyTreeStyle(class QTreeView *tree);
    static QRect expandToggleRect(const QRect &visualRect, bool hasChildren);
};
