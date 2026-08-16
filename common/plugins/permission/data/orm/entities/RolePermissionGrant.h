#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QtCore/QString>

/// 角色-权限关联（junction 表，通过原生 SQL 维护）
struct RolePermissionGrant
{
    long    roleId       = 0;
    long    permissionId = 0;
    QString grantedAt;
};

#endif // COMMON_ORM_ENABLED
