#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QtCore/QString>

/// 用户-角色关联（junction 表，通过原生 SQL 维护）
struct UserRoleAssignment
{
    long    userId    = 0;
    long    roleId    = 0;
    bool    isPrimary = false;
    QString assignedAt;
};

#endif // COMMON_ORM_ENABLED
