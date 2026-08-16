#pragma once

#include <QString>

namespace permission {

/**
 * 全局唯一资源 key（perm_permission.key），与 titleKey / Command::setName 对齐。
 */
struct PermKeys
{
    static inline const QString NavPerm          = QStringLiteral("nav.permission");
    static inline const QString SubPermUsers     = QStringLiteral("sub.perm.users");
    static inline const QString SubPermRoles     = QStringLiteral("sub.perm.roles");
    static inline const QString SubPermResources = QStringLiteral("sub.perm.resources");
    static inline const QString SubPermAudit     = QStringLiteral("sub.perm.audit");

    static inline const QString UsersQuery           = QStringLiteral("perm.users.query");
    static inline const QString UsersResetFilter     = QStringLiteral("perm.users.resetFilter");
    static inline const QString UsersCreate          = QStringLiteral("perm.users.create");
    static inline const QString UsersEdit            = QStringLiteral("perm.users.edit");
    static inline const QString UsersResetPassword   = QStringLiteral("perm.users.resetPassword");
    static inline const QString UsersToggleStatus    = QStringLiteral("perm.users.toggleStatus");
    static inline const QString UsersBulkEnable      = QStringLiteral("perm.users.bulkEnable");
    static inline const QString UsersBulkDisable     = QStringLiteral("perm.users.bulkDisable");
    static inline const QString UsersBulkAssignRoles = QStringLiteral("perm.users.bulkAssignRoles");

    static inline const QString RolesCreate    = QStringLiteral("perm.roles.create");
    static inline const QString RolesEdit        = QStringLiteral("perm.roles.edit");
    static inline const QString RolesSelectAll   = QStringLiteral("perm.roles.selectAll");
    static inline const QString RolesClear       = QStringLiteral("perm.roles.clear");
    static inline const QString RolesReset       = QStringLiteral("perm.roles.reset");
    static inline const QString RolesSave        = QStringLiteral("perm.roles.save");

    static inline const QString ResourcesCreateNav       = QStringLiteral("perm.resources.createNav");
    static inline const QString ResourcesCreateSubPage   = QStringLiteral("perm.resources.createSubPage");
    static inline const QString ResourcesCreateAction    = QStringLiteral("perm.resources.createAction");
    static inline const QString ResourcesToggleEnabled   = QStringLiteral("perm.resources.toggleEnabled");
    static inline const QString ResourcesReset           = QStringLiteral("perm.resources.reset");
    static inline const QString ResourcesSave            = QStringLiteral("perm.resources.save");
    static inline const QString ResourcesDelete          = QStringLiteral("perm.resources.delete");

    static inline const QString AuditQuery       = QStringLiteral("perm.audit.query");
    static inline const QString AuditResetFilter = QStringLiteral("perm.audit.resetFilter");
    static inline const QString AuditExport      = QStringLiteral("perm.audit.export");
    static inline const QString AuditPrevPage    = QStringLiteral("perm.audit.prevPage");
    static inline const QString AuditNextPage    = QStringLiteral("perm.audit.nextPage");
    static inline const QString AuditView        = QStringLiteral("perm.audit.view");
};

} // namespace permission
