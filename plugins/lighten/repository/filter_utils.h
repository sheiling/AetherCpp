#pragma once

/**
 * @file filter_utils.h
 * @brief 检测历史筛选、SQL WHERE 构建、分页与 KPI 统计共用工具
 */

#include "inspection_results_model.h"
#include "lighten_types.h"

#include <interface/ilighten_inspection_history_service.h>

#include <QtGlobal>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace lighten {
namespace repo_filter {

/** 将跨插件历史查询条件转为检测结果页 InspectionFilter（其余筛选项为「全部」） */
inline InspectionFilter filterFromServiceQuery(const LightenInspectionHistoryQuery &query)
{
    InspectionFilter f;
    f.productModel = query.productModel.trimmed().isEmpty() ? QStringLiteral("全部")
                                                             : query.productModel.trimmed();
    f.boardSerial       = query.boardSerial;
    f.resultFilter      = QStringLiteral("全部");
    f.defectType        = QStringLiteral("全部");
    f.flashResultFilter = QStringLiteral("全部");
    return f;
}

/** 缺陷类型筛选下拉选项（与 defect_type 位掩码一致） */
inline QStringList defectTypeFilterOptions()
{
    return {QStringLiteral("全部"), QStringLiteral("死灯"), QStringLiteral("暗灯"),
            QStringLiteral("亮灯"), QStringLiteral("串色"), QStringLiteral("串灯"),
            QStringLiteral("混色"), QStringLiteral("黑屏亮灯"), QStringLiteral("特殊检测项")};
}

/** Flash 结果筛选下拉选项（与 flash_result 列一致：0=OK, 1=NG） */
inline QStringList flashResultFilterOptions()
{
    return {QStringLiteral("全部"), QStringLiteral("OK"), QStringLiteral("NG")};
}

/** 将 Flash 筛选项转为 DB 整型码；「全部」返回 -1 表示不过滤 */
inline int flashResultFilterCode(const QString &filterText)
{
    if (filterText == QStringLiteral("NG"))
        return 1;
    if (filterText == QStringLiteral("OK"))
        return 0;
    return -1;
}

/** 将界面缺陷类型文本转为 defect_type 位掩码（与 schema 一致） */
inline int defectTypeBitMask(const QString &defectType)
{
    if (defectType == QStringLiteral("死灯"))
        return 1 << 0;
    if (defectType == QStringLiteral("暗灯"))
        return 1 << 1;
    if (defectType == QStringLiteral("亮灯"))
        return 1 << 2;
    if (defectType == QStringLiteral("串色"))
        return 1 << 3;
    if (defectType == QStringLiteral("串灯"))
        return 1 << 4;
    if (defectType == QStringLiteral("混色"))
        return 1 << 5;
    if (defectType == QStringLiteral("黑屏亮灯"))
        return 1 << 6;
    if (defectType == QStringLiteral("特殊检测项"))
        return 1 << 7;
    return 0;
}

/** Mock 层内存记录是否匹配筛选条件 */
inline bool matchesRecord(const InspectionHistoryRecord &row, const InspectionFilter &f)
{
    if (f.productModel != QStringLiteral("全部") && row.model != f.productModel)
        return false;
    if (!f.boardSerial.isEmpty()
        && !row.serialNumber.contains(f.boardSerial.trimmed(), Qt::CaseInsensitive))
        return false;
    if (f.resultFilter != QStringLiteral("全部")
        && inspectionResultText(row.result) != f.resultFilter)
        return false;

    if (f.defectType != QStringLiteral("全部")) {
        const int mask = defectTypeBitMask(f.defectType);
        if (mask != 0 && (row.defectTypeMask & mask) == 0)
            return false;
    }

    if (const int flashCode = flashResultFilterCode(f.flashResultFilter);
        flashCode >= 0 && row.flashResult != flashCode)
        return false;

    if (f.timeFrom.isValid() || f.timeTo.isValid()) {
        const QDateTime dt =
            QDateTime::fromString(row.inspectedAt, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        if (dt.isValid()) {
            if (f.timeFrom.isValid() && dt < f.timeFrom)
                return false;
            if (f.timeTo.isValid() && dt > f.timeTo)
                return false;
        }
    }
    return true;
}

/** 根据 InspectionFilter 构建 SQL WHERE 子句及绑定参数 */
inline QString buildWhereClause(const InspectionFilter &filter, QVariantMap *bindValues)
{
    QStringList clauses;

    if (filter.productModel != QStringLiteral("全部")) {
        clauses << QStringLiteral("product_model = :product_model");
        bindValues->insert(QStringLiteral(":product_model"), filter.productModel);
    }
    if (!filter.boardSerial.trimmed().isEmpty()) {
        clauses << QStringLiteral("board_sn LIKE :board_sn");
        bindValues->insert(QStringLiteral(":board_sn"),
                           QStringLiteral("%%1%").arg(filter.boardSerial.trimmed()));
    }
    if (filter.resultFilter != QStringLiteral("全部")) {
        int code = 0;
        if (filter.resultFilter == QStringLiteral("NG"))
            code = 1;
        else if (filter.resultFilter == QStringLiteral("RW"))
            code = 2;
        clauses << QStringLiteral("inspection_result = :inspection_result");
        bindValues->insert(QStringLiteral(":inspection_result"), code);
    }
    if (filter.defectType != QStringLiteral("全部")) {
        const int mask = defectTypeBitMask(filter.defectType);
        if (mask != 0) {
            clauses << QStringLiteral("(defect_type & :defect_mask) != 0");
            bindValues->insert(QStringLiteral(":defect_mask"), mask);
        }
    }
    if (const int flashCode = flashResultFilterCode(filter.flashResultFilter); flashCode >= 0) {
        clauses << QStringLiteral("flash_result = :flash_result");
        bindValues->insert(QStringLiteral(":flash_result"), flashCode);
    }
    if (filter.timeFrom.isValid()) {
        clauses << QStringLiteral("inspected_at >= :time_from");
        bindValues->insert(QStringLiteral(":time_from"),
                           filter.timeFrom.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    }
    if (filter.timeTo.isValid()) {
        clauses << QStringLiteral("inspected_at <= :time_to");
        bindValues->insert(QStringLiteral(":time_to"),
                           filter.timeTo.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    }

    if (clauses.isEmpty())
        return {};
    return QStringLiteral(" WHERE ") + clauses.join(QStringLiteral(" AND "));
}

/** 由匹配记录列表计算 KPI 汇总 */
inline InspectionStats statsFromRows(const QVector<InspectionHistoryRecord> &rows)
{
    InspectionStats s;
    if (rows.isEmpty())
        return s;

    double defectSum = 0.0;
    for (const auto &row : rows) {
        switch (row.result) {
        case InspectionResult::Ok:
            ++s.okCount;
            break;
        case InspectionResult::Ng:
            ++s.ngCount;
            break;
        case InspectionResult::Rw:
            ++s.rwCount;
            break;
        }
        defectSum += row.blackScreenLightCount;
        for (int v : row.defectCounts)
            defectSum += v;
    }

    const int total = s.okCount + s.ngCount + s.rwCount;
    s.passRatePercent = total > 0 ? (100.0 * s.okCount / total) : 0.0;
    s.avgDefectCount  = total > 0 ? (defectSum / total) : 0.0;
    return s;
}

/** 对已匹配全量结果做内存分页切片 */
inline InspectionPage makePage(const QVector<InspectionHistoryRecord> &matched, int pageIndex,
                               int pageSize)
{
    InspectionPage page;
    page.totalCount = matched.size();
    page.pageSize   = qMax(1, pageSize);
    page.totalPages = qMax(1, (page.totalCount + page.pageSize - 1) / page.pageSize);
    page.pageIndex  = qBound(0, pageIndex, page.totalPages - 1);

    const int start = page.pageIndex * page.pageSize;
    for (int i = start; i < start + page.pageSize && i < matched.size(); ++i)
        page.rows.append(matched[i]);
    return page;
}

} // namespace repo_filter
} // namespace lighten
