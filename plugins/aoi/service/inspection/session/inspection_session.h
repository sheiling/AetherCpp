#pragma once

#include "data/dataModel/recipe_config_model.h"
#include "data/dataModel/system_config_model.h"
#include "service/capture/capture_profile.h"
#include "service/flow/scan_flow_context.h"
#include "inspection_types.h"
#include "service/fov_planner/fov_geometry_planner.h"
#include "service/marker/inspection_marker_types.h"
#include "service/path_planner/capture_path_types.h"

#include <common/core/device/imageCapture/image_capture.h>

#include <QString>
#include <QVector>

#include <future>
#include <memory>
#include <vector>

namespace aoi::flow {
class ScanFlowProgressBridge;
} // namespace aoi::flow

namespace aoi::inspection {

/**
 * @brief 单板扫描工步流运行时上下文（经 FlowContext 以 shared_ptr 传递）
 *
 * | 字段 | 写入工步 | 说明 |
 * |------|----------|------|
 * | fovGeometryPlan | plan_geometry | FOV 网格与芯片索引 |
 * | captureMotionPlan | plan_geometry | 采集路径（默认逐点） |
 * | markerLocatedPhysical | mark_locate | 仿射锚点，不写回配方 |
 */
struct InspectionSession
{
    RecipeDocument                   recipe;
    AoiSystemConfig                  sysConfig;
    fov_planner::FovGeometryPlan     fovGeometryPlan;
    path_planner::CaptureMotionPlan  captureMotionPlan;
    /** 采图参数快照（Z/曝光/光源），由 capture::buildCaptureProfile 填充 */
    capture::CaptureProfile          captureProfile;

    int     currentImageIndex  = 0;
    int     totalImageCount    = 0;
    int     capturedImageCount = 0;
    int     detectedImageCount = 0;
    /** 最近 FOV debayer 后 RGB，供虚拟图/进度桥接 */
    std::shared_ptr<common::device::ImageInfo> lastFovPreview;

    /** Marker 采图预览（槽位 0..3），RGB ImageInfo */
    std::shared_ptr<common::device::ImageInfo> markerPreviews[marker::kMaxMarkerSlotCount];

    /**
     * Marker 模板匹配后的物理中心 (mm)，用于 FOV 仿射锚点。
     * 由 `service/marker/inspection_marker_service` 写入；不修改 recipe.marks.capturePos。
     */
    aoi::Point markerLocatedPhysical[marker::kMaxMarkerSlotCount];
    bool       markerLocatedValid[marker::kMaxMarkerSlotCount]{false, false, false, false};

    QVector<FovCaptureRecord> fovCaptures;
    QVector<FovDetectRecord>  fovDetects;
    PanelInspectionResult     panelResult;
    /** 检测历史库行 id（aggregate 后 append 写入，供复判 UPDATE） */
    qint64                    historyRecordId = -1;

    /** 双轨：轨1 规划可在轨2 Marker 期间异步计算 */
    int activeTrack = 0;

    QString progressMessage;
    flow::AbortFlag abortFlag;

    flow::ScanFlowProgressBridge *progressBridge = nullptr;
    /** 采图工步逐帧提交的异步检测 future（由 aoi.fov_detect 工步统一 get） */
    std::vector<std::future<FovDetectRecord>> pendingDetectFutures;

    InspectionSession();
    ~InspectionSession();

    bool isAbortRequested() const
    {
        return abortFlag && abortFlag->load();
    }
};

} // namespace aoi::inspection
