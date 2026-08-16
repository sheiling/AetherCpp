#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>

namespace permission {

struct UserListRoleBadge {
    QString name;
    QString color;
};

/// 用户列表行（ViewModel → View 展示）
struct UserListItem {
    qint64                   id = 0;
    bool                     checked = false;
    QString                  username;
    QString                  displayName;
    QString                  department;
    QVector<UserListRoleBadge> roles;
    int                      accountStatus = 1;
    QDateTime                lastLoginAt;
    bool                     accountEnabled = true;
    bool                     accountStatusMutable = true; ///< 最小用户等不可启用/禁用
};

/// 用户编辑表单
struct UserEditorForm {
    qint64  editingUserId = 0;
    QString username;
    QString password;
    QString displayName;
    QString department;
    QString roleCode;
    int     accountStatus = 1;
    QString remark;
};

inline constexpr const char *kPasswordPlaceholder = "********";

} // namespace permission

Q_DECLARE_METATYPE(permission::UserEditorForm)
