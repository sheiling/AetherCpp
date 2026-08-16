#pragma once

/**
 * @file fov_layout_types.h
 * @brief FOV 规划布局参数：组网格、芯片几何、Marker 锚点与相机视野
 *
 * 逻辑坐标系以 Mark BL 为原点 (0,0)，TR 由组阵列与 offsetGroupMarkTr 推算；
 * 物理坐标系为机台运动坐标 (mm)。二者由 FovCoordinateSystem 中的仿射变换关联。
 */

#include <common/include/geometry.h>

namespace aoi::fov_planner {

using Vec2d  = common::geometry::Vec2d;
using Rect2d = common::geometry::Rect2d;

/** 组内三芯片颜色顺序（与配方 basic.chipColorOrder 对应） */
enum class FovChipColorOrder
{
    RGB = 0,
    BGR = 1,
};

/** 组内三芯片排列方向（与配方 basic.chipArrangement 对应） */
enum class FovChipArrangement
{
    Horizontal = 0,
    Vertical   = 1,
};

/**
 * @brief FOV 规划所需的灯板布局快照
 *
 * 由 recipe_fov_layout_adapter 从配方 + Marker 定位锚点构建；
 * 不含采集路径（pathMode 由 `service/path_planner` 读取配方并规划）。
 */
struct FovLayoutConfig
{
    /** 逻辑 Mark 左下 / 右上（mm，逻辑系） */
    Vec2d logicalMarkerBottomLeft;
    Vec2d logicalMarkerTopRight;

    /** 物理 Mark 左下 / 右上（mm，机台系；来自 Marker 定位或配方标定） */
    Vec2d physicalMarkerBottomLeft;
    Vec2d physicalMarkerTopRight;

    /** 芯片组阵列行列数（配方 basic.width / height） */
    int groupRowCount = 0;
    int groupColCount = 0;

    double chipWidthMm   = 0.5;
    double chipHeightMm  = 0.5;
    double groupWidthMm  = 1.5;
    double groupHeightMm = 0.5;

    FovChipColorOrder  chipColorOrder = FovChipColorOrder::RGB;
    FovChipArrangement chipArrangement = FovChipArrangement::Horizontal;

    /** 组中心间距 (mm) */
    double groupPitchXmm      = 1.25;
    double groupPitchYmm      = 1.25;
    /** 组内相邻芯片间距 (mm) */
    double chipPitchInGroupMm = 0.5;

    /** 组中心 → Mark 的偏移：mark = groupCenter + offset */
    Vec2d offsetGroupMarkBl;
    Vec2d offsetGroupMarkTr;

    /** 单 FOV 物理尺寸 (mm)，由 imageSize(px) × captureAccuracy(μm/px) / 1000 推算 */
    double fovWidthMm  = 10.0;
    double fovHeightMm = 10.0;
    /** 扫描区域相对芯片区外扩 (mm) */
    double edgeExpandMm = 2.0;
    /** 相邻 FOV 重叠量 (mm) */
    double fovOverlapMm = 5.0;

    int chipRowCount() const { return groupRowCount; }
    int chipColCount() const { return groupColCount; }
};

/** 指定组 (groupRow, groupCol) 的逻辑中心坐标 (mm) */
Vec2d computeGroupCenterLogical(const FovLayoutConfig &config, int groupRow, int groupCol);

/** 由 BL Mark 与组阵列推算逻辑 TR Mark 坐标 */
Vec2d computeLogicalMarkerTopRight(const FovLayoutConfig &config);

/** 芯片组区域外接矩形（逻辑坐标 mm） */
Rect2d computeChipRegionLogicalBounds(const FovLayoutConfig &config);

/** M1–M2 逻辑外接矩形 */
Rect2d computeMarkLogicalBounds(const FovLayoutConfig &config);

/** M1–M2 外接矩形外扩 3 组点间距（配方预览全图导航可点击范围） */
Rect2d computeMarkNavigateLogicalBounds(const FovLayoutConfig &config);

/** 全图导航视口：灯板外接矩形 + Mark 点（含边距） */
Rect2d computeNavigatorViewBounds(const FovLayoutConfig &config);

} // namespace aoi::fov_planner
