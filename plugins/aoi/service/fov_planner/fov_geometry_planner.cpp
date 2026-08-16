/**
 * @file fov_geometry_planner.cpp
 * @brief FovGeometryPlanner：坐标系 → 网格 → 芯片索引 三层串联
 */

#include "fov_geometry_planner.h"

namespace aoi::fov_planner {

FovGeometryPlan FovGeometryPlanner::plan(const FovLayoutConfig &config,
                                       const double mmPerPixel,
                                       const int imageWidthPx,
                                       const int imageHeightPx) const
{
    FovGeometryPlan result;
    result.valid_      = false;
    result.lastError_.clear();

    if (!result.coords.build(config)) {
        result.lastError_ = result.coords.lastError();
        return result;
    }

    FovGridPlanner gridPlanner;
    result.grid = gridPlanner.plan(result.coords);
    if (!gridPlanner.isValid()) {
        result.lastError_ = gridPlanner.lastError();
        return result;
    }

    if (!result.chipMap.build(result.coords, result.grid, mmPerPixel, imageWidthPx,
                              imageHeightPx)) {
        result.lastError_ = result.chipMap.lastError();
        return result;
    }

    result.valid_ = true;
    return result;
}

} // namespace aoi::fov_planner
