/**
 * @file lighten_inspection_history_service_impl.cpp
 * @brief ILightenInspectionHistoryService 实现
 *
 * 薄封装层：对外暴露历史读写服务接口，内部统一通过 lighten::DataStore 单例
 * 访问已打开的检测历史仓储（SQLite / MySQL / Mock）。
 * appendRecord 成功后会同步增量更新 lighten_production_kpi。
 */

#include "lighten_inspection_history_service_impl.h"

#include "repository/datastore.h"
#include "repository/interface/idatastore.h"

#include <interface/ilighten_production_kpi_service.h>

namespace {

/** 检测历史追加成功后，按本次判级结果增量更新 lighten_production_kpi 单行统计 */
bool applyAppendToProductionKpi(lighten::IDataStore *store,
                                  const LightenInspectionHistoryRecord &record,
                                  QString *errorOut)
{
    LightenProductionKpiRecord kpi;
    if (!store->productionKpi().load(&kpi, errorOut))
        return false;

    ++kpi.totalLightenedPanels;
    switch (record.result) {
    case LightenInspectionResult::Ok:
        ++kpi.okCount;
        break;
    case LightenInspectionResult::Ng:
        ++kpi.ngCount;
        break;
    case LightenInspectionResult::Rw:
        ++kpi.rwCount;
        break;
    }
    ++kpi.insertionCount;

    const int total = kpi.okCount + kpi.ngCount + kpi.rwCount;
    kpi.passRate = total > 0 ? (100.0 * kpi.okCount / total) : 0.0;

    return store->productionKpi().upsert(kpi, errorOut);
}

} // namespace

LightenInspectionHistoryServiceImpl::LightenInspectionHistoryServiceImpl() = default;

bool LightenInspectionHistoryServiceImpl::isReady() const
{
    const lighten::IDataStore *store = lighten::DataStore::instance();
    return store != nullptr && store->isOpen();
}

qint64 LightenInspectionHistoryServiceImpl::appendRecord(const LightenInspectionHistoryRecord &record,
                                                         QString *errorOut)
{
    lighten::IDataStore *store = lighten::DataStore::instance();
    if (!store || !store->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Lighten inspection DataStore is not ready");
        return -1;
    }

    const qint64 id = store->inspectionHistory().appendRecord(record, errorOut);
    if (id < 0)
        return id;

    QString kpiError;
    if (!applyAppendToProductionKpi(store, record, &kpiError)) {
        if (errorOut && errorOut->isEmpty())
            *errorOut = kpiError;
    }
    return id;
}

QVector<LightenInspectionHistoryRecord> LightenInspectionHistoryServiceImpl::queryRecords(
    const LightenInspectionHistoryQuery &query, QString *errorOut) const
{
    const lighten::IDataStore *store = lighten::DataStore::instance();
    if (!store || !store->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Lighten inspection DataStore is not ready");
        return {};
    }

    return store->inspectionHistory().queryByServiceQuery(query);
}

qint64 LightenInspectionHistoryServiceImpl::upsertRecord(const LightenInspectionHistoryRecord &record,
                                                         QString *errorOut)
{
    lighten::IDataStore *store = lighten::DataStore::instance();
    if (!store || !store->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Lighten inspection DataStore is not ready");
        return -1;
    }

    return store->inspectionHistory().upsertRecord(record, errorOut);
}
