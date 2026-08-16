#pragma once

#include <QString>
#include <QStringList>

namespace permission {

struct AuthSession
{
    qint64      userId       = 0;
    qint64      roleId       = 0; ///< 运行时主角色（primaryRoleId）
    QString     username;
    QString     displayName;
    QStringList roleCodes;
    QString     primaryRoleCode;
    bool        isValid() const { return userId > 0 && !username.isEmpty(); }
};

} // namespace permission
