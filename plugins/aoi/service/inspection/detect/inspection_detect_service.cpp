#include "service/inspection/detect/inspection_detect_service.h"

#include "service/flow/scan_flow_pause.h"
#include "inspection_step_utils.h"
#include "service/vision_bridge/chip_detect_bridge.h"

#include "data/dataModel/recipe_config_model.h"

#include <vision_algo/core/service_access.h>
#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <Logger.h>

#include <future>
#include <condition_variable>
#include <mutex>

namespace aoi::inspection {

namespace {

std::mutex              g_detectAsyncMutex;
std::condition_variable g_detectAsyncCv;
int                     g_detectAsyncInFlight = 0;

struct DetectAsyncSlotGuard
{
    ~DetectAsyncSlotGuard()
    {
        {
            std::lock_guard<std::mutex> lock(g_detectAsyncMutex);
            --g_detectAsyncInFlight;
        }
        g_detectAsyncCv.notify_one();
    }
};

void acquireDetectAsyncSlot()
{
    std::unique_lock<std::mutex> lock(g_detectAsyncMutex);
    g_detectAsyncCv.wait(lock, [] {
        return g_detectAsyncInFlight
               < static_cast<int>(vision_algo::kDefaultDieBondingPoolSize);
    });
    ++g_detectAsyncInFlight;
}

scheduler::StepResult collectPendingDetectResults(const std::shared_ptr<InspectionSession> &session)
{
    session->fovDetects.clear();
    session->fovDetects.resize(static_cast<int>(session->pendingDetectFutures.size()));
    session->detectedImageCount = 0;

    for (std::size_t i = 0; i < session->pendingDetectFutures.size(); ++i) {
        if (session->isAbortRequested())
            return scheduler::StepResult::fail("aborted");
        if (!flow::waitWhileMachinePaused(session))
            return scheduler::StepResult::fail("aborted");

        FovDetectRecord record = session->pendingDetectFutures[i].get();
        session->fovDetects[static_cast<int>(i)] = record;
        session->detectedImageCount              = static_cast<int>(i) + 1;

        session->progressMessage =
            QStringLiteral("FOV %1 检测完成: %2")
                .arg(record.fovIndex)
                .arg(record.ok ? QStringLiteral("OK") : QStringLiteral("NG"));
        reportSessionLog(session, session->progressMessage,
                         record.ok ? QStringLiteral("INFO") : QStringLiteral("WARN"));
    }

    session->pendingDetectFutures.clear();
    return scheduler::StepResult::ok();
}

bool isValidRgbProcessImage(const std::shared_ptr<common::device::ImageInfo> &info)
{
    return info && info->imageMat && info->imageMat->data && info->imageMat->cols > 0
           && info->imageMat->rows > 0;
}

} // namespace

InspectionDetectEngine::InspectionDetectEngine() = default;

InspectionDetectEngine::~InspectionDetectEngine() = default;

InspectionDetectService &InspectionDetectService::instance()
{
    static InspectionDetectService inst;
    return inst;
}

void InspectionDetectService::warmUp()
{
    try {
        if (auto *va = vision_algo::service(); va && !va->isReady()) {
            LOG_WARN("[InspectionDetect] 预热跳过：VisionAlgo 服务未就绪");
            return;
        }
        RecipeDocument recipe;
        if (!RecipeRepository::load(RecipeRepository::defaultRecipeId(), recipe)) {
            LOG_WARN("[InspectionDetect] 预热跳过：默认配方未加载");
            return;
        }
        QString err;
        if (ensureInitialized(recipe, &err))
            LOG_INFO("[InspectionDetect] 预热完成，引擎就绪 {}", engine_.isReady());
        else
            LOG_WARN("[InspectionDetect] 预热失败：{}", err.toStdString());
    } catch (const std::exception &ex) {
        LOG_WARN("[InspectionDetect] 预热异常：{}", ex.what());
    } catch (...) {
        LOG_WARN("[InspectionDetect] 预热发生未知异常");
    }
}

bool InspectionDetectService::ensureInitialized(const RecipeDocument &recipe, QString *errorOut)
{
    return engine_.initialize(recipe, errorOut);
}

bool InspectionDetectEngine::initialize(const RecipeDocument &recipe, QString *errorOut,
                                        size_t poolSize, int dlInferenceEngineOverride)
{
    vision_algo::DieBondingChipDetectParam newParam;
    if (!buildDieBondingDetectParam(recipe, &newParam, errorOut))
        return false;

    if (dlInferenceEngineOverride >= 0)
        newParam.dlInferenceEngine = dlInferenceEngineOverride;

    logDieBondingDetectParam("InspectionDetectEngine::initialize", newParam,
                             recipe.camera.captureAccuracy, &recipe, 0, 0);

    if (ready_ && dieBondingDetectParamsEqual(param_, newParam))
        return true;

    param_ = newParam;
    ready_ = initDieBondingEngine(param_, poolSize, dlInferenceEngineOverride, errorOut);
    LOG_INFO("[FovDetect] shared engine ready={} poolSize={}", ready_, poolSize);
    return ready_;
}

bool InspectionDetectEngine::syncDetectParam(const RecipeDocument &recipe, QString *errorOut)
{
    if (!ready_) {
        if (errorOut)
            *errorOut = QStringLiteral("检测引擎未初始化");
        return false;
    }

    vision_algo::DieBondingChipDetectParam newParam;
    if (!buildDieBondingDetectParam(recipe, &newParam, errorOut))
        return false;

    newParam.dlInferenceEngine = param_.dlInferenceEngine;
    if (dieBondingDetectParamsEqual(param_, newParam))
        return true;

    LOG_INFO("[FovDetect] syncDetectParam chip={}x{} group={}x{} umPerPx={}", newParam.chipWidth,
             newParam.chipHeight, newParam.chipGroupWidth, newParam.chipGroupHeight,
             recipe.camera.captureAccuracy);
    if (!syncDieBondingEngineParam(newParam, errorOut)) {
        LOG_ERROR("[FovDetect] engine syncParam failed");
        return false;
    }
    param_ = newParam;
    return true;
}

FovDetectRecord InspectionDetectEngine::detectFov(const FovDetectRequest &request)
{
    FovDetectRecord record;
    record.fovIndex = request.options.fovIndex;

    if (!ready_) {
        record.error = QStringLiteral("检测引擎未初始化");
        record.ok    = false;
        return record;
    }

    if (!isValidRgbProcessImage(request.processImage)) {
        record.error = QStringLiteral("FOV 处理图为空或无效");
        record.ok    = false;
        return record;
    }

    const char *modeTag = request.mode == FovDetectMode::Planned ? "planned" : "adhoc";
    const int   matW    = request.processImage->imageMat->cols;
    const int   matH    = request.processImage->imageMat->rows;
    LOG_INFO("[FovDetect] detectFov mode={} fovIndex={} grid=({},{}) dlCrop={} image={}x{}",
             modeTag, request.options.fovIndex, request.options.gridRow, request.options.gridCol,
             request.options.dlCropLocate, matW, matH);

    const ChipDetectOutcome outcome = detectFovProcessImage(
        request.recipe, request.processImage, request.options.fovIndex,
        request.options.useDeepLearning, request.options.dlCropLocate);

    LOG_INFO("[FovDetect] detectFov outcome ok={} err={}", outcome.ok,
             outcome.error.toStdString());

    if (!outcome.ok) {
        record.error = outcome.error;
        record.ok    = false;
        return record;
    }

    record.ok       = outcome.record.ok;
    record.groupNum = outcome.record.groupNum;
    record.groups   = outcome.record.groups;
    return record;
}

std::future<FovDetectRecord> InspectionDetectEngine::detectFovAsync(FovDetectRequest request)
{
    return std::async(std::launch::async, [this, request = std::move(request)]() {
        acquireDetectAsyncSlot();
        const DetectAsyncSlotGuard guard;
        return detectFov(request);
    });
}

FovDetectRecord InspectionDetectEngine::detectSync(
    std::shared_ptr<common::device::ImageInfo> processImage, int fovIndex,
    const RecipeDocument &recipe, bool dlCropLocate)
{
    auto req = makeAdHocFovRequest(std::move(processImage), recipe, fovIndex);
    req.options.dlCropLocate = dlCropLocate;
    return detectFov(req);
}

std::future<FovDetectRecord> InspectionDetectEngine::detectAsync(
    std::shared_ptr<common::device::ImageInfo> processImage, int fovIndex,
    const RecipeDocument &recipe)
{
    FovCaptureRecord cap;
    cap.fovIndex     = fovIndex;
    cap.processImage = std::move(processImage);
    return detectFovAsync(makePlannedFovRequest(cap, recipe));
}

scheduler::StepResult runFovDetectService(const std::shared_ptr<InspectionSession> &session,
                                          scheduler::FlowContext &ctx)
{
    Q_UNUSED(ctx);

    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    if (!session->pendingDetectFutures.empty()) {
        scheduler::StepResult result = collectPendingDetectResults(session);
        if (!result.isOk())
            return result;
        session->progressMessage =
            QStringLiteral("缺陷检测完成，共 %1 张 FOV").arg(session->detectedImageCount);
        reportSessionLog(session, session->progressMessage);
        return scheduler::StepResult::ok();
    }

    if (session->fovCaptures.isEmpty())
        return scheduler::StepResult::fail("no fov captures");

    QString initError;
    if (!InspectionDetectService::instance().ensureInitialized(session->recipe, &initError)) {
            reportSessionLog(session, initError, QStringLiteral("ERROR"));
            return scheduler::StepResult::fail(initError.toStdString());
    }

    auto &detectEngine = InspectionDetectService::instance().engine();

    session->fovDetects.clear();
    session->fovDetects.resize(session->fovCaptures.size());
    session->detectedImageCount = 0;

    std::vector<std::future<FovDetectRecord>> futures;
    futures.reserve(static_cast<size_t>(session->fovCaptures.size()));

    for (const FovCaptureRecord &cap : session->fovCaptures) {
        if (session->isAbortRequested())
            return scheduler::StepResult::fail("aborted");
        if (!flow::waitWhileMachinePaused(session))
            return scheduler::StepResult::fail("aborted");

        futures.push_back(
            detectEngine.detectFovAsync(makePlannedFovRequest(cap, session->recipe)));

        session->progressMessage = QStringLiteral("FOV %1/%2 已提交异步缺陷检测")
                                       .arg(cap.fovIndex)
                                       .arg(session->totalImageCount);
        reportSessionLog(session, session->progressMessage);
    }

    for (std::size_t i = 0; i < futures.size(); ++i) {
        if (session->isAbortRequested())
            return scheduler::StepResult::fail("aborted");
        if (!flow::waitWhileMachinePaused(session))
            return scheduler::StepResult::fail("aborted");

        FovDetectRecord record = futures[i].get();
        session->fovDetects[static_cast<int>(i)] = record;
        session->detectedImageCount              = static_cast<int>(i) + 1;

        session->progressMessage =
            QStringLiteral("FOV %1 检测完成: %2")
                .arg(record.fovIndex)
                .arg(record.ok ? QStringLiteral("OK") : QStringLiteral("NG"));
        reportSessionLog(session, session->progressMessage,
                         record.ok ? QStringLiteral("INFO") : QStringLiteral("WARN"));
    }

    session->progressMessage =
        QStringLiteral("缺陷检测完成，共 %1 张 FOV").arg(session->detectedImageCount);
    reportSessionLog(session, session->progressMessage);
    return scheduler::StepResult::ok();
}

} // namespace aoi::inspection
