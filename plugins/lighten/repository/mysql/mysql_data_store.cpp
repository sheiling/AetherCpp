/**
 * @file mysql_data_store.cpp
 * @brief MysqlDataStore 连接与建表实现
 */

#include "repository/mysql/mysql_data_store.h"

#include "repository/orm/db_connection.h"
#include "repository/orm/schema_initializer.h"

/** @brief MysqlDataStore 连接与 schema 初始化实现 */
namespace lighten {

MysqlDataStore::MysqlDataStore(StorageConfig config)
    : OrmDataStore(std::move(config), StorageBackend::MySql)
{
}

bool MysqlDataStore::connectDatabase(QString *errorString)
{
    if (config().dbConfig.resolvedDriverName() != QStringLiteral("QMYSQL")) {
        if (errorString)
            *errorString = QStringLiteral("MySQL backend requires DbConfig with QMYSQL driver.");
        return false;
    }
    return DbConnection::open(config(), errorString);
}

bool MysqlDataStore::createSchema(QString *errorString)
{
    return SchemaInitializer::apply(StorageBackend::MySql, errorString);
}

} // namespace lighten
