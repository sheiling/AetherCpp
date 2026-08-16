#pragma once

#include "data/domain/domain_types.h"
#include "ui/view/role_manage/role_permission_list_model.h"

#include <QSet>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QVector>

class QEvent;
class QAbstractItemModel;
class QMouseEvent;
class QPainter;

namespace permission {

class RolePermissionListModel;

/// 角色权限面板：每 nav 一张合并卡片（子页行 + 引导线 + 操作换行）
class RolePermissionPageDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum class TitleStyle {
        Nav,
        SubPage,
    };

    explicit RolePermissionPageDelegate(RolePermissionListModel *model,
                                        QObject *parent = nullptr);

    void resetNavExpandState();
    void setInteractionEnabled(bool enabled);
    bool interactionEnabled() const { return m_interactionEnabled; }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    enum class HitTarget {
        None = 0,
        NavExpand,
        NavGroupAll,
        NavVisible,
        NavAction,
        SubVisible,
        SubGroupAll,
        SubAction,
    };

    struct HitRegion {
        HitTarget target     = HitTarget::None;
        qint64    resourceId = 0;
        QRect     rect;
    };

    struct SubBlockLayout {
        qint64                        subPageId = 0;
        int                           subTitleLeft = 0;
        QRect                         accentRect;
        QRect                         visibleRect;
        QRect                         groupAllRect;
        QVector<QPair<QRect, qint64>> actionRects;
    };

    struct LayoutMetrics {
        QRect                         cardRect;
        QRect                         navExpandRect;
        int                           navTitleLeft = 0;
        QRect                         navVisibleRect;
        QRect                         navGroupAllRect;
        QVector<QPair<QRect, qint64>> navActionRects;
        QVector<SubBlockLayout>       subBlocks;
        bool                          navExpanded = true;
        int                           totalHeight = 0;
    };

    LayoutMetrics computeLayout(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

    void paintNavCard(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index, const LayoutMetrics &layout) const;

    void paintCheckboxItem(QPainter *painter, const QRect &itemRect, bool checked, bool enabled,
                           const QString &label) const;
    void paintActionTags(QPainter *painter, const QVector<QPair<QRect, qint64>> &tagRects,
                         const QVector<PermissionRecord> &actions, bool parentEnabled) const;
    void paintTitleWithVisible(QPainter *painter, const QRect &lineRect, const QString &title,
                               TitleStyle titleStyle, const QRect &visibleRect, bool visibleChecked,
                               bool visibleEnabled) const;
    void paintNavHeaderRow(QPainter *painter, const QStyleOptionViewItem &option,
                           const RolePermListRow &row, const LayoutMetrics &layout, bool navGranted,
                           bool navAllOn) const;
    void paintExpandToggle(QPainter *painter, const QRect &rect, bool expanded) const;
    void paintSectionAccent(QPainter *painter, const QRect &accentRect) const;

    HitRegion hitTest(const LayoutMetrics &layout, const QPoint &pos) const;

    bool isNavExpanded(qint64 navId) const;
    void toggleNavExpanded(qint64 navId);
    void relayoutListView(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int groupAllButtonWidth(const QFont &font) const;

    RolePermissionListModel *m_model = nullptr;
    QSet<qint64>              m_collapsedNavIds;
    bool                      m_interactionEnabled = true;
};

} // namespace permission
