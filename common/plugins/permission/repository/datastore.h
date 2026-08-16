#pragma once

#include "repository/interface/idatastore.h"
#include "repository/storageconfig.h"

namespace permission {

/**
 * @brief 全局权限数据存储配置与生命周期（应用启动时调用 configure）
 */
class DataStore
{
public:
    static void configure(const StorageConfig &config);
    static StorageConfig currentConfig();

    /// 按当前配置创建并打开数据存储；成功后可 RepositoryFactory::setInstance
    static bool initialize(QString *errorString = nullptr);

    static IDataStore *instance();
    static void shutdown();

private:
    DataStore() = delete;
};

} // namespace permission
