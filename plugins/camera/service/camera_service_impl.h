#pragma once

/**
 * @file camera_service_impl.h
 * @brief ICameraService 实现：采集器生命周期、预览会话、帧与角色图像投递
 *
 * 线程模型：
 * - ImageCapture DispCB 可能在采集线程触发 → FrameDispatcher 合并后 QueuedConnection 到插件线程 emit
 * - previewActiveChanged 若跨线程则 invokeMethod 到插件对象线程
 *
 * 会话状态（m_mutex 保护）：
 * - 同时最多一个 activePreviewCaptureName
 * - m_captureKindByName 区分 LivePreview / SingleShot，供 DispCB 路由
 */

#include <icamera_service.h>

#include "capture_manager.h"
#include "frame_dispatcher.h"

#include <QMutex>

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace camera_plugin {

class CameraServiceImpl final : public ICameraService
{
    Q_OBJECT

public:
    explicit CameraServiceImpl(QObject *parent = nullptr);
    ~CameraServiceImpl() override;

    /** delayedInitialize 阶段从 DeviceManager 刷新采集器并注册 DispCB */
    bool refreshCaptures(QString *errorOut = nullptr);

    QStringList captureNames() const override;
    QStringList captureAxisNames(const QString &captureName) const override;
    bool        isCaptureReady(const QString &captureName) const override;

    void setExposure(const QString &captureName, int exposureUs) override;

    bool snapOnce(const QString &captureName, int exposureUs = 0) override;
    bool startPreview(const QString &captureName, int exposureUs = 0) override;
    void stopPreview(bool deferHardware = false) override;

    void publishRoleImageFromInfo(const QString &captureName,
                                  const QString &imageRole,
                                  const std::shared_ptr<common::device::ImageInfo> &rgb,
                                  const QString &cameraName = {},
                                  int maxDisplayEdge        = 1280) override;
    void publishRoleImage(const QString &captureName,
                          const QString &imageRole,
                          QImage image,
                          const QString &cameraName = {},
                          int maxDisplayEdge        = 1280) override;

    bool    isPreviewActive() const override;
    QString activeCaptureName() const override;

    void onDisplayDetached() override;

    /** 插件卸载：停预览、清回调、CaptureManager::shutdown */
    void shutdown();

private:
    /** 为每个就绪采集器注册 ImageCapture::RegisterDispCB */
    void wireCaptureCallbacks();
    /** 绑定 FrameDispatcher → emit frameReady */
    void wireFrameEmitter();
    /** 取消尚未执行的 deferHardware 停流 */
    void cancelDeferredStop();
    void setPreviewActiveState(bool active);
    bool stopCaptureInternal(const QString &captureName, bool waitDrain);

    mutable QMutex                                   m_mutex;
    CaptureManager                                   m_captureManager;
    std::unique_ptr<FrameDispatcher>                 m_frameDispatcher;
    QString                                          m_activePreviewCaptureName;
    std::unordered_map<std::string, CameraImageKind> m_captureKindByName;
    /** stopPreview(defer) 代际号，新预览/单帧会递增以作废 pending 停流 */
    std::atomic<quint64>                             m_deferredStopGeneration{0};
    bool                                             m_initialized = false;
};

} // namespace camera_plugin
