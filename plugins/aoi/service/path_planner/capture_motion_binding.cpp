/**
 * @file capture_motion_binding.cpp
 * @brief 抽象 CapturePathPlan 绑定运控轴名 → CaptureMotionPlan
 */

#include "capture_motion_binding.h"

namespace aoi::path_planner {

CaptureMotionPlan bindCaptureMotionPlan(const CapturePathPlan &path,
                                        const std::string &axisXName,
                                        const std::string &axisYName,
                                        const double speed,
                                        const int timeoutMs)
{
    CaptureMotionPlan motion;
    motion.executionMode   = path.executionMode;
    motion.imageIndexOrder = path.visitOrder;

    if (path.executionMode == CaptureExecutionMode::PointToPoint) {
        motion.pointTasks.reserve(path.waypoints.size());
        for (const CapturePathWaypoint &wp : path.waypoints) {
            CapturePointMotionTask task;
            task.axisXName = axisXName;
            task.axisYName = axisYName;
            task.xMm       = wp.xMm;
            task.yMm       = wp.yMm;
            task.fovIndex  = wp.fovIndex;
            task.speed     = speed;
            task.timeoutMs = timeoutMs;
            motion.pointTasks.push_back(task);
        }
        return motion;
    }

    motion.linearTasks.reserve(path.scanStrips.size());
    for (const CaptureLinearScanStrip &strip : path.scanStrips) {
        CaptureLinearMotionTask task;
        task.axisXName          = axisXName;
        task.axisYName          = axisYName;
        task.speed              = speed;
        task.timeoutMs          = timeoutMs;
        task.comparePositionsMm = strip.triggerPositionsMm;
        task.fovIndices         = strip.fovIndices;

        if (strip.scanAxis == ScanAxis::X) {
            task.compareOnX = true;
            task.startYmm   = strip.fixedCoordMm;
            task.endYmm     = strip.fixedCoordMm;
            task.startXmm   = strip.startPosMm;
            task.endXmm     = strip.endPosMm;
        } else {
            task.compareOnX = false;
            task.startXmm   = strip.fixedCoordMm;
            task.endXmm     = strip.fixedCoordMm;
            task.startYmm   = strip.startPosMm;
            task.endYmm     = strip.endPosMm;
        }
        motion.linearTasks.push_back(task);
    }
    return motion;
}

} // namespace aoi::path_planner
