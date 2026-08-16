#pragma once

/**
 * @file ilighten_production_kpi_service.h
 * @brief 生产 KPI 统计服务接口（纯虚接口，无 QObject / Qt 插件 IID）
 *
 * Lighten 插件通过 LightenProductionKpiServiceImpl 实现此接口，并注册到 PluginManager 对象池。
 * 消费方通过 PluginManager::getInterface<ILightenProductionKpiService>() 获取实例。
 *
 * 数据落库表：lighten_production_kpi（与 lighten_inspection_history 共用 lighten_inspection.db）
 *
 * 包含方式：#include <interface/ilighten_production_kpi_service.h>
 */

#include <QtGlobal>
#include <QString>

/** 生产 KPI 统计（与 lighten_production_kpi schema 对齐） */
struct LightenProductionKpiRecord
{
    int    totalLightenedPanels = 0; ///< 总点亮灯板数
    int    okCount              = 0; ///< OK 数量
    int    ngCount              = 0; ///< NG 数量
    int    rwCount              = 0; ///< RW 数量
    int    insertionCount       = 0; ///< 对插次数
    double passRate             = 0.0; ///< 通过率（百分比 0–100）
};

inline bool operator==(const LightenProductionKpiRecord &a, const LightenProductionKpiRecord &b)
{
    return a.totalLightenedPanels == b.totalLightenedPanels && a.okCount == b.okCount
           && a.ngCount == b.ngCount && a.rwCount == b.rwCount
           && a.insertionCount == b.insertionCount && qFuzzyCompare(a.passRate, b.passRate);
}

inline bool operator!=(const LightenProductionKpiRecord &a, const LightenProductionKpiRecord &b)
{
    return !(a == b);
}

/**
 * @brief 生产 KPI 读写服务
 *
 * 单行统计表：表为空时 upsert 执行 INSERT，否则 UPDATE 唯一一行。
 */
class ILightenProductionKpiService
{
public:
    virtual ~ILightenProductionKpiService() = default;

    /** DataStore 已初始化且连接可用 */
    virtual bool isReady() const = 0;

    /**
     * @brief 读取当前生产 KPI
     * @param out      输出；无数据时写入全 0 默认值
     * @param errorOut 失败时写入原因
     */
    virtual bool loadKpi(LightenProductionKpiRecord *out,
                         QString *errorOut = nullptr) const = 0;

    /**
     * @brief 更新生产 KPI；表为空则插入，否则更新唯一一行
     * @param record   KPI 字段
     * @param errorOut 失败时写入原因
     */
    virtual bool upsertKpi(const LightenProductionKpiRecord &record,
                           QString *errorOut = nullptr) = 0;
};
