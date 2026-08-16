#pragma once

#include "uibase_global.h"

#include <QSet>
#include <QString>

/**
 * @brief 权限数据后端（由 Permission 插件实现并注册到 PermService）
 */
class UIBASE_EXPORT IPermInterface
{
public:
    virtual ~IPermInterface() = default;

    virtual bool isReady() const = 0;

    /// 按角色加载 grants（含祖先展开），元素为资源表全局唯一 key
    virtual QSet<QString> loadGrantKeysForRole(qint64 roleId) = 0;

    virtual bool primaryRoleIdForUser(qint64 userId, qint64 *roleIdOut) const = 0;

    virtual bool findUserIdByRoleCode(const QString &roleCode, qint64 *userIdOut) const = 0;

    virtual void clearAuthSession() = 0;
};
