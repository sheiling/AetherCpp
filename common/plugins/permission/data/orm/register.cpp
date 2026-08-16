#ifdef COMMON_ORM_ENABLED

#include "entities/AuditLog.h"
#include "entities/PermissionResource.h"
#include "entities/Role.h"
#include "entities/UserAccount.h"

QX_REGISTER_CPP(UserAccount)
QX_REGISTER_CPP(Role)
QX_REGISTER_CPP(PermissionResource)
QX_REGISTER_CPP(AuditLog)

namespace qx {

template <>
void register_class(QxClass<UserAccount> &t)
{
    t.setName(QStringLiteral("perm_user"));
    t.id(&UserAccount::id, QStringLiteral("id"));
    t.data(&UserAccount::username, QStringLiteral("username"));
    t.data(&UserAccount::passwordHash, QStringLiteral("password_hash"));
    t.data(&UserAccount::displayName, QStringLiteral("display_name"));
    t.data(&UserAccount::department, QStringLiteral("department"));
    t.data(&UserAccount::status, QStringLiteral("status"));
    t.data(&UserAccount::lastLoginAt, QStringLiteral("last_login_at"));
    t.data(&UserAccount::createdAt, QStringLiteral("created_at"));
    t.data(&UserAccount::updatedAt, QStringLiteral("updated_at"));
    t.data(&UserAccount::remark, QStringLiteral("remark"));
    t.data(&UserAccount::isHidden, QStringLiteral("is_hidden"));
}

template <>
void register_class(QxClass<Role> &t)
{
    t.setName(QStringLiteral("perm_role"));
    t.id(&Role::id, QStringLiteral("id"));
    t.data(&Role::code, QStringLiteral("code"));
    t.data(&Role::name, QStringLiteral("name"));
    t.data(&Role::description, QStringLiteral("description"));
    t.data(&Role::isSystem, QStringLiteral("is_system"));
    t.data(&Role::isHidden, QStringLiteral("is_hidden"));
    t.data(&Role::tagColor, QStringLiteral("tag_color"));
    t.data(&Role::sortOrder, QStringLiteral("sort_order"));
    t.data(&Role::enabled, QStringLiteral("enabled"));
    t.data(&Role::createdAt, QStringLiteral("created_at"));
    t.data(&Role::updatedAt, QStringLiteral("updated_at"));
}

template <>
void register_class(QxClass<PermissionResource> &t)
{
    t.setName(QStringLiteral("perm_permission"));
    t.id(&PermissionResource::id, QStringLiteral("id"));
    t.data(&PermissionResource::parentId, QStringLiteral("parent_id"));
    t.data(&PermissionResource::key, QStringLiteral("key"));
    t.data(&PermissionResource::resourceType, QStringLiteral("resource_type"));
    t.data(&PermissionResource::pageKey, QStringLiteral("page_key"));
    t.data(&PermissionResource::displayName, QStringLiteral("display_name"));
    t.data(&PermissionResource::description, QStringLiteral("description"));
    t.data(&PermissionResource::source, QStringLiteral("source"));
    t.data(&PermissionResource::ownerPlugin, QStringLiteral("owner_plugin"));
    t.data(&PermissionResource::sortOrder, QStringLiteral("sort_order"));
    t.data(&PermissionResource::enabled, QStringLiteral("enabled"));
    t.data(&PermissionResource::createdAt, QStringLiteral("created_at"));
    t.data(&PermissionResource::updatedAt, QStringLiteral("updated_at"));
}

template <>
void register_class(QxClass<AuditLog> &t)
{
    t.setName(QStringLiteral("perm_audit_log"));
    t.id(&AuditLog::id, QStringLiteral("id"));
    t.data(&AuditLog::occurredAt, QStringLiteral("occurred_at"));
    t.data(&AuditLog::userId, QStringLiteral("user_id"));
    t.data(&AuditLog::username, QStringLiteral("username"));
    t.data(&AuditLog::roleNames, QStringLiteral("role_names"));
    t.data(&AuditLog::actionType, QStringLiteral("action_type"));
    t.data(&AuditLog::actionName, QStringLiteral("action_name"));
    t.data(&AuditLog::target, QStringLiteral("target"));
    t.data(&AuditLog::result, QStringLiteral("result"));
    t.data(&AuditLog::ip, QStringLiteral("ip"));
    t.data(&AuditLog::moduleCode, QStringLiteral("module_code"));
    t.data(&AuditLog::detailJson, QStringLiteral("detail_json"));
}

} // namespace qx

#endif // COMMON_ORM_ENABLED
