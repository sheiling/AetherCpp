#pragma once

#include "data/domain/domain_types.h"

#include <QAbstractListModel>
#include <QSet>
#include <QVector>

namespace permission {

struct RolePermSubPageBlock {
    PermissionRecord          subPage;
    QVector<PermissionRecord> actions;
};

struct RolePermListRow {
    PermissionRecord              nav;
    QVector<PermissionRecord>     navActions;
    QVector<RolePermSubPageBlock> subPages;
};

/// 角色权限面板：每 nav 一张合并配置卡片
class RolePermissionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RolePermissionListModel(QObject *parent = nullptr);

    int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setPermissionTree(const QVector<PermissionRecord> &all,
                           const QVector<qint64>           &grantedIds);
    QVector<qint64> grantedIds() const;

    const RolePermListRow &rowAt(int row) const;

    bool isGranted(qint64 resourceId) const;
    bool isNavGranted(qint64 navId) const;
    bool isSubPageGranted(qint64 subPageId) const;

    bool isNavSubtreeFullyGranted(qint64 navId) const;
    bool isSubPageFullyGranted(qint64 subPageId) const;

    void setNavGranted(qint64 navId, bool granted);
    void setSubPageGranted(qint64 subPageId, bool granted);
    void setActionGranted(qint64 actionId, bool granted);
    void setNavSelectAll(qint64 navId, bool selectAll);
    void setSubPageSelectAll(qint64 subPageId, bool selectAll);

    void selectAll();
    void clearAll();

    /// 恢复为指定授予集（不重建树、不重置展开状态）
    void restoreGrantedIds(const QVector<qint64> &grantedIds);

signals:
    void grantsChanged();

private:
    int  findNavRow(qint64 navId) const;
    int  findNavRowForSubPage(qint64 subPageId) const;
    const RolePermSubPageBlock *subPageBlock(qint64 subPageId, int *navRowOut) const;
    void rebuildRows(const QVector<PermissionRecord> &all);
    void refreshNavSubtree(qint64 navId);
    void emitRowRefresh(int row);

    QVector<RolePermListRow> m_rows;
    QSet<qint64>             m_granted;
};

} // namespace permission
