#pragma once

#include "data/domain/domain_types.h"
#include "service/perm_system_constants.h"

namespace permission {

inline bool isSuperAdminRole(const RoleRecord &role)
{
    return role.isHidden || role.code == QLatin1String(kRoleCodeSuperAdmin);
}

inline bool isMinimalRole(const RoleRecord &role)
{
    return role.code == QLatin1String(kRoleCodeMinimal);
}

inline bool isSuperUser(const UserRecord &user)
{
    if (user.isHidden)
        return true;
    return user.roleCodes.contains(QLatin1String(kRoleCodeSuperAdmin));
}

inline bool isMinimalUser(const UserRecord &user)
{
    return user.username == QLatin1String(kMinimalUsername);
}

inline bool canChangeUserAccountStatus(const UserRecord &user)
{
    return !isMinimalUser(user) && !isSuperUser(user);
}

inline bool canAssignRoleCode(const QString &roleCode)
{
    return roleCode != QLatin1String(kRoleCodeSuperAdmin);
}

inline bool canDeleteRole(const RoleRecord &role)
{
    return !isSuperAdminRole(role) && !isMinimalRole(role) && !role.isSystem;
}

/// 角色管理弹窗可编辑名称与说明（内置/隐藏角色不可改）
inline bool canEditRoleMeta(const RoleRecord &role)
{
    return !isSuperAdminRole(role) && !isMinimalRole(role) && !role.isSystem;
}

} // namespace permission
