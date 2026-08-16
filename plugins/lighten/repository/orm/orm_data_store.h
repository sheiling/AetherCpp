#pragma once

/**
 * @file orm_data_store.h
 * @brief SQLite / MySQL 共用的 OrmDataStore 基类
 */

#include "repository/interface/idatastore.h"
#include "repository/storageconfig.h"

#include <memory>

namespace lighten {

/**
 * @brief 基于独立 QSqlDatabase 连接的 IDataStore 基类
 *
 * SqliteDataStore / MysqlDataStore 分别实现 connectDatabase 与 createSchema。
 */
class OrmDataStore : public IDataStore
{
public:
    explicit OrmDataStore(StorageConfig config, StorageBackend backend);

    bool           open(QString *errorString) override;
    void           close() override;
    bool           isOpen() const override { return m_open; }
    StorageBackend backend() const override { return m_backend; }
    StorageConfig  config() const override { return m_config; }

    IInspectionHistoryRepository &inspectionHistory() override { return *m_history; }
    const IInspectionHistoryRepository &inspectionHistory() const override { return *m_history; }
    IProductionKpiRepository &productionKpi() override { return *m_kpi; }
    const IProductionKpiRepository &productionKpi() const override { return *m_kpi; }

protected:
    virtual bool connectDatabase(QString *errorString) = 0;
    virtual bool createSchema(QString *errorString) = 0;

private:
    StorageConfig m_config;
    StorageBackend m_backend;
    bool m_open = false;

    std::unique_ptr<IInspectionHistoryRepository> m_history;
    std::unique_ptr<IProductionKpiRepository> m_kpi;
};

} // namespace lighten
