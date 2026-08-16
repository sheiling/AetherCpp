#pragma once

#include "ui/view/user_manage/user_list_item_model.h"
#include "ui/view/user_manage/user_list_layout.h"

#include <QStyledItemDelegate>

#ifdef Edit
#undef Edit
#endif

class UserRowDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit UserRowDelegate(QObject *parent = nullptr);

    void setActionLabels(const QString &edit, const QString &resetPassword,
                         const QString &enable, const QString &disable);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void rowActionTriggered(UserRowAction action, qint64 userId);

private:
    void paintCheckbox(QPainter *painter, const QRect &rect, bool checked) const;
    void paintText(QPainter *painter, const QRect &rect, const QString &text,
                   const QColor &color, Qt::Alignment align) const;
    void paintRoleBadges(QPainter *painter, const QRect &rect,
                         const QVariantList &roles) const;
    QVector<QPair<UserRowAction, QRect>> actionButtonRects(const QRect &rect,
                                                           bool enabled) const;
    void paintActionButtons(QPainter *painter, const QRect &rect, bool enabled,
                            bool accountStatusMutable) const;
    bool hitAction(const QPoint &pos, const QRect &actionsRect, bool enabled,
                   bool accountStatusMutable, UserRowAction *action) const;
    bool actionPermitted(UserRowAction action, bool accountStatusMutable) const;

    QString m_lblEdit;
    QString m_lblReset;
    QString m_lblEnable;
    QString m_lblDisable;
};
