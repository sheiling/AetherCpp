#include "inspection_step_ops.h"

#include "service/capture/fov_capture_executor.h"
#include "inspection_detect_service.h"
#include "inspection_result_service.h"
#include "inspection_step_utils.h"
#include "service/fov_planner/fov_plan_service.h"
#include "service/marker/inspection_marker_service.h"
#include "service/path_planner/capture_path_service.h"
#include "param_define.h"

#include <QRectF>

#include <future>

namespace aoi::inspection {

namespace {
using aoi::fov_planner::FovPlanGenerateResult;
using aoi::path_planner::CapturePathService;
using aoi::path_planner::CapturePathServiceResult;
} // namespace

scheduler::StepResult runPlanGeometry(const std::shared_ptr<InspectionSession> &session,
                                      scheduler::FlowContext &ctx,
                                      int trackIndex)
{
    Q_UNUSED(trackIndex);

    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    const FovPlanGenerateResult planResult = fov_planner::FovPlanService::generate(*session);
    if (!planResult.ok) {
        reportSessionLog(session, planResult.message, QStringLiteral("ERROR"));
        return scheduler::StepResult::fail(planResult.message.toStdString());
    }

    session->fovGeometryPlan = planResult.plan;
    session->totalImageCount = static_cast<int>(planResult.plan.grid.cells.size());

    const auto &common = session->sysConfig.common;
    const CapturePathServiceResult pathResult = CapturePathService::planFromGrid(
        planResult.plan.grid, session->recipe.panelCalib.pathModeIndex,
        common.axisXName.toStdString(), common.axisYName.toStdString(), common.motionSpeed,
        common.motionTimeoutMs);

    if (!pathResult.ok) {
        reportSessionLog(session, QString::fromStdString(pathResult.error),
                         QStringLiteral("ERROR"));
        return scheduler::StepResult::fail(pathResult.error);
    }

    session->captureMotionPlan = pathResult.motion;

    session->recipe.panelCalib.fovInfo =
        fov_planner::FovPlanService::formatSummary(planResult.plan);
    session->progressMessage = session->recipe.panelCalib.fovInfo;
    reportSessionLog(session, session->progressMessage);
    reportSessionProgress(session);

    if (session->progressBridge) {
        const int rows       = planResult.plan.grid.rowCount;
        const int cols       = planResult.plan.grid.colCount;
        const bool dualTrack = session->sysConfig.common.trackMode == TrackMode::DualTrack;
        const int  track     = session->activeTrack > 0 ? session->activeTrack : 1;
        session->progressBridge->reportVirtualFov(session->progressMessage, 0,
                                                  session->totalImageCount, {}, QRectF(),
                                                  session->recipe.basic.width,
                                                  session->recipe.basic.height, rows, cols, -1, -1,
                                                  track, dualTrack);
    }
    return scheduler::StepResult::ok();
}

scheduler::StepResult runFovCapture(const std::shared_ptr<InspectionSession> &session,
                                    scheduler::FlowContext &ctx)
{
    return capture::FovCaptureExecutor::execute(session, ctx, true);
}

scheduler::StepResult runFovDetect(const std::shared_ptr<InspectionSession> &session,
                                    scheduler::FlowContext &ctx)
{
    if (!session->pendingDetectFutures.empty())
        return runFovDetectService(session, ctx);
    if (!session->fovDetects.isEmpty()
        && session->detectedImageCount >= session->fovCaptures.size())
        return scheduler::StepResult::ok();
    return runFovDetectService(session, ctx);
}

scheduler::StepResult runAggregateResults(const std::shared_ptr<InspectionSession> &session,
                                          scheduler::FlowContext &ctx)
{
    return runAggregateResultsService(session, ctx);
}

scheduler::StepResult runSingleTrackPipeline(const std::shared_ptr<InspectionSession> &session,
                                             scheduler::FlowContext &ctx)
{
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    const int markTotal = recipeMarkCount(session->sysConfig.common.trackMode);
    auto      result    = runMarkerLocateSlots(session, ctx, 0, markTotal);
    if (!result.isOk())
        return result;

    result = runPlanGeometry(session, ctx);
    if (!result.isOk())
        return result;

    result = runFovCapture(session, ctx);
    if (!result.isOk())
        return result;

    result = runFovDetect(session, ctx);
    if (!result.isOk())
        return result;

    return runAggregateResults(session, ctx);
}

} // namespace aoi::inspection
