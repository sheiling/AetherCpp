#include "aoi_inspection_steps.h"

#include "data/dataModel/recipe_config_model.h"

#include "data/data.h"
#include "data/dataModel/system_config_model.h"
#include "param_define.h"
#include "service/capture/capture_motion_gateway.h"
#include "service/capture/capture_profile.h"
#include "service/inspection/session/inspection_step_ops.h"
#include "service/marker/inspection_marker_service.h"
#include "inspection_step_utils.h"

#include <scheduler/step/step_factory.h>

#include <future>

namespace aoi::step {

using aoi::capture::CaptureMotionGateway;
using aoi::capture::buildCaptureProfile;
using aoi::inspection::reportSessionLog;
using aoi::inspection::sessionFrom;

scheduler::StepResult AoiLoadRecipeStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    RecipeRepository::setStorageDirectory(data::recipeStorageDir());
    RecipeRepository::load();

    const QString recipeId = RecipeRepository::defaultRecipeId();
    if (recipeId.isEmpty() || !RecipeRepository::load(recipeId, session->recipe)) {
        reportSessionLog(session, QStringLiteral("加载配方失败"), QStringLiteral("ERROR"));
        return scheduler::StepResult::fail("recipe load failed");
    }

    session->sysConfig = SystemConfigRepository::load();
    session->captureProfile = buildCaptureProfile(session->sysConfig, session->recipe);

    const auto ready = CaptureMotionGateway::checkReady(session->sysConfig.common);
    if (!ready.ok) {
        reportSessionLog(session, ready.message, QStringLiteral("ERROR"));
        return scheduler::StepResult::fail(ready.message.toStdString());
    }

    session->progressMessage =
        QStringLiteral("配方已加载: %1").arg(session->recipe.basic.name);
    reportSessionLog(session, session->progressMessage);
    ctx.set(aoi::flow::kCtxRecipeId, recipeId.toStdString());

    session->recipe.basic.name = session->recipe.basic.name.isEmpty()
                                     ? recipeId
                                     : session->recipe.basic.name;
    return scheduler::StepResult::ok();
}

scheduler::StepResult AoiMarkLocateStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    const int markTotal = aoi::recipeMarkCount(session->sysConfig.common.trackMode);
    return aoi::inspection::runMarkerLocateSlots(session, ctx, 0, markTotal);
}

scheduler::StepResult AoiPlanGeometryStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    return aoi::inspection::runPlanGeometry(session, ctx);
}

scheduler::StepResult AoiFovCaptureStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    return aoi::inspection::runFovCapture(session, ctx);
}

scheduler::StepResult AoiFovDetectStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    return aoi::inspection::runFovDetect(session, ctx);
}

scheduler::StepResult AoiAggregateResultsStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    return aoi::inspection::runAggregateResults(session, ctx);
}

scheduler::StepResult AoiTrackPipelineStep::execute(scheduler::FlowContext &ctx)
{
    auto session = sessionFrom(ctx);
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    const aoi::TrackMode mode = session->sysConfig.common.trackMode;

    if (mode == aoi::TrackMode::SingleTrack) {
        return aoi::inspection::runSingleTrackPipeline(session, ctx);
    }

    // 双轨流水线：轨1 Mark+采图 → 异步规划；轴去轨2 Mark+采图；汇合后轨2 规划与 FOV
    reportSessionLog(session, QStringLiteral("双轨模式：开始轨1 Mark 定位与采图"));
    session->activeTrack = 0;

    auto result = aoi::inspection::runMarkerLocateSlots(session, ctx, 0, 2);
    if (!result.isOk())
        return result;

    reportSessionLog(session, QStringLiteral("轨1 Mark 完成，后台启动路径规划，轴前往轨2"));

    std::shared_ptr<aoi::inspection::InspectionSession> sessionPtr = session;
    auto planFuture = std::async(std::launch::async, [sessionPtr]() {
        scheduler::FlowContext dummyCtx;
        return aoi::inspection::runPlanGeometry(sessionPtr, dummyCtx, 0);
    });

    session->activeTrack = 1;
    reportSessionLog(session, QStringLiteral("轨2 Mark 定位与采图"));
    result = aoi::inspection::runMarkerLocateSlots(session, ctx, 2, 2);
    if (!result.isOk()) {
        planFuture.wait();
        return result;
    }

    const auto track1Plan = planFuture.get();
    if (!track1Plan.isOk()) {
        reportSessionLog(session,
                         QString::fromStdString(track1Plan.message),
                         QStringLiteral("ERROR"));
        return track1Plan;
    }
    reportSessionLog(session, QStringLiteral("轨1 规划已完成，继续轨2 规划与 FOV 采图"));

    result = aoi::inspection::runPlanGeometry(session, ctx, 1);
    if (!result.isOk())
        return result;

    result = aoi::inspection::runFovCapture(session, ctx);
    if (!result.isOk())
        return result;

    result = aoi::inspection::runFovDetect(session, ctx);
    if (!result.isOk())
        return result;

    return aoi::inspection::runAggregateResults(session, ctx);
}

SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiLoadRecipeStep, "aoi.load_recipe")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiMarkLocateStep, "aoi.mark_locate")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiPlanGeometryStep, "aoi.plan_geometry")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiFovCaptureStep, "aoi.fov_capture")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiFovDetectStep, "aoi.fov_detect")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiAggregateResultsStep, "aoi.aggregate_results")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiTrackPipelineStep, "aoi.track_pipeline")

} // namespace aoi::step
