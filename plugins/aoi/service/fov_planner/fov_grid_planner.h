#pragma once

/**
 * @file fov_grid_planner.h
 * @brief 根据坐标系与 FOV 尺寸，在物理扫描区域内划分 FOV 网格
 */

#include "fov_coordinate_system.h"

#include <string>
#include <vector>

namespace aoi::fov_planner {

/**
 * @brief 单个 FOV 网格单元
 *
 * index 为全局序号（行优先），与 path_planner.visitOrder、采图进度对齐。
 */
struct FovGridCell
{
    int    index = 0;           ///< 全局 FOV 序号
    int    gridRow = 0;         ///< FOV 网格行（非芯片组行）
    int    gridCol = 0;         ///< FOV 网格列
    Vec2d  centerLogical;       ///< 逻辑中心 (mm)
    Vec2d  centerPhysical;      ///< 物理中心 (mm)，path_planner 逐点目标
    Rect2d boundsLogical;       ///< 逻辑 FOV 矩形
    Rect2d boundsPhysical;      ///< 物理 FOV 矩形
    double widthMm  = 0.0;
    double heightMm = 0.0;
};

/** FOV 网格规划结果 */
struct FovGridPlan
{
    std::vector<FovGridCell> cells;
    int                      rowCount = 0;
    int                      colCount = 0;
    Rect2d                   scanBoundsPhysical; ///< 外扩 edgeExpand 后的扫描区
};

/**
 * @brief FOV 网格划分器
 *
 * 在 panelPhysicalBounds 外扩 edgeExpandMm 后，按 fov 尺寸与重叠步进生成 cells。
 */
class FovGridPlanner
{
public:
    FovGridPlan plan(const FovCoordinateSystem &coords);

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

private:
    bool        valid_ = false;
    std::string lastError_;
};

} // namespace aoi::fov_planner
