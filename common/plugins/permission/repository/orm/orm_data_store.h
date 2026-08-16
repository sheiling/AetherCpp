#pragma once

#include "repository/interface/idatastore.h"

#include <memory>

namespace permission {

/**
 * @brief 基于 orm::Repository<T> 的共享数据库实现（SQLite / MySQL 共用）
 */
class OrmDataStore : public IDataStore
{
public:
    explicit OrmDataStore(StorageConfig config, StorageBackend backend);

    bool           open(QString *errorString) override;
    void           close() override;
    bool           isOpen() const override { return m_open; }
    StorageBackend backend() const override { return m_backend; }
    StorageConfig  config() const override { return m_config; }

    IUserRepository               &users() override { return *m_users; }
    IRoleRepository               &roles() override { return *m_roles; }
    IPermissionResourceRepository &permissions() override { return *m_permissions; }
    IAuditRepository              &auditLogs() override { return *m_auditLogs; }

protected:
    virtual bool connectDatabase(QString *errorString) = 0;
    virtual bool createSchema(QString *errorString) = 0;

private:
    StorageConfig m_config;
    StorageBackend m_backend;
    bool m_open = false;

    std::unique_ptr<IUserRepository>                m_users;
    std::unique_ptr<IRoleRepository>                m_roles;
    std::unique_ptr<IPermissionResourceRepository>  m_permissions;
    std::unique_ptr<IAuditRepository>               m_auditLogs;
};

} // namespace permission
