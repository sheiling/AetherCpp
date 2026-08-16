#pragma once

/**
 * @file param_define.h
 * @brief AOI 参数模型共享枚举、几何类型与常量
 *
 * 路径：plugins/aoi/data/dataDefinition/
 * 枚举按整型序号持久化到 JSON，**禁止调整已有成员数值**，新增只能追加在末尾。
 * 缺陷类型见 `inspection_types.h`（`InspectionDefectType`）。
 */

#include "param_meta.h"
#include "param_model_macros.h"

#include <QMetaType>

namespace aoi {

// ── 轨道模式 ─────────────────────────────────────────────────────────────────

/** 设备轨道配置 */
enum class TrackMode
{
    SingleTrack = 0,
    DualTrack   = 1,
};

/** 按轨道模式返回配方 Mark 槽位数量（单轨 2 / 双轨 4） */
inline int recipeMarkCount(TrackMode mode)
{
    return mode == TrackMode::SingleTrack ? 2 : 4;
}

/** Mark 页签翻译键（随系统配置 trackMode 动态生成） */
inline QStringList recipeMarkTabKeys(TrackMode mode)
{
    QStringList keys;
    keys << QStringLiteral("aoi.recipe.mark.t1_bl") << QStringLiteral("aoi.recipe.mark.t1_tr");
    if (mode == TrackMode::DualTrack)
        keys << QStringLiteral("aoi.recipe.mark.t2_bl") << QStringLiteral("aoi.recipe.mark.t2_tr");
    return keys;
}

// ── 标定 ────────────────────────────────────────────────────────────────────

enum class CalibMode
{
    Panel  = 0,
    Gerber = 1,
};

// ── 几何类型 ─────────────────────────────────────────────────────────────────

#define POINT3_FIELDS(X) \
    X(double, x, 0.0, "x") \
    X(double, y, 0.0, "y") \
    X(double, z, 0.0, "z")

#define POINT_FIELDS(X) \
    X(double, x, 0.0, "x") \
    X(double, y, 0.0, "y")

#define RECT_FIELDS(X) \
    X(int, x, 0, "x") \
    X(int, y, 0, "y") \
    X(int, w, 0, "w") \
    X(int, h, 0, "h")

/** 三维坐标（μm/mm 等单位由业务字段语义决定） */
struct Point3
{
    POINT3_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(Point3, POINT3_FIELDS)
};

/** 二维坐标对（标定角点、Mark 实测坐标等） */
struct Point
{
    POINT_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(Point, POINT_FIELDS)
};

/** 整数像素矩形（ROI 等区域） */
struct Rect
{
    RECT_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(Rect, RECT_FIELDS)
};

} // namespace aoi

DEFINE_PARAM_TYPE_META(aoi::Point3, "point3", POINT3_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::Point, "point", POINT_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::Rect, "rect", RECT_FIELDS)

Q_DECLARE_METATYPE(aoi::TrackMode)
Q_DECLARE_METATYPE(aoi::Point3)
Q_DECLARE_METATYPE(aoi::Point)
Q_DECLARE_METATYPE(aoi::Rect)

#undef RECT_FIELDS
#undef POINT_FIELDS
#undef POINT3_FIELDS
