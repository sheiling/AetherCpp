#pragma once

#include "repository/orm/orm_data_store.h"

namespace permission {

class MysqlDataStore final : public OrmDataStore
{
public:
    explicit MysqlDataStore(StorageConfig config);

protected:
    bool connectDatabase(QString *errorString) override;
    bool createSchema(QString *errorString) override;
};

} // namespace permission
