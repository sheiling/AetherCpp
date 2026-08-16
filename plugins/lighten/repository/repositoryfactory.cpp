/**
 * @file repositoryfactory.cpp
 * @brief RepositoryFactory 创建逻辑与全局实例管理
 */

#include "repository/repositoryfactory.h"

#include "repository/mock/mock_data_store.h"
#if COMMON_ORM_ENABLED
#include "repository/mysql/mysql_data_store.h"
#include "repository/sqlite/sqlite_data_store.h"
#endif

#include <QDebug>

/** @brief RepositoryFactory 创建与单例持有实现 */
namespace lighten {

/** 全局 IDataStore 实例 */
namespace {
std::unique_ptr<IDataStore> g_instance;
}

std::unique_ptr<IDataStore> RepositoryFactory::create(const StorageConfig &config)
{
    switch (config.backend) {
    case StorageBackend::Mock:
        return std::make_unique<MockDataStore>(config);
#if COMMON_ORM_ENABLED
    case StorageBackend::Sqlite:
        return std::make_unique<SqliteDataStore>(config);
    case StorageBackend::MySql:
        return std::make_unique<MysqlDataStore>(config);
#else
    case StorageBackend::Sqlite:
    case StorageBackend::MySql:
        qWarning() << "[RepositoryFactory] ORM disabled; falling back to MockDataStore";
        break;
#endif
    }
    return std::make_unique<MockDataStore>(config);
}

IDataStore *RepositoryFactory::instance()
{
    return g_instance.get();
}

void RepositoryFactory::setInstance(std::unique_ptr<IDataStore> store)
{
    g_instance = std::move(store);
}

} // namespace lighten
