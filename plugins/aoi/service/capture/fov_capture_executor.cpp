/**
 * @file fov_capture_executor.cpp
 * @brief FOV 逐格运动采图工步实现
 *
 * 调用链：aoi.fov_capture → inspection::runFovCapture → FovCaptureExecutor::execute
 *
 * 单格链路（与 Marker 采图一致，每帧 debayer 一次）：
 *   Z→XY 运动 → 曝光/光源 → snapRaw → makeRgbForProcess → 缓存 + UI 上抛 → 可选 detectAsync
 *
 * 图像约束：后台全程 shared_ptr<ImageInfo>；角色预览经 ICameraService::publishRoleImageFromInfo。
 */

#include "fov_capture_executor.h"

#include "capture_device_access.h"
#include "capture_motion_gateway.h"
#include "capture_profile.h"

#include "service/inspection/detect/inspection_detect_service.h"
#include "service/flow/scan_flow_pause.h"
#include "service/flow/scan_flow_context.h"
#include "inspection_step_utils.h"

#include "extensionsystem/pluginmanager.h"
#include "icamera_service.h"

#include <QRectF>

namespace aoi::capture {
namespace {

/**
 * @brief 向 ScanFlowProgressBridge 上报当前 FOV 格在虚拟全图中的焦点位置
 * @param session   扫描会话（progressBridge / lastFovPreview / 双轨 activeTrack）
 * @param cell      当前格（gridRow/gridCol 与 centerPhysical 来自 fov_geometry_plan）
 * @param rowCount  FOV 网格总行数（虚拟图纵轴划分数）
 * @param colCount  FOV 网格总列数（虚拟图横轴划分数）
 *
 * focusRect 为归一化 [0,1] 矩形：左上角 (col/ cols, row/rows)，宽高 1/cols × 1/rows，
 * 供首页 PanelVirtualMap 高亮「正在采集」的格子。
 */
void reportVirtualFov(const std::shared_ptr<inspection::InspectionSession> &session,
                      const fov_planner::FovGridCell &cell, int rowCount, int colCount)
{
    if (!session || !session->progressBridge)
        return;

    const double normW = colCount > 0 ? 1.0 / static_cast<double>(colCount) : 1.0;
    const double normH = rowCount > 0 ? 1.0 / static_cast<double>(rowCount) : 1.0;
    const QRectF focusRect(normW * cell.gridCol, normH * cell.gridRow, normW, normH);
    const bool dualTrack = session->sysConfig.common.trackMode == TrackMode::DualTrack;
    const int  track     = session->activeTrack > 0 ? session->activeTrack : 1;
    session->progressBridge->reportVirtualFov(
        session->progressMessage, session->currentImageIndex, session->totalImageCount,
        session->lastFovPreview, focusRect, session->recipe.basic.width,
        session->recipe.basic.height, rowCount, colCount, cell.gridRow, cell.gridCol, track,
        dualTrack);
}

} // namespace

scheduler::StepResult FovCaptureExecutor::execute(
    const std::shared_ptr<inspection::InspectionSession> &session, scheduler::FlowContext &ctx,
    bool submitAsyncDetect)
{
    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    // 输入：plan_geometry 写入的 FOV 网格（cells 行优先，与 path_planner 访问序一致）
    const auto &cells = session->fovGeometryPlan.grid.cells;
    if (cells.empty())
        return scheduler::StepResult::fail("FOV plan empty");

    const auto &common = session->sysConfig.common;
    // 曝光/Z 轴等：load_recipe 可能已填充；此处兜底重建
    if (session->captureProfile.exposureUs <= 0)
        session->captureProfile = buildCaptureProfile(session->sysConfig, session->recipe);

    const double zMm  = session->captureProfile.fovCaptureZmm;
    const int    rows = session->fovGeometryPlan.grid.rowCount;
    const int    cols = session->fovGeometryPlan.grid.colCount;

    // 工步直访 DeviceManager::ImageCapture（debayer 在 capture 实例上完成，不经相机插件采图）
    common::device::ImageCapture *capture = resolveImageCapture(common.captureName);
    if (!capture) {
        inspection::reportSessionLog(session, QStringLiteral("采集器未找到: %1").arg(common.captureName),
                                     QStringLiteral("ERROR"));
        return scheduler::StepResult::fail("image capture missing");
    }

    // 本轮采图计数与检测 future 队列清零（检测 result 由 aoi.fov_detect 工步 get）
    session->fovCaptures.clear();
    session->fovDetects.clear();
    session->pendingDetectFutures.clear();
    session->capturedImageCount = 0;
    session->detectedImageCount = 0;
    session->totalImageCount    = static_cast<int>(cells.size());

    // 采图与检测流水线：此处仅初始化引擎；每帧采完后 push detectAsync，不阻塞下一格运动
    if (submitAsyncDetect) {
        QString initError;
        if (!inspection::InspectionDetectService::instance().ensureInitialized(session->recipe,
                                                                               &initError)) {
            inspection::reportSessionLog(session, initError, QStringLiteral("ERROR"));
            return scheduler::StepResult::fail(initError.toStdString());
        }
    }

    // 角色通道「Fov」预览：供首页 bindRoleImagePreview 订阅（QImage 边界在 Camera 插件内）
    auto *cam = ExtensionSystem::PluginManager::getInterface<ICameraService>();

    for (std::size_t i = 0; i < cells.size(); ++i) {
        // 流程 abort 或整机暂停（暂停时 waitWhileMachinePaused 阻塞，恢复后继续）
        if (session->isAbortRequested()) {
            CaptureMotionGateway::emergencyStop(common);
            return scheduler::StepResult::fail("aborted");
        }
        if (!flow::waitWhileMachinePaused(session))
            return scheduler::StepResult::fail("aborted");

        const fov_planner::FovGridCell &cell = cells[i];
        session->currentImageIndex           = static_cast<int>(i) + 1;

        session->progressMessage =
            QStringLiteral("FOV %1/%2 → (%.3f, %.3f, Z=%.3f) mm")
                .arg(session->currentImageIndex)
                .arg(session->totalImageCount)
                .arg(cell.centerPhysical.x)
                .arg(cell.centerPhysical.y)
                .arg(zMm);
        inspection::reportSessionLog(session, session->progressMessage);

        std::shared_ptr<common::device::ImageInfo> raw;
        QString                                    snapErr;
        // 运动 + 曝光 + 单帧原图（Bayer/Mono）；失败则整板采图工步 fail
        if (!CaptureMotionGateway::captureRawAt(common, session->captureProfile,
                                                cell.centerPhysical.x, cell.centerPhysical.y, zMm,
                                                &raw, &snapErr)) {
            inspection::reportSessionLog(session, snapErr, QStringLiteral("ERROR"));
            return scheduler::StepResult::fail(snapErr.toStdString());
        }

        // 每帧 debayer 一次；processImage 供检测、虚拟图、角色预览共用
        std::shared_ptr<common::device::ImageInfo> processImage = capture->makeRgbForProcess(raw);
        if (!processImage) {
            inspection::reportSessionLog(session, QStringLiteral("FOV debayer 失败"),
                                         QStringLiteral("ERROR"));
            return scheduler::StepResult::fail("fov debayer failed");
        }

        inspection::FovCaptureRecord record;
        record.fovIndex     = session->currentImageIndex;
        record.gridRow      = cell.gridRow;
        record.gridCol      = cell.gridCol;
        record.centerXmm    = cell.centerPhysical.x;
        record.centerYmm    = cell.centerPhysical.y;
        record.rawImage     = raw;
        record.processImage = processImage;
        session->fovCaptures.append(record);

        session->lastFovPreview     = processImage;
        session->capturedImageCount = session->currentImageIndex;

        ctx.set(flow::kCtxCurrentImageIndex, session->currentImageIndex);
        ctx.set(flow::kCtxCapturedImages, session->capturedImageCount);

        if (cam) {
            cam->publishRoleImageFromInfo(common.captureName, QStringLiteral("Fov"), processImage);
        }

        reportVirtualFov(session, cell, rows, cols);

        // 提交异步 CISDK 检测；future 存入 session，由 runFovDetectService 统一 wait/get
        if (submitAsyncDetect) {
            auto &detectEngine = inspection::InspectionDetectService::instance().engine();
            session->pendingDetectFutures.push_back(
                detectEngine.detectAsync(processImage, record.fovIndex, session->recipe));
            inspection::reportSessionLog(
                session, QStringLiteral("FOV %1 已提交异步缺陷检测").arg(record.fovIndex));
        }
    }

    session->progressMessage =
        QStringLiteral("采图完成，共 %1 张 FOV").arg(session->capturedImageCount);
    inspection::reportSessionLog(session, session->progressMessage);
    return scheduler::StepResult::ok();
}

} // namespace aoi::capture
