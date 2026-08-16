#pragma once

#include "data/domain/domain_types.h"
#include "languagemanager.h"
#include "model/role_management_model.h"
#include "service/perm_system_constants.h"

#include <QString>

namespace permission_ui {

inline QString translateByKey(const QString &key)
{
    if (key.isEmpty())
        return {};
    const QByteArray utf8 = key.toUtf8();
    const QString translated = TR(utf8.constData());
    return translated == key ? QString() : translated;
}

/// 权限资源展示名：先 TR(key)，再 TR(display_name 中的 i18n key)，最后回退原文。
/// 翻译来源约定见 plugins/aoi/docs/knowledge-base/i18n/权限资源与翻译约定.md
inline QString permissionResourceDisplayText(const QString &key, const QString &fallback)
{
    const QString translated = translateByKey(key);
    if (!translated.isEmpty())
        return translated;
    const QString fromDisplay = translateByKey(fallback);
    if (!fromDisplay.isEmpty())
        return fromDisplay;
    return fallback.isEmpty() ? key : fallback;
}

inline QString builtInRoleName(const permission::RoleRecord &role)
{
    if (role.code == QLatin1String(permission::kRoleCodeMinimal)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.minimal.name"));
        if (!translated.isEmpty())
            return translated;
    }

    if (role.code == QLatin1String(permission::kRoleCodeSuperAdmin)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.super_admin.name"));
        if (!translated.isEmpty())
            return translated;
    }

    return role.name;
}

inline QString builtInRoleName(const permission::RoleListItem &role)
{
    if (role.code == QLatin1String(permission::kRoleCodeMinimal)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.minimal.name"));
        if (!translated.isEmpty())
            return translated;
    }

    if (role.code == QLatin1String(permission::kRoleCodeSuperAdmin)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.super_admin.name"));
        if (!translated.isEmpty())
            return translated;
    }

    return role.name;
}

inline QString builtInRoleDescription(const permission::RoleRecord &role)
{
    if (role.code == QLatin1String(permission::kRoleCodeMinimal)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.minimal.description"));
        if (!translated.isEmpty())
            return translated;
    }

    if (role.code == QLatin1String(permission::kRoleCodeSuperAdmin)) {
        const QString translated = translateByKey(QStringLiteral("perm.role.super_admin.description"));
        if (!translated.isEmpty())
            return translated;
    }

    return role.description;
}

inline QString builtInRoleDescription(const permission::RoleListItem &role)
{
    if (role.code == QLatin1String(permission::kRoleCodeMinimal)) {
        const QString translated =
            translateByKey(QStringLiteral("perm.role.minimal.description"));
        if (!translated.isEmpty())
            return translated;
    }

    if (role.code == QLatin1String(permission::kRoleCodeSuperAdmin)) {
        const QString translated =
            translateByKey(QStringLiteral("perm.role.super_admin.description"));
        if (!translated.isEmpty())
            return translated;
    }

    return role.description;
}

} // namespace permission_ui
