#pragma once

#include "repository/interface/idatastore.h"
#include "uibase/permission/iperminterface.h"

#include <QSet>
#include <QString>
#include <QVector>

namespace permission {

struct PermissionRecord;

class PermPermissionBackend : public IPermInterface
{
public:
    explicit PermPermissionBackend(IDataStore *store = nullptr);

    bool isReady() const override;

    QSet<QString> loadGrantKeysForRole(qint64 roleId) override;

    bool primaryRoleIdForUser(qint64 userId, qint64 *roleIdOut) const override;
    bool findUserIdByRoleCode(const QString &roleCode, qint64 *userIdOut) const override;
    void clearAuthSession() override;

private:
    QVector<PermissionRecord> catalog() const;
    static QSet<QString> expandGrantKeys(const QVector<PermissionRecord> &all,
                                         const QVector<qint64>           &grantedIds);

    IDataStore *m_store = nullptr;
};

} // namespace permission
