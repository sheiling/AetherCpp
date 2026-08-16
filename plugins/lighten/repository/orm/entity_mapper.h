#pragma once

/**
 * @file entity_mapper.h
 * @brief ORM 行实体与 UI 模型 InspectionHistoryRecord 互转
 */

#include "inspection_results_model.h"
#include "lighten_types.h"
#include "repository/orm/inspection_history_row.h"

#include <QDateTime>
#include <QSqlQuery>

namespace lighten {
namespace orm_mapper {

namespace {

int defectTotalFromRecord(const InspectionHistoryRecord &r)
{
    int total = r.blackScreenLightCount;
    for (int count : r.defectCounts)
        total += count;
    return total;
}

} // namespace

/** 将 UI/服务层记录转为 ORM 插入实体 */
inline LightenInspectionHistoryRow rowFromRecord(const InspectionHistoryRecord &r)
{
    LightenInspectionHistoryRow row;
    row.product_model       = r.model.trimmed();
    row.board_sn            = r.serialNumber.trimmed();
    row.inspection_result   = static_cast<int>(r.result);
    row.inspect_times       = qMax(0, r.inspectTimes);
    row.rw_count            = qMax(0, r.rwCount);
    row.inspect_duration_ms = qMax(0, r.inspectDurationMs);
    row.dead_led_r          = r.defectCounts[0];
    row.dead_led_g          = r.defectCounts[1];
    row.dead_led_b          = r.defectCounts[2];
    row.dim_led_r           = r.defectCounts[3];
    row.dim_led_g           = r.defectCounts[4];
    row.dim_led_b           = r.defectCounts[5];
    row.bright_led_r        = r.defectCounts[6];
    row.bright_led_g        = r.defectCounts[7];
    row.bright_led_b        = r.defectCounts[8];
    row.color_var_r         = r.defectCounts[9];
    row.color_var_g         = r.defectCounts[10];
    row.color_var_b         = r.defectCounts[11];
    row.series_led_r        = r.defectCounts[12];
    row.series_led_g        = r.defectCounts[13];
    row.series_led_b        = r.defectCounts[14];
    row.mixed_color_r       = r.defectCounts[15];
    row.mixed_color_g       = r.defectCounts[16];
    row.mixed_color_b       = r.defectCounts[17];
    row.black_screen_light_count = qMax(0, r.blackScreenLightCount);
    row.defect_total_count  = defectTotalFromRecord(r);
    row.defect_type         = r.defectTypeMask;
    row.flash_result        = r.flashResult;
    row.special_detect      = r.specialDetect;
    row.inspected_at        = r.inspectedAt.trimmed();
    if (row.inspected_at.isEmpty())
        row.inspected_at = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    return row;
}

/** 从 SELECT * 当前行填充 ORM 实体（列名与 schema 一致） */
inline LightenInspectionHistoryRow rowFromSqlQuery(const QSqlQuery &query)
{
    LightenInspectionHistoryRow row;
    row.id                  = query.value(QStringLiteral("id")).toLongLong();
    row.product_model       = query.value(QStringLiteral("product_model")).toString();
    row.board_sn            = query.value(QStringLiteral("board_sn")).toString();
    row.inspection_result   = query.value(QStringLiteral("inspection_result")).toInt();
    row.inspect_times       = query.value(QStringLiteral("inspect_times")).toInt();
    row.rw_count            = query.value(QStringLiteral("rw_count")).toInt();
    row.inspect_duration_ms = query.value(QStringLiteral("inspect_duration_ms")).toInt();
    row.dead_led_r          = query.value(QStringLiteral("dead_led_r")).toInt();
    row.dead_led_g          = query.value(QStringLiteral("dead_led_g")).toInt();
    row.dead_led_b          = query.value(QStringLiteral("dead_led_b")).toInt();
    row.dim_led_r           = query.value(QStringLiteral("dim_led_r")).toInt();
    row.dim_led_g           = query.value(QStringLiteral("dim_led_g")).toInt();
    row.dim_led_b           = query.value(QStringLiteral("dim_led_b")).toInt();
    row.bright_led_r        = query.value(QStringLiteral("bright_led_r")).toInt();
    row.bright_led_g        = query.value(QStringLiteral("bright_led_g")).toInt();
    row.bright_led_b        = query.value(QStringLiteral("bright_led_b")).toInt();
    row.color_var_r         = query.value(QStringLiteral("color_var_r")).toInt();
    row.color_var_g         = query.value(QStringLiteral("color_var_g")).toInt();
    row.color_var_b         = query.value(QStringLiteral("color_var_b")).toInt();
    row.series_led_r        = query.value(QStringLiteral("series_led_r")).toInt();
    row.series_led_g        = query.value(QStringLiteral("series_led_g")).toInt();
    row.series_led_b        = query.value(QStringLiteral("series_led_b")).toInt();
    row.mixed_color_r       = query.value(QStringLiteral("mixed_color_r")).toInt();
    row.mixed_color_g       = query.value(QStringLiteral("mixed_color_g")).toInt();
    row.mixed_color_b       = query.value(QStringLiteral("mixed_color_b")).toInt();
    row.black_screen_light_count =
        query.value(QStringLiteral("black_screen_light_count")).toInt();
    row.defect_total_count = query.value(QStringLiteral("defect_total_count")).toInt();
    row.defect_type        = query.value(QStringLiteral("defect_type")).toInt();
    row.flash_result       = query.value(QStringLiteral("flash_result")).toInt();
    row.special_detect     = query.value(QStringLiteral("special_detect")).toString();
    row.inspected_at       = query.value(QStringLiteral("inspected_at")).toString();
    return row;
}

/** 将数据库行映射为检测结果页宽表一行 */
inline InspectionHistoryRecord toRecord(const LightenInspectionHistoryRow &row)
{
    InspectionHistoryRecord r;
    r.model         = row.product_model;
    r.serialNumber  = row.board_sn;
    r.result        = static_cast<InspectionResult>(row.inspection_result);
    r.inspectTimes  = row.inspect_times;
    r.rwCount       = row.rw_count;
    r.inspectDurationMs = row.inspect_duration_ms;
    r.blackScreenLightCount = row.black_screen_light_count;
    r.defectCounts  = {
        row.dead_led_r,   row.dead_led_g,   row.dead_led_b,   row.dim_led_r,    row.dim_led_g,
        row.dim_led_b,    row.bright_led_r, row.bright_led_g, row.bright_led_b, row.color_var_r,
        row.color_var_g,  row.color_var_b,  row.series_led_r, row.series_led_g, row.series_led_b,
        row.mixed_color_r, row.mixed_color_g, row.mixed_color_b};
    r.defectTypeMask = row.defect_type;
    r.flashResult    = row.flash_result;
    r.specialDetect = row.special_detect;
    r.inspectedAt   = row.inspected_at;
    return r;
}

} // namespace orm_mapper
} // namespace lighten
