#pragma once

/**
 * @file repositoryfactory.h
 * @brief 按 StorageConfig 创建 IDataStore 实现
 */

#include "repository/interface/idatastore.h"
#include "repository/storageconfig.h"

#include <memory>

namespace lighten {

/**
 * @brief IDataStore 工厂与进程内单例持有者
 */
class RepositoryFactory
{
public:
    /** 按 backend 创建 Mock / Sqlite / MySql 实现 */
    static std::unique_ptr<IDataStore> create(const StorageConfig &config);

    /** 当前全局 IDataStore 指针（不转移所有权） */
    static IDataStore *instance();
    /** 设置全局实例（initialize 成功后调用） */
    static void setInstance(std::unique_ptr<IDataStore> store);

private:
    RepositoryFactory() = delete;
};

} // namespace lighten
