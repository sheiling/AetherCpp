#pragma once

#include "repository/orm/orm_data_store.h"

namespace permission {

class SqliteDataStore final : public OrmDataStore
{
public:
    explicit SqliteDataStore(StorageConfig config);

protected:
    bool connectDatabase(QString *errorString) override;
    bool createSchema(QString *errorString) override;
};

} // namespace permission
