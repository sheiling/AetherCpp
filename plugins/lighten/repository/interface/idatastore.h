#pragma once

/**
 * @file idatastore.h
 * @brief 检测历史数据访问总入口接口
 */

#include "repository/interface/iinspectionhistoryrepository.h"
#include "repository/interface/iproductionkpirepository.h"
#include "repository/storageconfig.h"

#include <QString>

namespace lighten {

/**
 * @brief 检测历史 IDataStore 聚合接口
 *
 * 通过 RepositoryFactory::create(config) 按 StorageBackend 创建 Mock / SQLite / MySQL 实现。
 */
class IDataStore
{
public:
    virtual ~IDataStore() = default;

    virtual bool           open(QString *errorString = nullptr) = 0;
    virtual void           close() = 0;
    virtual bool           isOpen() const = 0;
    virtual StorageBackend backend() const = 0;
    virtual StorageConfig  config() const = 0;

    /** 检测历史宽表仓储 */
    virtual IInspectionHistoryRepository &inspectionHistory() = 0;
    virtual const IInspectionHistoryRepository &inspectionHistory() const = 0;

    /** 生产 KPI 统计表仓储 */
    virtual IProductionKpiRepository &productionKpi() = 0;
    virtual const IProductionKpiRepository &productionKpi() const = 0;
};

} // namespace lighten
