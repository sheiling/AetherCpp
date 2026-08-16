#pragma once

/**
 * @file capture_motion_binding.h
 * @brief 将 CapturePathPlan 绑定运控轴名，生成 CaptureMotionPlan（仍不发起运动）
 */

#include "capture_path_types.h"

#include <string>

namespace aoi::path_planner {

/**
 * @brief 为抽象路径计划填充轴名、速度、超时，供后续运控网关执行
 *
 * @param path       几何路径计划
 * @param axisXName  X 轴名称（sysConfig.common.axisXName）
 * @param axisYName  Y 轴名称
 * @param speed      插补/定位速度
 * @param timeoutMs  单任务超时 (ms)
 */
CaptureMotionPlan bindCaptureMotionPlan(const CapturePathPlan &path,
                                        const std::string &axisXName,
                                        const std::string &axisYName,
                                        double speed = 0.0,
                                        int timeoutMs = 30000);

} // namespace aoi::path_planner
