/**
 * @file virtual_inspect_runner.cpp
 * @brief 配方页虚拟调测：AdHoc detectFov → buildSnapshot + reportReviewSession
 */

#include "virtual_inspect_runner.h"

#include "flow_host/flow_host_coordinator.h"
#include "fov_detect_request.h"
#include "service/inspection/detect/inspection_detect_service.h"
#include "inspection_result_service.h"

#include "data/dataModel/system_config_model.h"
#include "ui/image/image_info_display.h"

#include <Logger.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTemporaryFile>

#include <memory>

Q_DECLARE_METATYPE(aoi::CurrentInspectionSnapshot)

namespace aoi::inspection {

namespace {

constexpr int kAdHocFovIndex = 1;

} // namespace

bool makeTempResultImagePath(QString *outPath)
{
    if (!outPath)
        return false;
    auto temp = std::make_unique<QTemporaryFile>(
        QDir::tempPath() + QStringLiteral("/aoi_vi_result_XXXXXX.png"));
    temp->setAutoRemove(false);
    if (!temp->open())
        return false;
    *outPath = temp->fileName();
    temp->close();
    return true;
}

namespace {

QString ensureOriginImagePath(const QImage &frame, const QString &sourceImagePath)
{
    if (!sourceImagePath.isEmpty() && QFileInfo::exists(sourceImagePath))
        return QFileInfo(sourceImagePath).absoluteFilePath();

    QString tempPath;
    if (!makeTempResultImagePath(&tempPath))
        return {};
    if (!frame.save(tempPath, "PNG"))
        return {};
    return QFileInfo(tempPath).absoluteFilePath();
}

} // namespace

VirtualInspectRunner::VirtualInspectRunner(QObject *parent) : QObject(parent) {}

VirtualInspectRunner::~VirtualInspectRunner()
{
    cancel();
}

void VirtualInspectRunner::bumpProgress(int percent, const QString &stageKey)
{
    emit progressChanged(percent, stageKey);
}

void VirtualInspectRunner::start(const RecipeDocument &recipe, const QImage &frame,
                                 const QString &sourceImagePath)
{
    if (m_running)
        return;

    m_running         = true;
    m_cancelRequested = false;

    bumpProgress(0, QStringLiteral("aoi.recipe.inspect.stage.start"));

    if (flow_host::flowHostCoordinator().isProductionRunActive()) {
        finishWithResult({}, QStringLiteral("aoi.recipe.inspect.blocked_while_running"));
        return;
    }

    if (m_cancelRequested) {
        finishWithResult({}, QStringLiteral("aoi.recipe.inspect.cancelled"));
        return;
    }

    if (frame.isNull()) {
        finishWithResult({}, QStringLiteral("预览帧为空"));
        return;
    }

    bumpProgress(8, QStringLiteral("aoi.recipe.inspect.stage.runtime"));

    QString initError;
    if (!InspectionDetectService::instance().ensureInitialized(recipe, &initError)) {
        finishWithResult({},
                         initError.isEmpty() ? QStringLiteral("检测引擎初始化失败") : initError);
        return;
    }

    auto &engine = InspectionDetectService::instance().engine();

    if (m_cancelRequested) {
        finishWithResult({}, QStringLiteral("aoi.recipe.inspect.cancelled"));
        return;
    }

    bumpProgress(25, QStringLiteral("aoi.recipe.inspect.stage.init_models"));

    const auto processImage = image_display::qImageToRgbImageInfo(frame);
    if (!processImage || !processImage->imageMat) {
        finishWithResult({}, QStringLiteral("预览帧转 ImageInfo 失败"));
        return;
    }

    bumpProgress(45, QStringLiteral("aoi.recipe.inspect.stage.locate"));

    const FovDetectRequest request = makeAdHocFovRequest(processImage, recipe, kAdHocFovIndex);
    const FovDetectRecord  record  = engine.detectFov(request);

    if (m_cancelRequested) {
        finishWithResult({}, QStringLiteral("aoi.recipe.inspect.cancelled"));
        return;
    }

    bumpProgress(85, QStringLiteral("aoi.recipe.inspect.stage.summarize"));

    const QString originPath = ensureOriginImagePath(frame, sourceImagePath);

    const AoiSystemConfig sysConfig  = SystemConfigRepository::load();
    const DefectInfoList  defectList = buildAdHocDefectInfoList(
        record, recipe, sysConfig.defects, originPath, originPath);

    if (!record.error.isEmpty() && !record.ok) {
        finishWithResult({}, record.error);
        return;
    }

    if (auto *bridge = flow_host::flowHostCoordinator().activeBridge())
        bridge->reportReviewSession(defectList);

    const aoi::CurrentInspectionSnapshot snapshot =
        buildSnapshotFromDefectInfoList(defectList, sysConfig.defects);

    bumpProgress(100, QStringLiteral("aoi.recipe.inspect.stage.done"));
    LOG_INFO("[VirtualInspect] adhoc detectFov done groups={} partitions={} overall={}",
             record.groupNum, defectList.defectPartitions.size(),
             snapshot.overallResult == aoi::InspectionOverallResult::Ok ? "OK" : "NG");
    finishWithResult(snapshot, {});
}

void VirtualInspectRunner::cancel()
{
    m_cancelRequested = true;
}

void VirtualInspectRunner::finishWithResult(aoi::CurrentInspectionSnapshot snapshot, QString error)
{
    if (!m_running)
        return;

    m_running      = false;
    m_lastSnapshot = std::move(snapshot);
    m_lastError    = std::move(error);
    emit finished(m_lastSnapshot, m_lastError);
}

} // namespace aoi::inspection
