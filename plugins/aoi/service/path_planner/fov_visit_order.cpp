/**
 * @file fov_visit_order.cpp
 * @brief FOV 网格访问顺序：S 型 / Z 型 / 逐行 / 逐列
 */

#include "fov_visit_order.h"

#include <algorithm>

namespace aoi::path_planner {

FovVisitPattern fovVisitPatternFromRecipeIndex(const int pathModeIndex)
{
    switch (pathModeIndex) {
    case 1:
        return FovVisitPattern::RowZPattern;
    case 2:
        return FovVisitPattern::RowMajor;
    case 3:
        return FovVisitPattern::ColumnMajor;
    case 0:
    default:
        return FovVisitPattern::Serpentine;
    }
}

namespace {

std::vector<const fov_planner::FovGridCell *>
collectRowCells(const fov_planner::FovGridPlan &gridPlan, int row)
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

std::vector<const fov_planner::FovGridCell *>
collectColCells(const fov_planner::FovGridPlan &gridPlan, int col)
{
    std::vector<const fov_planner::FovGridCell *> colCells;
    for (const fov_planner::FovGridCell &cell : gridPlan.cells) {
        if (cell.gridCol == col)
            colCells.push_back(&cell);
    }
    std::sort(colCells.begin(), colCells.end(),
              [](const fov_planner::FovGridCell *a, const fov_planner::FovGridCell *b) {
                  return a->gridRow < b->gridRow;
              });
    return colCells;
}

} // namespace

std::vector<const fov_planner::FovGridCell *>
buildFovVisitSequence(const fov_planner::FovGridPlan &gridPlan, const FovVisitPattern pattern)
{
    std::vector<const fov_planner::FovGridCell *> sequence;
    if (gridPlan.cells.empty())
        return sequence;

    const int rows = gridPlan.rowCount;
    const int cols = gridPlan.colCount;

    if (pattern == FovVisitPattern::ColumnMajor) {
        for (int c = 0; c < cols; ++c) {
            auto colCells = collectColCells(gridPlan, c);
            for (const fov_planner::FovGridCell *cell : colCells)
                sequence.push_back(cell);
        }
        return sequence;
    }

    for (int r = 0; r < rows; ++r) {
        auto rowCells = collectRowCells(gridPlan, r);
        if (rowCells.empty())
            continue;

        // S 型：奇数行 col 降序；Z/逐行：始终升序
        const bool reverse =
            (pattern == FovVisitPattern::Serpentine) && (r % 2 == 1);
        if (reverse)
            std::reverse(rowCells.begin(), rowCells.end());

        for (const fov_planner::FovGridCell *cell : rowCells)
            sequence.push_back(cell);
    }
    return sequence;
}

} // namespace aoi::path_planner
