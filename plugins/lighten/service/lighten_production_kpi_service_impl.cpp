/**
 * @file lighten_production_kpi_service_impl.cpp
 * @brief ILightenProductionKpiService 实现
 *
 * 薄封装层：对外暴露 KPI 服务接口，内部统一通过 lighten::DataStore 单例
 * 访问已打开的生产 KPI 仓储（SQLite / MySQL / Mock）。
 */

#include "lighten_production_kpi_service_impl.h"

#include "repository/datastore.h"
#include "repository/interface/idatastore.h"

LightenProductionKpiServiceImpl::LightenProductionKpiServiceImpl() = default;

bool LightenProductionKpiServiceImpl::isReady() const
{
    const lighten::IDataStore *store = lighten::DataStore::instance();
    return store != nullptr && store->isOpen();
}

bool LightenProductionKpiServiceImpl::loadKpi(LightenProductionKpiRecord *out,
                                                QString *errorOut) const
{
    if (!out) {
        if (errorOut)
            *errorOut = QStringLiteral("output pointer is null");
        return false;
    }

    const lighten::IDataStore *store = lighten::DataStore::instance();
    if (!store || !store->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Lighten inspection DataStore is not ready");
        return false;
    }

    return store->productionKpi().load(out, errorOut);
}

bool LightenProductionKpiServiceImpl::upsertKpi(const LightenProductionKpiRecord &record,
                                                    QString *errorOut)
{
    lighten::IDataStore *store = lighten::DataStore::instance();
    if (!store || !store->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Lighten inspection DataStore is not ready");
        return false;
    }

    return store->productionKpi().upsert(record, errorOut);
}
