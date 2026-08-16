#pragma once

#include "data/domain/domain_types.h"
#include "model/permission_result.h"
#include "model/user_management_model.h"

#include <QString>
#include <QVector>

namespace permission {

class IDataStore;

/// 用户业务 Model（无 Q_OBJECT，对齐 lighten *Repository）
class UserModel
{
public:
    explicit UserModel(IDataStore *store = nullptr);

    static QString hashPassword(const QString &plain);
    static bool    shouldUpdatePassword(const QString &passwordField);

    QVector<UserRecord> list(const UserQueryFilter &filter) const;
    qint64              count(const UserQueryFilter &filter) const;
    bool                findById(qint64 id, UserRecord &out) const;

    OperationResult createUser(const UserEditorForm &form, const RoleRecord &role);
    OperationResult updateUser(const UserEditorForm &form, const RoleRecord &role);
    OperationResult resetPassword(qint64 userId, const QString &plainPassword);
    OperationResult setAccountStatus(qint64 userId, int accountStatus);
    OperationResult assignRoles(qint64 userId, qint64 roleId);

private:
    IDataStore *m_store = nullptr;
};

} // namespace permission
