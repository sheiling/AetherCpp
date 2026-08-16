#pragma once

/**
 * @file iproductionkpirepository.h
 * @brief 生产 KPI 统计表读写仓储接口
 */

#include <interface/ilighten_production_kpi_service.h>

#include <QString>

namespace lighten {

/**
 * @brief lighten_production_kpi 单行统计表读写
 */
class IProductionKpiRepository
{
public:
    virtual ~IProductionKpiRepository() = default;

    /** 读取当前 KPI；无行时 out 为全 0 */
    virtual bool load(LightenProductionKpiRecord *out,
                      QString *errorOut = nullptr) const = 0;

    /** 表为空则 INSERT，否则 UPDATE 唯一一行 */
    virtual bool upsert(const LightenProductionKpiRecord &record,
                        QString *errorOut = nullptr) = 0;
};

} // namespace lighten
