#pragma once

/**
 * @file scan_flow_progress_bridge.h
 * @brief 扫描工步 → UI 主线程的进度/预览桥（与 scan_flow_lifecycle / scan_flow_pause 同属 flow 模块）
 *
 * 业务相关性：强。接口面向 Mark/FOV 采图、虚拟全图导航、板级 KPI 快照等 AOI 首页场景，
 * 不属于通用 scheduler 基础设施。
 *
 * 线程模型：
 * - report* 由 InspectionSession / 工步线程调用
 * - signal 经 QueuedConnection 投递到 HomeViewModel / DefectReviewViewModel 所在 UI 线程
 *
 * 图像约定：入参与 signal 均为 shared_ptr<ImageInfo>；UI 侧 image_display::imageInfoToQImage 转 QImage。
 */

#include "inspection_types.h"

#include <common/core/device/imageCapture/image_capture.h>

#include <QObject>
#include <QRectF>
#include <QString>

#include <memory>

namespace aoi::flow {

/** @brief 扫描流程进度桥（QObject，供 HomeViewModel 连接 signal） */
class ScanFlowProgressBridge final : public QObject
{
    Q_OBJECT

public:
    explicit ScanFlowProgressBridge(QObject *parent = nullptr) : QObject(parent) {}

    /** @param level 运行日志级别：INFO / WARN / ERROR */
    void reportLog(const QString &message, const QString &level = QStringLiteral("INFO"));

    /**
     * @brief FOV 采图进度与当前帧预览
     * @param currentIndex 当前 FOV 序号（1-based）
     * @param totalCount   本轮 FOV 总数
     * @param fovImage     debayer 后 RGB 预览，可为空
     */
    void reportProgress(const QString &message, int currentIndex, int totalCount,
                        const std::shared_ptr<common::device::ImageInfo> &fovImage = {});

    /**
     * @brief 虚拟 FOV 全图导航：高亮当前格、同步预览与轨号
     * @param focusRect       归一化 [0,1] 焦点矩形（col/row 映射见 fov_capture_executor）
     * @param panelGroupCols  Panel 芯片组列数
     * @param panelGroupRows  Panel 芯片组行数
     * @param fovGridRows     FOV 网格行数
     * @param fovGridCols     FOV 网格列数
     * @param fovGridRow      当前 FOV 网格行
     * @param fovGridCol      当前 FOV 网格列
     * @param activeTrack     双轨时当前轨 1/2
     * @param dualTrackMode   是否双轨模式
     */
    void reportVirtualFov(const QString &message, int currentIndex, int totalCount,
                          const std::shared_ptr<common::device::ImageInfo> &fovImage,
                          const QRectF &focusRect, int panelGroupCols, int panelGroupRows,
                          int fovGridRows, int fovGridCols, int fovGridRow, int fovGridCol,
                          int activeTrack, bool dualTrackMode);

    /** Marker1 / Marker2 角色预览（ImageInfo） */
    void reportMarkPreview(const std::shared_ptr<common::device::ImageInfo> &mark1,
                           const std::shared_ptr<common::device::ImageInfo> &mark2);

    /** 板级检测快照（KPI、结果摘要等，不含大图） */
    void reportInspectionSnapshot(const aoi::CurrentInspectionSnapshot &snapshot);

    /**
     * @brief 上抛整板待复判会话
     *
     * 该信号承载的是 `DefectInfoList` 形式的完整复判输入数据，
     * 与首页快照 `inspectionSnapshotUpdated` 分离，避免 UI 侧把 KPI 快照误用为复判数据源。
     */
    void reportReviewSession(const aoi::inspection::DefectInfoList &data);

    /** 整轮扫描流程结束（成功/失败文案） */
    void reportFlowFinished(bool success, const QString &message);

signals:
    void logAppended(const QString &message, const QString &level);
    void progressUpdated(const QString &message, int currentIndex, int totalCount,
                         const std::shared_ptr<common::device::ImageInfo> &fovImage);
    void virtualFovUpdated(const QString &message, int currentIndex, int totalCount,
                           const std::shared_ptr<common::device::ImageInfo> &fovImage,
                           const QRectF &focusRect, int panelGroupCols, int panelGroupRows,
                           int fovGridRows, int fovGridCols, int fovGridRow, int fovGridCol,
                           int activeTrack, bool dualTrackMode);
    void markPreviewUpdated(const std::shared_ptr<common::device::ImageInfo> &mark1,
                            const std::shared_ptr<common::device::ImageInfo> &mark2);
    void inspectionSnapshotUpdated(const aoi::CurrentInspectionSnapshot &snapshot);
    void reviewSessionReady(const aoi::inspection::DefectInfoList &data);
    void flowFinished(bool success, const QString &message);
};

} // namespace aoi::flow
