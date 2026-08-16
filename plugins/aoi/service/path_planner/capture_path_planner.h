#pragma once

/**
 * @file capture_path_planner.h
 * @brief 由 FOV 网格与访问/执行模式生成 CapturePathPlan
 */

#include "capture_path_types.h"
#include "service/fov_planner/fov_grid_planner.h"

namespace aoi::path_planner {

/**
 * @brief 采集路径规划器
 *
 * 仅做几何排序与路径数据结构填充，不绑定轴名、不调用运控。
 */
class CapturePathPlanner
{
public:
    /**
     * @param gridPlan      FOV 规划输出的网格
     * @param visitPattern  S/Z/逐行/逐列
     * @param executionMode 逐点（默认）或双轴插补硬件触发
     */
    CapturePathGenerateResult plan(const fov_planner::FovGridPlan &gridPlan,
                                   FovVisitPattern visitPattern,
                                   CaptureExecutionMode executionMode =
                                       CaptureExecutionMode::PointToPoint) const;

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

private:
    void fillPointWaypoints(CapturePathPlan &plan,
                            const std::vector<const fov_planner::FovGridCell *> &sequence) const;

    void fillLinearScanStrips(CapturePathPlan &plan,
                              const fov_planner::FovGridPlan &gridPlan,
                              FovVisitPattern visitPattern) const;

    mutable bool        valid_ = false;
    mutable std::string lastError_;
};

} // namespace aoi::path_planner
