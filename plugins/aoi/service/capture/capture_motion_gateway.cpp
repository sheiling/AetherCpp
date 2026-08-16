#include "capture_motion_gateway.h"

#include "capture_device_access.h"

#include "extensionsystem/pluginmanager.h"
#include "icamera_service.h"
#include "imotion_control_service.h"

#include <common/utility/diagnostics/timing_debug.h>

#include <functional>

#include <QThread>

namespace aoi::capture {
namespace {

using motion_control_plugin::IMotionControlService;

/** 到位判定容差 (mm) */
constexpr double kPositionToleranceMm = 0.05;

/** 轮询 predicate 直至 true 或超时 */
bool waitUntil(const std::function<bool()> &predicate, int timeoutMs)
{
    const int stepMs = 20;
    int       elapsed = 0;
    while (elapsed < timeoutMs) {
        if (predicate())
            return true;
        QThread::msleep(static_cast<unsigned long>(stepMs));
        elapsed += stepMs;
    }
    return predicate();
}

/** 工步采图前若同相机在预览则停止，避免与单帧采图冲突 */
void stopPreviewIfActive(const QString &captureName)
{
    auto *cam = ExtensionSystem::PluginManager::getInterface<ICameraService>();
    if (!cam || !cam->isPreviewActive()) {
        return;
    }
    if (cam->activeCaptureName().isEmpty() || cam->activeCaptureName() == captureName) {
        common::utility::TimingDebugScope stopPreviewTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "AOI.停预览",
            captureName.toStdString());
        cam->stopPreview(false);
    }
}

} // namespace

CaptureReadyStatus CaptureMotionGateway::checkReady(const CommonParams &common)
{
    CaptureReadyStatus status;

    if (common.captureName.isEmpty()) {
        status.message = QStringLiteral("系统配置未设置相机名称 (common.capture_name)");
        return status;
    }
    if (common.axisXName.isEmpty() || common.axisYName.isEmpty()) {
        status.message = QStringLiteral("系统配置未设置运控轴名 (common.axis_x/y_name)");
        return status;
    }

    common::device::ImageCapture *capture = resolveImageCapture(common.captureName);
    if (!capture) {
        status.message = QStringLiteral("采集器未找到: %1").arg(common.captureName);
        return status;
    }
    if (!capture->isReady()) {
        status.message = QStringLiteral("相机未就绪: %1").arg(common.captureName);
        return status;
    }

    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!motion) {
        status.message = QStringLiteral("运控服务未加载");
        return status;
    }
    if (!motion->isReady()) {
        status.message = QStringLiteral("运控未就绪");
        return status;
    }
    if (!motion->axisNames().contains(common.axisXName)
        || !motion->axisNames().contains(common.axisYName)) {
        status.message = QStringLiteral("运控轴不存在: %1 / %2")
                             .arg(common.axisXName, common.axisYName);
        return status;
    }

    status.ok = true;
    return status;
}

bool CaptureMotionGateway::moveAxisAndWait(const CommonParams &common, const QString &axisName,
                                           double targetPos, QString *errorOut)
{
    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!motion) {
        if (errorOut)
            *errorOut = QStringLiteral("运控服务未加载");
        return false;
    }

    if (!motion->moveToPosition(axisName, targetPos, true, common.motionSpeed)) {
        if (errorOut)
            *errorOut = QStringLiteral("轴 %1 运动指令失败").arg(axisName);
        return false;
    }

    const bool arrived = waitUntil(
        [&]() {
            motion->refreshAxisStates();
            const auto st = motion->axisState(axisName);
            return st.inPosition || qAbs(st.fbPos - targetPos) <= kPositionToleranceMm;
        },
        common.motionTimeoutMs);

    if (!arrived) {
        if (errorOut)
            *errorOut = QStringLiteral("轴 %1 运动超时").arg(axisName);
        return false;
    }
    return true;
}

bool CaptureMotionGateway::moveXYAndWait(const CommonParams &common, double xMm, double yMm,
                                         QString *errorOut)
{
    QString err;
    if (!moveAxisAndWait(common, common.axisXName, xMm, &err)) {
        if (errorOut)
            *errorOut = err;
        return false;
    }
    if (!moveAxisAndWait(common, common.axisYName, yMm, &err)) {
        if (errorOut)
            *errorOut = err;
        return false;
    }
    return true;
}

bool CaptureMotionGateway::moveXYZAndWait(const CommonParams &common, double xMm, double yMm,
                                          double zMm, const CaptureProfile &profile,
                                          QString *errorOut)
{
    QString err;
    if (profile.hasZAxis && !profile.axisZName.isEmpty()) {
        if (!moveAxisAndWait(common, profile.axisZName, zMm, &err)) {
            if (errorOut)
                *errorOut = err;
            return false;
        }
    }
    return moveXYAndWait(common, xMm, yMm, errorOut);
}

bool CaptureMotionGateway::applyCaptureProfile(const CommonParams &common,
                                               const CaptureProfile &profile, QString *errorOut)
{
    common::device::ImageCapture *capture = resolveImageCapture(common.captureName);
    if (!capture) {
        if (errorOut)
            *errorOut = QStringLiteral("采集器未找到: %1").arg(common.captureName);
        return false;
    }

    if (profile.exposureUs > 0) {
        capture->setExposureTime(profile.exposureUs);
    }

    Q_UNUSED(profile.lightBrightness);
    return true;
}

bool CaptureMotionGateway::captureRawAt(const CommonParams &common, const CaptureProfile &profile,
                                        double xMm, double yMm, double zMm,
                                        std::shared_ptr<common::device::ImageInfo> *outRaw,
                                        QString *errorOut)
{
    QString err;
    if (!moveXYZAndWait(common, xMm, yMm, zMm, profile, &err)) {
        if (errorOut)
            *errorOut = err;
        return false;
    }
    if (!applyCaptureProfile(common, profile, &err)) {
        if (errorOut)
            *errorOut = err;
        return false;
    }

    return snapRawImage(common, profile.exposureUs, outRaw, errorOut);
}

bool CaptureMotionGateway::snapRawImage(const CommonParams &common, int exposureUs,
                                        std::shared_ptr<common::device::ImageInfo> *outRaw,
                                        QString *errorOut)
{
    common::utility::TimingDebugScope totalTiming(common::utility::TimingDebugKeys::CaptureTiming,
                                                  "AOI.采图总耗时", common.captureName.toStdString());

    common::device::ImageCapture *capture = resolveImageCapture(common.captureName);
    if (!capture) {
        if (errorOut)
            *errorOut = QStringLiteral("采集器未找到: %1").arg(common.captureName);
        return false;
    }

    stopPreviewIfActive(common.captureName);

    if (!snapRawFromCapture(capture, exposureUs > 0 ? exposureUs : 0, outRaw, errorOut)) {
        return false;
    }
    return outRaw && *outRaw;
}

void CaptureMotionGateway::emergencyStop(const CommonParams &common)
{
    Q_UNUSED(common);
    if (auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>())
        motion->emergencyStopAll();
}

} // namespace aoi::capture
