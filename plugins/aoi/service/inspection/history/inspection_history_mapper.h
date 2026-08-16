#pragma once

/**
 * @file inspection_history_mapper.h
 * @brief DefectInfoList ↔ 历史行派生列（结果、缺陷计数、次数）
 */

#include "inspection_history_types.h"

#include <QVector>

namespace aoi::inspection {

/** 遍历 defectItems 统计 11 类缺陷数（复判后以复判字段为准） */
std::array<int, kInspectionDefectTypeCount> defectCountsFromList(const DefectInfoList &list);

/**
 * @brief 板级 OK/NG/RW
 * - 无缺陷项 → OK
 * - 存在未复判且带缺陷的项 → RW
 * - 全部已复判且无复判 NG → OK；否则 NG
 */
InspectionHistoryResult inspectionResultFromList(const DefectInfoList &list);

/** 由 DefectInfoList 填充可落库行（不含 id / inspect_times / rw_count，由仓储计算） */
InspectionHistoryRow rowFromDefectInfoList(const DefectInfoList &list);

/** 历史行 → 复判加载用 DefectInfoList */
DefectInfoList defectInfoListFromRow(const InspectionHistoryRow &row);

/** 导出宽表 CSV（列顺序与检测历史页表格一致；UTF-8 BOM 便于 Excel 打开） */
bool writeInspectionHistoryCsv(const QVector<InspectionHistoryRow> &rows, const QString &filePath,
                               QString *errorOut = nullptr);

} // namespace aoi::inspection
