/**
 * @file sqlite_data_store.cpp
 * @brief SqliteDataStore 连接与建表实现
 */

#include "repository/sqlite/sqlite_data_store.h"

#include "repository/orm/db_connection.h"
#include "repository/orm/schema_initializer.h"

/** @brief SqliteDataStore 连接与 schema 初始化实现 */
namespace lighten {

SqliteDataStore::SqliteDataStore(StorageConfig config)
    : OrmDataStore(std::move(config), StorageBackend::Sqlite)
{
}

bool SqliteDataStore::connectDatabase(QString *errorString)
{
    return DbConnection::open(config(), errorString);
}

bool SqliteDataStore::createSchema(QString *errorString)
{
    return SchemaInitializer::apply(StorageBackend::Sqlite, errorString);
}

} // namespace lighten
