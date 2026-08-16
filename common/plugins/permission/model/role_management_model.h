#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

namespace permission {

/// 角色列表行（ViewModel → View）
struct RoleListItem {
    qint64  id = 0;
    QString code; ///< 角色 code；内置角色用于 i18n（minimal / super_admin）
    QString name;
    QString description;
};

/// 新增角色表单（Model 层；code 由 Model 自动生成）
struct RoleCreateForm {
    QString code;
    QString name;
    QString description;
    QString tagColor = QStringLiteral("#3b82f6");
};

/// 角色编辑弹窗表单
struct RoleEditorForm {
    qint64  editingRoleId = 0;
    QString name;
    QString description;
};

} // namespace permission

Q_DECLARE_METATYPE(permission::RoleEditorForm)
