#pragma once

/**
 * @file iinspectionhistoryrepository.h
 * @brief 检测历史宽表查询仓储接口
 */

#include "inspection_results_model.h"

#include <interface/ilighten_inspection_history_service.h>

namespace lighten {

/**
 * @brief 检测历史记录查询接口（筛选、分页、统计、全量导出）
 */
class IInspectionHistoryRepository
{
public:
    virtual ~IInspectionHistoryRepository() = default;

    /** 按筛选条件分页查询 */
    virtual InspectionPage queryPage(const InspectionFilter &filter, int pageIndex,
                                     int pageSize) const = 0;
    /** 按筛选条件计算 KPI 汇总 */
    virtual InspectionStats statsFor(const InspectionFilter &filter) const = 0;
    /** 按筛选条件查询全部记录（CSV 导出等） */
    virtual QVector<InspectionHistoryRecord> queryAll(const InspectionFilter &filter) const = 0;

    /**
     * @brief 追加一条检测历史记录
     * @return 新行 id（>0）；失败返回 -1，可选写入 errorOut
     */
    virtual qint64 appendRecord(const InspectionHistoryRecord &record,
                                QString *errorOut = nullptr) = 0;

    /** 按 product_model + board_sn 定位；存在则更新，否则插入 */
    virtual qint64 upsertRecord(const InspectionHistoryRecord &record,
                                  QString *errorOut = nullptr) = 0;

    /** 跨插件查询：将 LightenInspectionHistoryQuery 转为 InspectionFilter 后 queryAll */
    virtual QVector<InspectionHistoryRecord> queryByServiceQuery(
        const LightenInspectionHistoryQuery &query) const = 0;
};

} // namespace lighten
