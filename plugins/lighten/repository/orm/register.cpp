/**
 * @file register.cpp
 * @brief LightenInspectionHistoryRow QxOrm 列注册
 */

#include "repository/orm/inspection_history_row.h"

QX_REGISTER_CPP(LightenInspectionHistoryRow)

/** QxOrm：lighten_inspection_history 表列映射注册 */
namespace qx {

template <>
void register_class(QxClass<LightenInspectionHistoryRow> &t)
{
    t.setName(QStringLiteral("lighten_inspection_history"));
    t.id(&LightenInspectionHistoryRow::id, QStringLiteral("id"));
    t.data(&LightenInspectionHistoryRow::product_model, QStringLiteral("product_model"));
    t.data(&LightenInspectionHistoryRow::board_sn, QStringLiteral("board_sn"));
    t.data(&LightenInspectionHistoryRow::inspection_result, QStringLiteral("inspection_result"));
    t.data(&LightenInspectionHistoryRow::inspect_times, QStringLiteral("inspect_times"));
    t.data(&LightenInspectionHistoryRow::rw_count, QStringLiteral("rw_count"));
    t.data(&LightenInspectionHistoryRow::inspect_duration_ms, QStringLiteral("inspect_duration_ms"));
    t.data(&LightenInspectionHistoryRow::dead_led_r, QStringLiteral("dead_led_r"));
    t.data(&LightenInspectionHistoryRow::dead_led_g, QStringLiteral("dead_led_g"));
    t.data(&LightenInspectionHistoryRow::dead_led_b, QStringLiteral("dead_led_b"));
    t.data(&LightenInspectionHistoryRow::dim_led_r, QStringLiteral("dim_led_r"));
    t.data(&LightenInspectionHistoryRow::dim_led_g, QStringLiteral("dim_led_g"));
    t.data(&LightenInspectionHistoryRow::dim_led_b, QStringLiteral("dim_led_b"));
    t.data(&LightenInspectionHistoryRow::bright_led_r, QStringLiteral("bright_led_r"));
    t.data(&LightenInspectionHistoryRow::bright_led_g, QStringLiteral("bright_led_g"));
    t.data(&LightenInspectionHistoryRow::bright_led_b, QStringLiteral("bright_led_b"));
    t.data(&LightenInspectionHistoryRow::color_var_r, QStringLiteral("color_var_r"));
    t.data(&LightenInspectionHistoryRow::color_var_g, QStringLiteral("color_var_g"));
    t.data(&LightenInspectionHistoryRow::color_var_b, QStringLiteral("color_var_b"));
    t.data(&LightenInspectionHistoryRow::series_led_r, QStringLiteral("series_led_r"));
    t.data(&LightenInspectionHistoryRow::series_led_g, QStringLiteral("series_led_g"));
    t.data(&LightenInspectionHistoryRow::series_led_b, QStringLiteral("series_led_b"));
    t.data(&LightenInspectionHistoryRow::mixed_color_r, QStringLiteral("mixed_color_r"));
    t.data(&LightenInspectionHistoryRow::mixed_color_g, QStringLiteral("mixed_color_g"));
    t.data(&LightenInspectionHistoryRow::mixed_color_b, QStringLiteral("mixed_color_b"));
    t.data(&LightenInspectionHistoryRow::black_screen_light_count,
           QStringLiteral("black_screen_light_count"));
    t.data(&LightenInspectionHistoryRow::defect_total_count, QStringLiteral("defect_total_count"));
    t.data(&LightenInspectionHistoryRow::defect_type, QStringLiteral("defect_type"));
    t.data(&LightenInspectionHistoryRow::flash_result, QStringLiteral("flash_result"));
    t.data(&LightenInspectionHistoryRow::special_detect, QStringLiteral("special_detect"));
    t.data(&LightenInspectionHistoryRow::inspected_at, QStringLiteral("inspected_at"));
}

} // namespace qx
