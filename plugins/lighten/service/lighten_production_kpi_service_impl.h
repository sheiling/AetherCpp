#pragma once

/**
 * @file lighten_production_kpi_service_impl.h
 * @brief ILightenProductionKpiService 实现
 */

#include <interface/ilighten_production_kpi_service.h>

#include <QObject>

/**
 * @brief 生产 KPI 读写服务实现，注册到 PluginManager 对象池
 */
class LightenProductionKpiServiceImpl final : public QObject, public ILightenProductionKpiService
{
public:
    LightenProductionKpiServiceImpl();
    ~LightenProductionKpiServiceImpl() override = default;

    bool isReady() const override;
    bool loadKpi(LightenProductionKpiRecord *out, QString *errorOut = nullptr) const override;
    bool upsertKpi(const LightenProductionKpiRecord &record, QString *errorOut = nullptr) override;
};
