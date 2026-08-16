#pragma once

/**
 * @file sqlite_data_store.h
 * @brief 检测历史 SQLite IDataStore 实现
 */

#include "repository/orm/orm_data_store.h"

namespace lighten {

/** @brief SQLite 版 OrmDataStore（连接本地 .db 并执行 schema 脚本） */
class SqliteDataStore final : public OrmDataStore
{
public:
    explicit SqliteDataStore(StorageConfig config);

protected:
    bool connectDatabase(QString *errorString) override;
    bool createSchema(QString *errorString) override;
};

} // namespace lighten
