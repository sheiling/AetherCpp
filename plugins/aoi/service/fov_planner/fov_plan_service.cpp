/**
 * @file fov_plan_service.cpp
 * @brief FovPlanService 对外入口实现
 */

#include "fov_plan_service.h"

#include "fov_geometry_planner.h"
#include "inspection_session.h"
#include "service/marker/inspection_marker_types.h"

#include <Logger.h>

#include <QtGlobal>

namespace aoi::fov_planner {
namespace {

constexpr int kMaxGroupGridDim = 512;
constexpr int kMaxGroupGridCells = 200000;
constexpr int kMaxFovGridCells = 10000;

/**
 * @brief 规划成功后按 FOV 网格行列输出各格物理中心坐标（debug 级）
 * @param grid 已生成的 FOV 网格（cells 行优先，与 gridRow/gridCol 一致）
 */
void logFovGridCoordinates(const FovGridPlan &grid)
{
    const int rows = grid.rowCount;
    const int cols = grid.colCount;
    LOG_DEBUG("[FovPlanService] FOV规划坐标: {}行×{}列, 共{}张", rows, cols,
              static_cast<int>(grid.cells.size()));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const FovGridCell *cell = nullptr;
            for (const FovGridCell &candidate : grid.cells) {
                if (candidate.gridRow == r && candidate.gridCol == c) {
                    cell = &candidate;
                    break;
                }
            }
            if (!cell) {
                LOG_DEBUG("[FovPlanService]   [R{} C{}] (缺失)", r, c);
                continue;
            }
            LOG_DEBUG("[FovPlanService]   [R{} C{}] idx={} physical=({:.3f}, {:.3f}) mm logical=({:.3f}, {:.3f}) mm",
                      r, c, cell->index, cell->centerPhysical.x, cell->centerPhysical.y,
                      cell->centerLogical.x, cell->centerLogical.y);
        }
    }
}

} // namespace

FovPlanGenerateResult FovPlanService::generate(const aoi::RecipeDocument &recipe,
                                               const MarkerAnchorInput &markers)
{
    FovPlanGenerateResult result;
    const FovLayoutConfig layout = buildFovLayoutConfig(recipe, markers);

    if (layout.groupRowCount <= 0 || layout.groupColCount <= 0) {
        result.message = QStringLiteral("aoi.recipe.calib.fov_invalid_group_count");
        return result;
    }
    if (layout.groupRowCount > kMaxGroupGridDim || layout.groupColCount > kMaxGroupGridDim) {
        result.message = QStringLiteral("aoi.recipe.calib.fov_group_count_exceeded");
        return result;
    }
    const qint64 groupCells =
        static_cast<qint64>(layout.groupRowCount) * static_cast<qint64>(layout.groupColCount);
    if (groupCells > kMaxGroupGridCells) {
        result.message = QStringLiteral("aoi.recipe.calib.fov_group_count_exceeded");
        return result;
    }

    if (layout.physicalMarkerBottomLeft.x == 0.0 && layout.physicalMarkerBottomLeft.y == 0.0
        && layout.physicalMarkerTopRight.x == 0.0 && layout.physicalMarkerTopRight.y == 0.0) {
        result.message = QStringLiteral("aoi.recipe.calib.fov_markers_required");
        return result;
    }

    const double mmPerPixel = captureMmPerPixel(recipe);
    const int imageWidth  = recipe.camera.imageWidth > 0 ? recipe.camera.imageWidth : 1920;
    const int imageHeight = recipe.camera.imageHeight > 0 ? recipe.camera.imageHeight : 1080;

    FovGeometryPlanner planner;
    result.plan = planner.plan(layout, mmPerPixel, imageWidth, imageHeight);
    if (!result.plan.isValid()) {
        const QString err = QString::fromStdString(result.plan.lastError());
        if (err.contains(QStringLiteral("fov grid cell count exceeded"), Qt::CaseInsensitive))
            result.message = QStringLiteral("aoi.recipe.calib.fov_grid_too_large");
        else
            result.message = err;
        return result;
    }

    result.ok      = true;
    result.message = formatSummary(result.plan);
    logFovGridCoordinates(result.plan.grid);
    return result;
}

FovPlanGenerateResult FovPlanService::generate(const aoi::inspection::InspectionSession &session)
{
    const MarkerAnchorInput markers = markerAnchorFromSession(
        session.markerLocatedPhysical, session.markerLocatedValid);
    return generate(session.recipe, markers);
}

QString FovPlanService::formatSummary(const FovGeometryPlan &plan)
{
    if (!plan.isValid())
        return QStringLiteral("aoi.recipe.calib.fov_not_generated");

    return QStringLiteral("FOV %1×%2，共 %3 张")
        .arg(plan.grid.colCount)
        .arg(plan.grid.rowCount)
        .arg(plan.grid.cells.size());
}

} // namespace aoi::fov_planner
