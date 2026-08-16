/**
 * @file scan_flow_progress_bridge.cpp
 * @brief ScanFlowProgressBridge 实现：跨线程 signal 投递与元类型注册
 */

#include "scan_flow_progress_bridge.h"

#include <QMetaType>
#include <QRectF>

namespace aoi::flow {
namespace {

/** QueuedConnection 所需元类型（程序启动时注册一次） */
const bool kBridgeMetaRegistered = [] {
    qRegisterMetaType<aoi::CurrentInspectionSnapshot>("aoi::CurrentInspectionSnapshot");
    qRegisterMetaType<aoi::inspection::DefectInfoList>("aoi::inspection::DefectInfoList");
    qRegisterMetaType<QRectF>("QRectF");
    qRegisterMetaType<std::shared_ptr<common::device::ImageInfo>>(
        "std::shared_ptr<common::device::ImageInfo>");
    return true;
}();

} // namespace

void ScanFlowProgressBridge::reportLog(const QString &message, const QString &level)
{
    emit logAppended(message, level);
}

void ScanFlowProgressBridge::reportProgress(const QString &message, int currentIndex,
                                            int totalCount,
                                            const std::shared_ptr<common::device::ImageInfo> &fovImage)
{
    emit progressUpdated(message, currentIndex, totalCount, fovImage);
}

void ScanFlowProgressBridge::reportVirtualFov(
    const QString &message, int currentIndex, int totalCount,
    const std::shared_ptr<common::device::ImageInfo> &fovImage, const QRectF &focusRect,
    int panelGroupCols, int panelGroupRows, int fovGridRows, int fovGridCols, int fovGridRow,
    int fovGridCol, int activeTrack, bool dualTrackMode)
{
    emit virtualFovUpdated(message, currentIndex, totalCount, fovImage, focusRect, panelGroupCols,
                           panelGroupRows, fovGridRows, fovGridCols, fovGridRow, fovGridCol,
                           activeTrack, dualTrackMode);
}

void ScanFlowProgressBridge::reportMarkPreview(
    const std::shared_ptr<common::device::ImageInfo> &mark1,
    const std::shared_ptr<common::device::ImageInfo> &mark2)
{
    emit markPreviewUpdated(mark1, mark2);
}

void ScanFlowProgressBridge::reportInspectionSnapshot(const aoi::CurrentInspectionSnapshot &snapshot)
{
    emit inspectionSnapshotUpdated(snapshot);
}

void ScanFlowProgressBridge::reportReviewSession(const aoi::inspection::DefectInfoList &data)
{
    emit reviewSessionReady(data);
}

void ScanFlowProgressBridge::reportFlowFinished(bool success, const QString &message)
{
    emit flowFinished(success, message);
}

} // namespace aoi::flow
