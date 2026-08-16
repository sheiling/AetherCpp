#pragma once

#ifdef COMMON_ORM_ENABLED

#include "data/domain/domain_types.h"
#include "data/orm/entities/AuditLog.h"
#include "data/orm/entities/PermissionResource.h"
#include "data/orm/entities/Role.h"
#include "data/orm/entities/UserAccount.h"

#include <QtCore/QDateTime>

namespace permission {
namespace orm_mapper {

inline QString nowIso()
{
    return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

inline QDateTime parseIso(const QString &s)
{
    if (s.isEmpty())
        return {};
    return QDateTime::fromString(s, Qt::ISODateWithMs);
}

inline RoleRecord toRole(const Role &e)
{
    RoleRecord r;
    r.id          = e.id;
    r.code        = e.code;
    r.name        = e.name;
    r.description = e.description;
    r.isSystem    = e.isSystem;
    r.isHidden    = e.isHidden;
    r.tagColor    = e.tagColor;
    r.sortOrder   = e.sortOrder;
    r.enabled     = e.enabled;
    return r;
}

inline Role fromRole(const RoleRecord &r)
{
    Role e;
    e.id          = r.id;
    e.code        = r.code;
    e.name        = r.name;
    e.description = r.description;
    e.isSystem    = r.isSystem;
    e.isHidden    = r.isHidden;
    e.tagColor    = r.tagColor;
    e.sortOrder   = r.sortOrder;
    e.enabled     = r.enabled;
    e.createdAt   = nowIso();
    e.updatedAt   = e.createdAt;
    return e;
}

inline PermissionRecord toPermission(const PermissionResource &e)
{
    PermissionRecord r;
    r.id        = e.id;
    r.parentId  = e.parentId > 0 ? e.parentId : 0;
    r.key           = e.key;
    r.resourceType  = e.resourceType;
    r.pageKey       = e.pageKey;
    r.displayName   = e.displayName;
    r.description   = e.description;
    r.source        = e.source;
    r.ownerPlugin   = e.ownerPlugin;
    r.sortOrder = e.sortOrder;
    r.enabled   = e.enabled;
    return r;
}

inline PermissionResource fromPermission(const PermissionRecord &r)
{
    PermissionResource e;
    e.id        = r.id;
    e.parentId  = r.parentId;
    e.key           = r.key;
    e.resourceType  = r.resourceType;
    e.pageKey       = r.pageKey;
    e.displayName   = r.displayName;
    e.description   = r.description;
    e.source        = r.source.isEmpty() ? QStringLiteral("manual") : r.source;
    e.ownerPlugin   = r.ownerPlugin;
    e.sortOrder = r.sortOrder;
    e.enabled   = r.enabled;
    e.createdAt = nowIso();
    e.updatedAt = e.createdAt;
    return e;
}

inline UserRecord toUser(const UserAccount &e)
{
    UserRecord r;
    r.id           = e.id;
    r.username     = e.username;
    r.displayName  = e.displayName;
    r.department = e.department;
    r.remark     = e.remark;
    r.accountStatus = e.status;
    r.lastLoginAt   = parseIso(e.lastLoginAt);
    r.isHidden      = e.isHidden;
    return r;
}

inline UserAccount fromUser(const UserRecord &r, const QString &passwordHash = QString())
{
    UserAccount e;
    e.id           = r.id;
    e.username     = r.username;
    e.passwordHash = passwordHash;
    e.displayName  = r.displayName;
    e.department   = r.department;
    e.remark       = r.remark;
    e.isHidden     = r.isHidden;
    e.status       = r.accountStatus;
    if (r.lastLoginAt.isValid())
        e.lastLoginAt = r.lastLoginAt.toString(Qt::ISODateWithMs);
    e.createdAt = nowIso();
    e.updatedAt = e.createdAt;
    return e;
}

inline AuditLogRecord toAudit(const AuditLog &e)
{
    AuditLogRecord r;
    r.id          = e.id;
    r.occurredAt  = parseIso(e.occurredAt);
    r.userId      = e.userId;
    r.username    = e.username;
    r.roleNames   = e.roleNames;
    r.actionType  = e.actionType;
    r.actionName  = e.actionName;
    r.target      = e.target;
    r.result      = e.result;
    r.ip          = e.ip;
    r.moduleCode  = e.moduleCode;
    r.detailJson  = e.detailJson;
    return r;
}

inline AuditLog fromAudit(const AuditLogRecord &r)
{
    AuditLog e;
    e.id          = r.id;
    e.occurredAt  = r.occurredAt.isValid() ? r.occurredAt.toString(Qt::ISODateWithMs)
                                           : nowIso();
    e.userId      = r.userId;
    e.username    = r.username;
    e.roleNames   = r.roleNames;
    e.actionType  = r.actionType;
    e.actionName  = r.actionName;
    e.target      = r.target;
    e.result      = r.result;
    e.ip          = r.ip;
    e.moduleCode  = r.moduleCode;
    e.detailJson  = r.detailJson;
    return e;
}

} // namespace orm_mapper
} // namespace permission

#endif // COMMON_ORM_ENABLED
