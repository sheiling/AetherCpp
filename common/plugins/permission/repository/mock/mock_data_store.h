#pragma once

#include "repository/interface/idatastore.h"

#include <memory>

#include <QPair>
#include <QVector>

namespace permission {

/**
 * @brief 内存 Mock 实现，内置与 UI 原型一致的示例数据
 */
class MockDataStore final : public IDataStore
{
public:
    explicit MockDataStore(StorageConfig config = StorageConfig::mock());

    bool           open(QString *errorString) override;
    void           close() override;
    bool           isOpen() const override { return m_open; }
    StorageBackend backend() const override { return StorageBackend::Mock; }
    StorageConfig  config() const override { return m_config; }

    IUserRepository               &users() override;
    IRoleRepository               &roles() override;
    IPermissionResourceRepository &permissions() override;
    IAuditRepository              &auditLogs() override;

private:
    void seedData();

    StorageConfig m_config;
    bool          m_open = false;

    QVector<UserRecord>          m_users;
    QVector<RoleRecord>          m_roles;
    QVector<PermissionRecord>    m_permissionResources;
    QVector<AuditLogRecord>      m_auditLogs;
    QVector<QPair<qint64, qint64>> m_userRoles;
    QVector<QPair<qint64, qint64>> m_rolePerms;

    std::unique_ptr<IUserRepository>                m_usersRepo;
    std::unique_ptr<IRoleRepository>                m_rolesRepo;
    std::unique_ptr<IPermissionResourceRepository>  m_permissionsRepo;
    std::unique_ptr<IAuditRepository>               m_auditLogsRepo;

    qint64 m_userIdSeq  = 100;
    qint64 m_roleIdSeq  = 10;
    qint64 m_permIdSeq  = 1000;
    qint64 m_auditIdSeq = 1;
};

} // namespace permission
