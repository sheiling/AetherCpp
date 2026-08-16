#pragma once

#include <QString>
#include <QVector>

namespace permission {

/// 权限资源树节点（ViewModel → View 组树）
struct PermissionResourceTreeNode {
    qint64  id            = 0;
    qint64  parentId      = 0;
    QString resourceType;
    QString displayName;
    QString key;
    bool    enabled       = true;
    int     sortOrder     = 0;
    int     childCount    = 0;
    int     grantRefCount = 0;
};

struct PermissionResourceTreeBranch {
    PermissionResourceTreeNode              node;
    QVector<PermissionResourceTreeBranch> children;
};

/// 详情 / 编辑表单
struct PermissionResourceEditorForm {
    qint64  editingId    = 0; ///< 0 = 新建
    QString resourceType;
    qint64  parentId     = 0;
    QString key;
    QString pageKey;
    QString displayName;
    QString description;
    int     sortOrder    = 0;
    bool    enabled      = true;
    QString source;
    QString parentDisplayName; ///< 只读展示
    int     grantRefCount = 0;
};

} // namespace permission
