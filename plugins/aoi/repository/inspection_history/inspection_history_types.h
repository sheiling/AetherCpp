#pragma once

/**
 * @file inspection_history_types.h
 * @brief 检测历史仓储与 UI 共用类型（§3.1.5）
 */

#include "inspection_types.h"

#include <QDateTime>
#include <QVector>
#include <QString>

#include <array>

namespace aoi::inspection {

/** 板级检测结果（与库列 inspection_result 一致：0=OK, 1=NG, 2=RW） */
enum class InspectionHistoryResult
{
    Ok = 0,
    Ng = 1,
    Rw = 2,
};

/** 数据库一行 + 列表展示字段 */
struct InspectionHistoryRow
{
    qint64 id = -1;
    QString productName;
    QString sn;
    QString detectTime;
    InspectionHistoryResult result = InspectionHistoryResult::Ok;
    int     inspectTimes           = 0;
    int     rwCount                = 0;
    std::array<int, kInspectionDefectTypeCount> defectCounts{};

    QString defectPartitionsJson;
    QString defectInfoFullFileName;
    InspectionDefectTypeMask validDefectType = 0;
    bool                     append          = false;
    int                      chipGroupRowCount = 0;
    int                      chipGroupColCount = 0;
    int                      dieRowCount         = 0;
    int                      dieColCount         = 0;
};

/** 分页查询条件 */
struct InspectionHistoryQuery
{
    QString productName;
    QString sn;
    /** 空=全部；否则 OK / NG / RW */
    QString resultFilter;
    /** 空=全部；否则 InspectionDefectType 序号字符串 0..10 */
    QString defectTypeFilter;
    QDateTime timeFrom;
    QDateTime timeTo;
    int       pageIndex = 0;
    int       pageSize  = 20;
};

/** KPI 汇总（当前筛选条件下全量） */
struct InspectionHistoryStats
{
    double passRatePercent = 0.0;
    int    totalCount      = 0;
    int    okCount         = 0;
    int    ngCount         = 0;
    int    rwCount         = 0;
};

/** 分页结果 */
struct InspectionHistoryPage
{
    QVector<InspectionHistoryRow> rows;
    int totalCount = 0;
    int pageIndex  = 0;
    int pageSize   = 20;
    int totalPages = 1;
};

} // namespace aoi::inspection
