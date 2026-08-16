/**
 * @file capture_path_service.cpp
 * @brief CapturePathService 一站式入口
 */

#include "capture_path_service.h"

namespace aoi::path_planner {

CapturePathServiceResult CapturePathService::planFromGrid(
    const fov_planner::FovGridPlan &gridPlan,
    const int pathModeIndex,
    const std::string &axisXName,
    const std::string &axisYName,
    const double speed,
    const int timeoutMs,
    const CaptureExecutionMode executionMode)
{
    CapturePathServiceResult result;

    const FovVisitPattern pattern = fovVisitPatternFromRecipeIndex(pathModeIndex);

    CapturePathPlanner planner;
    const CapturePathGenerateResult pathResult =
        planner.plan(gridPlan, pattern, executionMode);
    if (!pathResult.ok) {
        result.error = pathResult.error;
        return result;
    }

    result.path   = pathResult.path;
    result.motion = bindCaptureMotionPlan(pathResult.path, axisXName, axisYName, speed, timeoutMs);
    result.ok     = true;
    return result;
}

} // namespace aoi::path_planner
