#include "repository/repositoryfactory.h"

#include "repository/mock/mock_data_store.h"
#include "repository/mysql/mysql_data_store.h"
#include "repository/sqlite/sqlite_data_store.h"

#include <QDebug>

namespace permission {

namespace {
std::unique_ptr<IDataStore> g_instance;
}

std::unique_ptr<IDataStore> RepositoryFactory::create(const StorageConfig &config)
{
    switch (config.backend) {
    case StorageBackend::Mock:
        return std::make_unique<MockDataStore>(config);
    case StorageBackend::Sqlite:
        return std::make_unique<SqliteDataStore>(config);
    case StorageBackend::MySql:
        return std::make_unique<MysqlDataStore>(config);
    case StorageBackend::File:
        qWarning() << "[RepositoryFactory] File backend not implemented, fallback to Mock.";
        return std::make_unique<MockDataStore>(config);
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

} // namespace permission
