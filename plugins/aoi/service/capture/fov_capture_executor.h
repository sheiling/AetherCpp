#pragma once

/**
 * @file fov_capture_executor.h
 * @brief FOV 图像采集工步：按路径规划逐格运动采图，可选采图阶段提交异步检测
 *
 * 流程位置（扫描链）：
 *   Marker → FOV 规划 → 路径规划 → **本模块** → 异步检测 → 结果汇总
 *
 * 写入 InspectionSession：fovCaptures、lastFovPreview、capturedImageCount、fovDetects（可选）。
 */

#include "service/inspection/session/inspection_session.h"

#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

namespace aoi::capture {

/**
 * @brief FOV 逐点采图执行器
 */
class FovCaptureExecutor
{
public:
    /**
     * @brief 按 session->fovGeometryPlan 逐格运动采图
     * @param session            单板扫描会话（含规划结果与 captureProfile）
     * @param ctx                工步上下文（写入 kCtxCurrentImageIndex 等）
     * @param submitAsyncDetect  true 时每帧采图后立即提交 detectAsync（不阻塞采图；结果在 aoi.fov_detect 收集）
     */
    static scheduler::StepResult execute(const std::shared_ptr<inspection::InspectionSession> &session,
                                         scheduler::FlowContext &ctx,
                                         bool submitAsyncDetect = true);
};

} // namespace aoi::capture
