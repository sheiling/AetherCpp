#include "repository/sqlite/sqlite_data_store.h"

#include "repository/orm/schema_initializer.h"

#include "repository/local/local_db_manager.h"

#include <QDir>
#include <QFileInfo>

namespace permission {

SqliteDataStore::SqliteDataStore(StorageConfig config)
    : OrmDataStore(std::move(config), StorageBackend::Sqlite)
{
}

bool SqliteDataStore::connectDatabase(QString *errorString)
{
    orm::DbConfig cfg = config().dbConfig;
    if (cfg.database.isEmpty())
        cfg = orm::DbConfig::sqlite(config().sqliteFilePath, cfg.traceSql);

    QFileInfo fi(cfg.database);
    QDir().mkpath(fi.absolutePath());

    if (!orm::DbManager::instance().init(cfg)) {
        if (errorString)
            *errorString = orm::DbManager::instance().lastError();
        return false;
    }
    return true;
}

bool SqliteDataStore::createSchema(QString *errorString)
{
    return SchemaInitializer::apply(StorageBackend::Sqlite, errorString);
}

} // namespace permission
