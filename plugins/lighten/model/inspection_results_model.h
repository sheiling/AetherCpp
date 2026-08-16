#pragma once

/**
 * @file inspection_results_model.h
 * @brief 检测历史数据模型（筛选、分页、统计、CSV 导出）
 */

#include "lighten_types.h"

#include <array>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QMetaType>

namespace lighten {

/** 历史筛选条件（检测结果页顶栏） */
struct InspectionFilter
{
    QString productModel; ///< "全部" 或型号
    QString boardSerial;  ///< 板卡序列号（SN）
    QString resultFilter;      ///< 全部 / OK / NG / RW
    QString defectType;        ///< 全部 / 死灯 / …
    QString flashResultFilter; ///< 全部 / OK / NG
    QDateTime timeFrom;   ///< 筛选起始时间
    QDateTime timeTo;     ///< 筛选结束时间

    bool operator==(const InspectionFilter &o) const;
    bool operator!=(const InspectionFilter &o) const { return !(*this == o); }

    /** 返回当日默认筛选条件（时间范围 00:00 至当前） */
    static InspectionFilter defaultToday();
};

/** 汇总统计（5 张 KPI 卡） */
struct InspectionStats
{
    double passRatePercent = 0.0; ///< 通过率（百分比）
    double avgDefectCount  = 0.0; ///< 平均缺陷数
    int    okCount         = 0;   ///< OK 数量
    int    ngCount         = 0;   ///< NG 数量
    int    rwCount         = 0;   ///< RW 数量

    bool operator==(const InspectionStats &o) const
    {
        return qFuzzyCompare(passRatePercent, o.passRatePercent)
               && qFuzzyCompare(avgDefectCount, o.avgDefectCount) && okCount == o.okCount
               && ngCount == o.ngCount && rwCount == o.rwCount;
    }
    bool operator!=(const InspectionStats &o) const { return !(*this == o); }
};

/** 历史表一行（27 列语义，与检测结果宽表列对齐） */
using InspectionHistoryRecord = ::LightenInspectionHistoryRecord;

/** 分页查询结果 */
struct InspectionPage
{
    QVector<InspectionHistoryRecord> rows; ///< 当前页记录
    int totalCount  = 0;  ///< 符合条件的总记录数
    int pageIndex   = 0; ///< 0-based
    int pageSize    = 20; ///< 每页条数
    int totalPages  = 1;  ///< 总页数

    bool operator==(const InspectionPage &o) const
    {
        return rows == o.rows && totalCount == o.totalCount && pageIndex == o.pageIndex
               && pageSize == o.pageSize && totalPages == o.totalPages;
    }
    bool operator!=(const InspectionPage &o) const { return !(*this == o); }
};

/**
 * @brief 检测历史数据访问门面（委托 DataStore → IInspectionHistoryRepository）
 */
class InspectionResultsRepository
{
public:
    /** 按筛选条件分页查询检测历史 */
    static InspectionPage query(const InspectionFilter &filter, int pageIndex, int pageSize);
    /** 按筛选条件计算汇总统计（5 张 KPI 卡） */
    static InspectionStats statsFor(const InspectionFilter &filter);
    /** 将筛选结果导出为 CSV 文件 */
    static bool exportToCsv(const InspectionFilter &filter, const QString &filePath);
};

} // namespace lighten

Q_DECLARE_METATYPE(lighten::InspectionFilter)
Q_DECLARE_METATYPE(lighten::InspectionStats)
Q_DECLARE_METATYPE(lighten::InspectionHistoryRecord)
Q_DECLARE_METATYPE(lighten::InspectionPage)
