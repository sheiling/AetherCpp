#pragma once

#include "repository/interface/iauditrepository.h"
#include "repository/interface/ipermissionresourcerepository.h"
#include "repository/interface/irolerepository.h"
#include "repository/interface/iuserrepository.h"
#include "repository/storageconfig.h"

#include <QString>

namespace permission {

/**
 * @brief 权限数据访问总入口（聚合各子 Repository）
 *
 * 通过 RepositoryFactory::create(config) 按 StorageConfig.backend 创建实现：
 *   Mock / SQLite / MySQL（File 预留）
 */
class IDataStore
{
public:
    virtual ~IDataStore() = default;

    virtual bool            open(QString *errorString = nullptr) = 0;
    virtual void            close() = 0;
    virtual bool            isOpen() const = 0;
    virtual StorageBackend  backend() const = 0;
    virtual StorageConfig   config() const = 0;

    virtual IUserRepository               &users() = 0;
    virtual IRoleRepository               &roles() = 0;
    virtual IPermissionResourceRepository &permissions() = 0;
    virtual IAuditRepository              &auditLogs() = 0;
};

} // namespace permission
