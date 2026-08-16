#pragma once

#include <QStyledItemDelegate>

class QHelpEvent;

/// 角色列表项：圆角卡片 + 双行文字（名称/描述不同字号与字重）
class RoleListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    static constexpr int RoleDescriptionRole = Qt::UserRole + 1;

    explicit RoleListItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;
};
