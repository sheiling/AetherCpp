#pragma once

#include "data/domain/domain_types.h"

#include <QString>
#include <QVector>

namespace permission {

/**
 * @brief 用户仓储接口
 */
class IUserRepository
{
public:
    virtual ~IUserRepository() = default;

    virtual QVector<UserRecord> list(const UserQueryFilter &filter) = 0;
    virtual qint64              count(const UserQueryFilter &filter) = 0;
    virtual bool                findById(qint64 id, UserRecord &out) = 0;
    virtual bool                findByUsername(const QString &username, UserRecord &out) = 0;
    virtual bool                insert(UserRecord &record, QString *errorString = nullptr) = 0;
    virtual bool                update(const UserRecord &record, QString *errorString = nullptr) = 0;
    virtual bool setAccountStatus(qint64 id, int accountStatus, QString *errorString = nullptr) = 0;
    virtual bool                resetPassword(qint64 id,
                                              const QString &passwordHash,
                                              QString *errorString = nullptr) = 0;

    /// 替换用户的全部角色关联；primaryRoleId 须包含在 roleIds 中（可为 0 表示不设主角色）
    virtual bool assignRoles(qint64 userId,
                               const QVector<qint64> &roleIds,
                               qint64 primaryRoleId,
                               QString *errorString = nullptr) = 0;

    virtual QVector<qint64> roleIdsOfUser(qint64 userId) = 0;
};

} // namespace permission
