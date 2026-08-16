#pragma once

/**
 * @file frame_dispatcher.h
 * @brief 采集线程 DispCB → 插件线程 UI 的帧合并与投递
 *
 * 策略：
 * - 采集线程只 publishFrame，合并为「最新一帧」pending
 * - AlgoMat→QImage 在 QtConcurrent 工作线程执行，避免阻塞插件/UI 线程
 * - deliverPendingFrame 在插件对象线程 QueuedConnection 执行，仅投递已转换 QImage
 * - onConvertFinished 必须投递已转好的 QImage；不可因 pending 非空丢弃（与采集侧 debayer 饿死同理）
 * - convertInFlight 期间仅合并 pending，勿置 m_publishPosted（否则转图完成后无法 scheduleDeliver）
 * - toDisplayImage：ImageInfo(RGB24/MONO8) → QImage，供 frameReady / publishRoleImageFromInfo 共用
 */

#include <camera_types.h>
#include <common/core/device/imageCapture/image_capture.h>

#include <QMutex>
#include <QObject>

#include <atomic>
#include <functional>
#include <memory>

namespace camera_plugin {

class FrameDispatcher final : public QObject
{
    Q_OBJECT

public:
    explicit FrameDispatcher(QObject *parent = nullptr);

    void setFrameHandler(FrameHandler handler);
    void publishFrame(CameraFrame frame);
    void clearPendingFrames();

    /** ImageCapture DispCB 入口：包装 metadata 后 publishFrame */
    void dispatchFromCapture(const std::string &captureName,
                             const std::string &cameraName,
                             CameraImageKind kind,
                             const std::shared_ptr<common::device::ImageInfo> &info,
                             const common::device::CameraInfo &cameraInfo);

    /** 插件内 ImageInfo → QImage 唯一转换点（预览 + 角色上抛） */
    static QImage toDisplayImage(const std::shared_ptr<common::device::ImageInfo> &info);

private slots:
    void deliverPendingFrame();
    void onConvertFinished(QImage image);

private:
    void finishDeliver(CameraFrame frame);
    void scheduleDeliver();
    void trySchedulePendingDeliver();
    void startConvert(CameraFrame frame);

    FrameHandler          m_frameHandler;
    QMutex                m_publishMutex;
    CameraFrame           m_pendingPublish;
    bool                  m_publishPosted   = false;
    std::atomic<bool>     m_convertInFlight {false};
    CameraFrame           m_convertingFrame;
};

} // namespace camera_plugin
