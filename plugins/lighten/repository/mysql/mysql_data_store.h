#pragma once

/**
 * @file mysql_data_store.h
 * @brief 检测历史 MySQL IDataStore 实现
 */

#include "repository/orm/orm_data_store.h"

namespace lighten {

/** @brief MySQL 版 OrmDataStore（可选自动建库 + schema 脚本） */
class MysqlDataStore final : public OrmDataStore
{
public:
    explicit MysqlDataStore(StorageConfig config);

protected:
    bool connectDatabase(QString *errorString) override;
    bool createSchema(QString *errorString) override;
};

} // namespace lighten
