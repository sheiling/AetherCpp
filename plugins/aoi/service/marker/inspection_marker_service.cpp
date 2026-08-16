/**
 * @file inspection_marker_service.cpp
 * @brief Marker 定位：运动至标定坐标采图、模板匹配、Session 缓存定位结果
 */

#include "inspection_marker_service.h"

#include "service/capture/capture_profile.h"
#include "service/capture/capture_motion_gateway.h"
#include "service/capture/capture_device_access.h"
#include "inspection_step_utils.h"

#include "service/vision_bridge/mark_point_locator.h"
#include "extensionsystem/pluginmanager.h"
#include "uibase/languagemanager.h"
#include "icamera_service.h"
#include "inspection_marker_types.h"
#include "param_define.h"

#include <common/base/imgproc/image_type_map.h>

namespace aoi::inspection {
namespace {

bool markerCapturePosReady(const MarkPointConfig &marker)
{
    return !(qFuzzyIsNull(marker.capturePos.x) && qFuzzyIsNull(marker.capturePos.y));
}

void publishMarkerRolePreview(const CommonParams &common, int slot,
                              const std::shared_ptr<common::device::ImageInfo> &processImage)
{
    auto *cam = ExtensionSystem::PluginManager::getInterface<ICameraService>();
    if (!cam || !processImage || !processImage->imageMat
        || AlgoMatIsEmpty(processImage->imageMat.get()) || slot < 0
        || slot >= marker::kMaxMarkerSlotCount) {
        return;
    }

    cam->publishRoleImageFromInfo(common.captureName,
                                  QStringLiteral("Marker%1").arg(slot + 1), processImage);
}

void reportMarkerPreview(const std::shared_ptr<InspectionSession> &session)
{
    if (!session || !session->progressBridge)
        return;
    session->progressBridge->reportMarkPreview(session->markerPreviews[0],
                                               session->markerPreviews[1]);
}

void applySlotResultToSession(const std::shared_ptr<InspectionSession> &session,
                              const marker::MarkerLocateSlotResult &result)
{
    if (!session || result.slot < 0 || result.slot >= marker::kMaxMarkerSlotCount)
        return;

    if (result.previewImage)
        session->markerPreviews[result.slot] = result.previewImage;

    session->markerLocatedPhysical[result.slot] = result.locatedPhysical;
    session->markerLocatedValid[result.slot]    = result.valid;
}

} // namespace

namespace marker {

aoi::Point computeMarkerPhysicalCenter(const RecipeDocument &recipe,
                                       const MarkPointConfig &marker,
                                       double pixelX,
                                       double pixelY)
{
    const double cx = recipe.camera.imageWidth > 0 ? recipe.camera.imageWidth * 0.5 : 960.0;
    const double cy = recipe.camera.imageHeight > 0 ? recipe.camera.imageHeight * 0.5 : 540.0;
    const double mmPerPx =
        recipe.camera.captureAccuracy > 0.0 ? recipe.camera.captureAccuracy : 1.0;

    aoi::Point physical;
    physical.x = marker.capturePos.x + (pixelX - cx) * mmPerPx;
    physical.y = marker.capturePos.y - (pixelY - cy) * mmPerPx;
    return physical;
}

scheduler::StepResult locateMarkerSlot(const std::shared_ptr<InspectionSession> &session,
                                       int slot,
                                       MarkerLocateSlotResult *outResult)
{
    MarkerLocateSlotResult result;
    result.slot = slot;

    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    if (session->isAbortRequested())
        return scheduler::StepResult::fail("aborted");
    if (slot < 0 || slot >= kMaxMarkerSlotCount)
        return scheduler::StepResult::fail("invalid marker slot");

    const auto &common = session->sysConfig.common;
    const int   markTotal = recipeMarkCount(common.trackMode);

    if (session->recipe.marks.size() < markTotal)
        session->recipe.marks.resize(markTotal);

    const MarkPointConfig &marker = session->recipe.marks[slot];
    if (!markerCapturePosReady(marker)) {
        const QString msg =
            QStringLiteral("Marker%1 物理坐标未配置，请先在配方 Mark 页完成标定").arg(slot + 1);
        reportSessionLog(session, msg, QStringLiteral("ERROR"));
        return scheduler::StepResult::fail(msg.toStdString());
    }

    session->captureProfile = capture::buildCaptureProfile(session->sysConfig, session->recipe);

    common::device::ImageCapture *capture = capture::resolveImageCapture(common.captureName);
    if (!capture) {
        reportSessionLog(session, QStringLiteral("采集器未找到"), QStringLiteral("ERROR"));
        return scheduler::StepResult::fail("image capture missing");
    }

    const double zMm   = capture::resolveMarkCaptureZ(session->recipe, slot);
    const int    trackNo =
        (common.trackMode == TrackMode::DualTrack && slot >= 2) ? 2 : 1;

    session->progressMessage =
        QStringLiteral("轨%1 Marker%2: 运动至标定 (%.3f, %.3f, %.3f) mm")
            .arg(trackNo)
            .arg(slot + 1)
            .arg(marker.capturePos.x)
            .arg(marker.capturePos.y)
            .arg(zMm);
    reportSessionLog(session, session->progressMessage);

    std::shared_ptr<common::device::ImageInfo> raw;
    QString                                    snapErr;
    if (!capture::CaptureMotionGateway::captureRawAt(common, session->captureProfile,
                                                     marker.capturePos.x, marker.capturePos.y, zMm,
                                                     &raw, &snapErr)) {
        reportSessionLog(session, snapErr, QStringLiteral("ERROR"));
        return scheduler::StepResult::fail(snapErr.toStdString());
    }

    // 每帧 debayer 一次：与 FOV 工步共用 makeRgbForProcess，processImage 供匹配 + 预览
    std::shared_ptr<common::device::ImageInfo> processImage = capture->makeRgbForProcess(raw);
    if (!processImage) {
        reportSessionLog(session, QStringLiteral("Marker debayer 失败"), QStringLiteral("ERROR"));
        return scheduler::StepResult::fail("marker debayer failed");
    }

    result.previewImage    = processImage;
    result.locatedPhysical = {marker.capturePos.x, marker.capturePos.y};
    result.templateMatched = false;
    result.valid           = true;

    if (MarkPointLocator::sdkReady()
        && !QString::fromStdString(marker.markerInfo.modelFileName).trimmed().isEmpty()) {
        // 全程 ImageInfo；匹配结果预览亦为 RGB24 ImageInfo
        const MarkLocateResult loc = MarkPointLocator::locate(processImage, marker);
        if (loc.ok) {
            result.locatedPhysical =
                computeMarkerPhysicalCenter(session->recipe, marker, loc.pixelX, loc.pixelY);
            result.templateMatched = true;
            reportSessionLog(
                session,
                QStringLiteral("Marker%1 模板匹配 OK 像素(%.1f, %.1f) → 定位物理(%.3f, %.3f) mm")
                    .arg(slot + 1)
                    .arg(loc.pixelX)
                    .arg(loc.pixelY)
                    .arg(result.locatedPhysical.x)
                    .arg(result.locatedPhysical.y));
            if (loc.resultImage)
                result.previewImage = loc.resultImage; // 深拷贝预览，可覆盖 debayer 原图
        } else {
            reportSessionLog(session,
                             QStringLiteral("Marker%1 模板匹配失败(%2)，使用标定坐标作为定位结果")
                                 .arg(slot + 1)
                                 .arg(loc.message.isEmpty() ? TR(loc.errorKey) : loc.message),
                             QStringLiteral("WARN"));
        }
    } else {
        reportSessionLog(session,
                         QStringLiteral("Marker%1 无模板/SDK，使用标定坐标作为定位结果")
                             .arg(slot + 1),
                         QStringLiteral("WARN"));
    }

    applySlotResultToSession(session, result);
    publishMarkerRolePreview(common, slot, result.previewImage);
    reportMarkerPreview(session);
    reportSessionProgress(session);

    if (outResult)
        *outResult = result;

    return scheduler::StepResult::ok();
}

scheduler::StepResult runMarkerLocateService(const std::shared_ptr<InspectionSession> &session,
                                             scheduler::FlowContext &ctx,
                                             int firstSlot,
                                             int slotCount)
{
    Q_UNUSED(ctx);

    if (!session)
        return scheduler::StepResult::fail("inspection session missing");
    if (session->isAbortRequested())
        return scheduler::StepResult::fail("aborted");

    const int markTotal = recipeMarkCount(session->sysConfig.common.trackMode);
    if (firstSlot < 0 || slotCount <= 0 || firstSlot + slotCount > markTotal)
        return scheduler::StepResult::fail("invalid marker slot range");

    for (int slot = firstSlot; slot < firstSlot + slotCount; ++slot) {
        if (session->isAbortRequested())
            return scheduler::StepResult::fail("aborted");

        const auto result = locateMarkerSlot(session, slot);
        if (!result.isOk())
            return result;
    }

    session->progressMessage =
        QStringLiteral("Marker 定位完成 [%1-%2]，定位坐标已写入 Session")
            .arg(firstSlot + 1)
            .arg(firstSlot + slotCount);
    reportSessionLog(session, session->progressMessage);
    return scheduler::StepResult::ok();
}

} // namespace marker

scheduler::StepResult runMarkerLocateSlots(const std::shared_ptr<InspectionSession> &session,
                                           scheduler::FlowContext &ctx,
                                           int firstSlot,
                                           int slotCount)
{
    return marker::runMarkerLocateService(session, ctx, firstSlot, slotCount);
}

} // namespace aoi::inspection
