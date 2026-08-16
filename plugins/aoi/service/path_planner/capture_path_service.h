#pragma once

/**
 * @file capture_path_service.h
 * @brief 采集路径规划对外入口：FOV 网格 → 路径 → 运动任务
 */

#include "capture_motion_binding.h"
#include "capture_path_planner.h"
#include "service/fov_planner/fov_grid_planner.h"

namespace aoi::path_planner {

/** CapturePathService::planFromGrid 返回值 */
struct CapturePathServiceResult
{
    bool              ok = false;
    std::string       error;
    CapturePathPlan   path;   ///< 几何层路径（无轴名）
    CaptureMotionPlan motion; ///< 绑定轴名后的运动规划
};

/**
 * @brief 一站式采集路径服务
 *
 * 默认 PointToPoint（逐点定位采图）；LinearScanTrigger 为双轴插补 + 位置比较触发。
 */
class CapturePathService
{
public:
    static CapturePathServiceResult planFromGrid(
        const fov_planner::FovGridPlan &gridPlan,
        int pathModeIndex,
        const std::string &axisXName,
        const std::string &axisYName,
        double speed = 0.0,
        int timeoutMs = 30000,
        CaptureExecutionMode executionMode = CaptureExecutionMode::PointToPoint);
};

} // namespace aoi::path_planner
