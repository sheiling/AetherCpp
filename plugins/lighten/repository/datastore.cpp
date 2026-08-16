/**
 * @file datastore.cpp
 * @brief DataStore 配置、初始化与 shutdown 实现
 */

#include "repository/datastore.h"

#include "repository/interface/idatastore.h"
#include "repository/repositoryfactory.h"

/** @brief DataStore 全局配置与生命周期实现 */
namespace lighten {

/** 进程内当前配置与待注册实例 */
namespace {
StorageConfig g_config = StorageConfig::mock();
std::unique_ptr<IDataStore> g_store;
}

void DataStore::configure(const StorageConfig &config)
{
    g_config = config;
}

StorageConfig DataStore::currentConfig()
{
    return g_config;
}

bool DataStore::initialize(QString *errorString)
{
    shutdown();
    g_store = RepositoryFactory::create(g_config);
    if (!g_store->open(errorString)) {
        g_store.reset();
        return false;
    }
    RepositoryFactory::setInstance(std::move(g_store));
    return true;
}

IDataStore *DataStore::instance()
{
    return RepositoryFactory::instance();
}

void DataStore::shutdown()
{
    if (IDataStore *s = RepositoryFactory::instance())
        s->close();
    RepositoryFactory::setInstance(nullptr);
    g_store.reset();
}

} // namespace lighten
