#pragma once

/**
 * @file mock_data_store.h
 * @brief 检测历史 Mock 内存存储实现
 */

#include "inspection_results_model.h"
#include "repository/interface/idatastore.h"

#include <interface/ilighten_production_kpi_service.h>

#include <memory>
#include <QVector>

namespace lighten {

/**
 * @brief 内存 Mock IDataStore，无内置演示数据
 */
class MockDataStore final : public IDataStore
{
public:
    explicit MockDataStore(StorageConfig config = StorageConfig::mock());

    bool           open(QString *errorString) override;
    void           close() override;
    bool           isOpen() const override { return m_open; }
    StorageBackend backend() const override { return StorageBackend::Mock; }
    StorageConfig  config() const override { return m_config; }

    IInspectionHistoryRepository &inspectionHistory() override { return *m_history; }
    const IInspectionHistoryRepository &inspectionHistory() const override { return *m_history; }
    IProductionKpiRepository &productionKpi() override { return *m_kpi; }
    const IProductionKpiRepository &productionKpi() const override { return *m_kpi; }

private:
    StorageConfig m_config;
    bool          m_open = false;
    QVector<InspectionHistoryRecord> m_rows;
    LightenProductionKpiRecord m_kpiRow;

    std::unique_ptr<IInspectionHistoryRepository> m_history;
    std::unique_ptr<IProductionKpiRepository> m_kpi;
};

} // namespace lighten
