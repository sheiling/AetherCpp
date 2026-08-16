/**
 * @file fov_grid_planner.cpp
 * @brief FovGridPlanner：物理扫描区内 FOV 网格划分
 */

#include "fov_grid_planner.h"

#include <algorithm>
#include <cmath>

#include <QtGlobal>

namespace aoi::fov_planner {

namespace {

constexpr int kMaxFovGridCells = 10000;

} // namespace

FovGridPlan FovGridPlanner::plan(const FovCoordinateSystem &coords)
{
    FovGridPlan plan;
    valid_      = false;
    lastError_.clear();

    if (!coords.isValid()) {
        lastError_ = "coordinate system invalid";
        return plan;
    }

    const FovLayoutConfig &cfg    = coords.config();
    Rect2d                 bounds = coords.panelPhysicalBounds();
    // 扫描区 = 芯片物理外接 + edgeExpandMm 四向外扩
    bounds.x -= cfg.edgeExpandMm;
    bounds.y -= cfg.edgeExpandMm;
    bounds.width += 2.0 * cfg.edgeExpandMm;
    bounds.height += 2.0 * cfg.edgeExpandMm;
    plan.scanBoundsPhysical = bounds;

    // 步进 = FOV 尺寸 − 重叠；下限 25% FOV 防止重叠过大导致步进为负
    const double stepX = std::max(cfg.fovWidthMm - cfg.fovOverlapMm, cfg.fovWidthMm * 0.25);
    const double stepY = std::max(cfg.fovHeightMm - cfg.fovOverlapMm, cfg.fovHeightMm * 0.25);

    plan.colCount = std::max(static_cast<int>(std::ceil(bounds.width / stepX)), 1);
    plan.rowCount = std::max(static_cast<int>(std::ceil(bounds.height / stepY)), 1);

    if (static_cast<qint64>(plan.rowCount) * plan.colCount > kMaxFovGridCells) {
        lastError_ = "fov grid cell count exceeded";
        return plan;
    }

    const double startX = bounds.left() + cfg.fovWidthMm * 0.5;
    const double startY = bounds.bottom() + cfg.fovHeightMm * 0.5;

    int index = 0;
    for (int r = 0; r < plan.rowCount; ++r) {
        for (int c = 0; c < plan.colCount; ++c) {
            FovGridCell cell;
            cell.index    = index++;
            cell.gridRow  = r;
            cell.gridCol  = c;
            cell.widthMm  = cfg.fovWidthMm;
            cell.heightMm = cfg.fovHeightMm;

            const double cx = startX + c * stepX;
            const double cy = startY + r * stepY;
            cell.centerPhysical = {cx, cy};
            cell.centerLogical  = coords.physicalToLogical(cell.centerPhysical);

            cell.boundsPhysical = {cx - cfg.fovWidthMm * 0.5,
                                   cy - cfg.fovHeightMm * 0.5,
                                   cfg.fovWidthMm,
                                   cfg.fovHeightMm};

            // 物理矩形四角反算逻辑边界（仿射下取 min/max 包络）
            const Vec2d bl = coords.physicalToLogical(
                {cell.boundsPhysical.left(), cell.boundsPhysical.bottom()});
            const Vec2d tr = coords.physicalToLogical(
                {cell.boundsPhysical.right(), cell.boundsPhysical.top()});
            cell.boundsLogical = {std::min(bl.x, tr.x),
                                  std::min(bl.y, tr.y),
                                  std::abs(tr.x - bl.x),
                                  std::abs(tr.y - bl.y)};

            plan.cells.push_back(cell);
        }
    }

    valid_ = !plan.cells.empty();
    if (!valid_)
        lastError_ = "fov grid plan empty";
    return plan;
}

} // namespace aoi::fov_planner
