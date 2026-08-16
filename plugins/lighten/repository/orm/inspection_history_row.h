#pragma once

/**
 * @file inspection_history_row.h
 * @brief lighten_inspection_history 表 QxOrm 实体（与 004 schema 字段对齐）
 */

#include <QxOrm.h>
#include <QtCore/QString>

/** QxOrm 映射实体：表 lighten_inspection_history */
struct LightenInspectionHistoryRow
{
    long    id                  = 0;
    QString product_model;       ///< 产品型号
    QString board_sn;            ///< 灯板编号
    int     inspection_result   = 0; ///< 0=OK, 1=NG, 2=RW
    int     inspect_times       = 1;
    int     rw_count            = 0;
    int     inspect_duration_ms = 0;
    int     dead_led_r          = 0;
    int     dead_led_g          = 0;
    int     dead_led_b          = 0;
    int     dim_led_r           = 0;
    int     dim_led_g           = 0;
    int     dim_led_b           = 0;
    int     bright_led_r        = 0;
    int     bright_led_g        = 0;
    int     bright_led_b        = 0;
    int     color_var_r         = 0;
    int     color_var_g         = 0;
    int     color_var_b         = 0;
    int     series_led_r        = 0;
    int     series_led_g        = 0;
    int     series_led_b        = 0;
    int     mixed_color_r       = 0;
    int     mixed_color_g       = 0;
    int     mixed_color_b       = 0;
    int     black_screen_light_count = 0;
    int     defect_total_count  = 0;
    int     defect_type         = 0; ///< 缺陷类型位掩码
    int     flash_result        = 0;  ///< 0=OK, 1=NG
    QString special_detect;
    QString inspected_at;        ///< yyyy-MM-dd HH:mm:ss
};

QX_REGISTER_HPP(LightenInspectionHistoryRow, qx::trait::no_base_class_defined, 0)

namespace qx {
template <> void register_class(QxClass<LightenInspectionHistoryRow> &t);
}
