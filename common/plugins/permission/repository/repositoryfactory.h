#pragma once

#include "repository/interface/idatastore.h"
#include "repository/storageconfig.h"

#include <memory>

namespace permission {

/**
 * @brief 按 StorageConfig 创建 IDataStore 实现
 */
class RepositoryFactory
{
public:
    static std::unique_ptr<IDataStore> create(const StorageConfig &config);

    static IDataStore *instance();
    static void setInstance(std::unique_ptr<IDataStore> store);

private:
    RepositoryFactory() = delete;
};

} // namespace permission
