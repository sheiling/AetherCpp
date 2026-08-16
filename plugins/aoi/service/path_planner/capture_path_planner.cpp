/**
 * @file capture_path_planner.cpp
 * @brief CapturePathPlanner：访问顺序 + 逐点/条带路径填充
 */

#include "capture_path_planner.h"

#include "fov_visit_order.h"

#include <algorithm>

namespace aoi::path_planner {

namespace {

std::vector<const fov_planner::FovGridCell *>
sortedRowCells(const fov_planner::FovGridPlan &gridPlan, int row)
{
    std::vector<const fov_planner::FovGridCell *> rowCells;
    for (const fov_planner::FovGridCell &cell : gridPlan.cells) {
        if (cell.gridRow == row)
            rowCells.push_back(&cell);
    }
    std::sort(rowCells.begin(), rowCells.end(),
              [](const fov_planner::FovGridCell *a, const fov_planner::FovGridCell *b) {
                  return a->gridCol < b->gridCol;
              });
    return rowCells;
}

} // namespace

void CapturePathPlanner::fillPointWaypoints(
    CapturePathPlan &plan,
    const std::vector<const fov_planner::FovGridCell *> &sequence) const
{
    plan.waypoints.clear();
    plan.visitOrder.clear();
    plan.waypoints.reserve(sequence.size());
    plan.visitOrder.reserve(sequence.size());

    for (const fov_planner::FovGridCell *cell : sequence) {
        if (!cell)
            continue;
        CapturePathWaypoint wp;
        wp.fovIndex = cell->index;
        wp.gridRow  = cell->gridRow;
        wp.gridCol  = cell->gridCol;
        wp.xMm      = cell->centerPhysical.x;
        wp.yMm      = cell->centerPhysical.y;
        plan.waypoints.push_back(wp);
        plan.visitOrder.push_back(cell->index);
    }
}

void CapturePathPlanner::fillLinearScanStrips(CapturePathPlan &plan,
                                              const fov_planner::FovGridPlan &gridPlan,
                                              const FovVisitPattern visitPattern) const
{
    plan.scanStrips.clear();
    plan.visitOrder.clear();

    const int rows = gridPlan.rowCount;
    const int cols = gridPlan.colCount;

    if (visitPattern == FovVisitPattern::ColumnMajor) {
        // 逐列：固定 X，沿 Y 插补，触发点为各 FOV 中心 Y
        for (int c = 0; c < cols; ++c) {
            CaptureLinearScanStrip strip;
            strip.scanAxis = ScanAxis::Y;

            std::vector<const fov_planner::FovGridCell *> colCells;
            for (const fov_planner::FovGridCell &cell : gridPlan.cells) {
                if (cell.gridCol == c)
                    colCells.push_back(&cell);
            }
            std::sort(colCells.begin(), colCells.end(),
                      [](const fov_planner::FovGridCell *a, const fov_planner::FovGridCell *b) {
                          return a->gridRow < b->gridRow;
                      });
            if (colCells.empty())
                continue;

            strip.fixedCoordMm = colCells.front()->centerPhysical.x;
            strip.startPosMm   = colCells.front()->centerPhysical.y;
            strip.endPosMm     = colCells.back()->centerPhysical.y;

            for (const fov_planner::FovGridCell *cell : colCells) {
                strip.triggerPositionsMm.push_back(cell->centerPhysical.y);
                strip.fovIndices.push_back(cell->index);
                plan.visitOrder.push_back(cell->index);
            }
            plan.scanStrips.push_back(strip);
        }
        return;
    }

    // 逐行 / S / Z：固定 Y，沿 X 插补
    for (int r = 0; r < rows; ++r) {
        auto rowCells = sortedRowCells(gridPlan, r);
        if (rowCells.empty())
            continue;

        const bool reverse =
            (visitPattern == FovVisitPattern::Serpentine) && (r % 2 == 1);
        if (reverse)
            std::reverse(rowCells.begin(), rowCells.end());

        CaptureLinearScanStrip strip;
        strip.scanAxis     = ScanAxis::X;
        strip.fixedCoordMm = rowCells.front()->centerPhysical.y;
        strip.startPosMm   = reverse ? rowCells.back()->centerPhysical.x
                                     : rowCells.front()->centerPhysical.x;
        strip.endPosMm     = reverse ? rowCells.front()->centerPhysical.x
                                     : rowCells.back()->centerPhysical.x;

        for (const fov_planner::FovGridCell *cell : rowCells) {
            strip.triggerPositionsMm.push_back(cell->centerPhysical.x);
            strip.fovIndices.push_back(cell->index);
            plan.visitOrder.push_back(cell->index);
        }
        plan.scanStrips.push_back(strip);
    }
}

CapturePathGenerateResult CapturePathPlanner::plan(const fov_planner::FovGridPlan &gridPlan,
                                                 const FovVisitPattern visitPattern,
                                                 const CaptureExecutionMode executionMode) const
{
    CapturePathGenerateResult result;
    valid_      = false;
    lastError_.clear();

    if (gridPlan.cells.empty()) {
        lastError_     = "empty fov grid plan";
        result.error   = lastError_;
        return result;
    }

    result.path.visitPattern   = visitPattern;
    result.path.executionMode = executionMode;

    const auto sequence = buildFovVisitSequence(gridPlan, visitPattern);

    if (executionMode == CaptureExecutionMode::PointToPoint) {
        fillPointWaypoints(result.path, sequence);
    } else {
        fillLinearScanStrips(result.path, gridPlan, visitPattern);
    }

    valid_ = !result.path.visitOrder.empty();
    if (!valid_) {
        lastError_   = "capture path empty";
        result.error = lastError_;
        return result;
    }

    result.ok   = true;
    return result;
}

} // namespace aoi::path_planner
