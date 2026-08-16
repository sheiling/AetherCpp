#pragma once

#include "inspection_session.h"
#include "inspection_types.h"

#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

namespace aoi::inspection {

/** FOV 路径规划（仿射 + FOV 网格 + 运动路径） */
scheduler::StepResult runPlanGeometry(const std::shared_ptr<InspectionSession> &session,
                                      scheduler::FlowContext &ctx,
                                      int trackIndex = -1);

/** FOV 逐点采图（含 Z/光源/曝光），虚拟 FOV 进度上报 UI */
scheduler::StepResult runFovCapture(const std::shared_ptr<InspectionSession> &session,
                                    scheduler::FlowContext &ctx);

/** 对已采 FOV 异步缺陷检测并等待完成 */
scheduler::StepResult runFovDetect(const std::shared_ptr<InspectionSession> &session,
                                   scheduler::FlowContext &ctx);

/** 汇总板级结果并上报 UI 快照 */
scheduler::StepResult runAggregateResults(const std::shared_ptr<InspectionSession> &session,
                                          scheduler::FlowContext &ctx);

/** 单轨完整链路：Marker 定位 → 规划 → 采图+异步检测 → 汇总 */
scheduler::StepResult runSingleTrackPipeline(const std::shared_ptr<InspectionSession> &session,
                                             scheduler::FlowContext &ctx);

} // namespace aoi::inspection
