#pragma once

/**
 * @file icamera_service.h
 * @brief 相机采图服务对外接口（Camera 插件导出）
 *
 * 职责边界（与工步 / ImageCapture 分工）：
 *
 * | 链路 | 触发方 | 数据流 |
 * |------|--------|--------|
 * | 预览 / 配方单帧 | UI 调 snapOnce / startPreview | ImageCapture(execDispFuncCB=true) → debayer → DispCB → FrameDispatcher → **frameReady(QImage)** |
 * | 角色图像上抛 | 工步 debayer 后 | 工步 shared_ptr<ImageInfo> → **publishRoleImageFromInfo** → QImage 降采样 → **roleImageReady** |
 * | 工步原图 / debayer | Inspection 工步 | **直调 DeviceManager::ImageCapture**，不经本接口 |
 *
 * QImage 仅在本插件内出现（预览 DispCB、角色上抛）；AOI 后台 algorithm/service/step 禁止 QImage。
 *
 * @see plugins/camera/service/camera_service_impl.cpp
 */

#include "camera_plugin_global.h"

#include <common/core/device/imageCapture/image_capture.h>

#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

class CAMERA_PLUGIN_EXPORT ICameraService : public QObject
{
    Q_OBJECT

public:
    explicit ICameraService(QObject *parent = nullptr) : QObject(parent) {}
    ~ICameraService() override = default;

    /** DeviceManager 中已注册且相机就绪的采集器名称列表 */
    virtual QStringList captureNames() const = 0;

    /** 采集器在 ImageCapture 配置里绑定的运控轴名（用于配方/标定页展示） */
    virtual QStringList captureAxisNames(const QString &captureName) const = 0;

    /** 采集器存在且底层相机 grab 回调已注册 */
    virtual bool isCaptureReady(const QString &captureName) const = 0;

    /** 设置曝光（微秒）；预览/单帧前可调用 */
    virtual void setExposure(const QString &captureName, int exposureUs) = 0;

    /**
     * @brief 配方单帧：触发一次 captureImage(true)，debayer 后经 frameReady 上抛
     * @note 若当前采集器处于预览，会先停预览再单帧
     */
    virtual bool snapOnce(const QString &captureName, int exposureUs = 0) = 0;

    /** 启动连续预览（continuousCapture(true)），帧经 frameReady 上抛 */
    virtual bool startPreview(const QString &captureName, int exposureUs = 0) = 0;

    /**
     * @brief 停止预览
     * @param deferHardware true 时先清 UI 状态，下一事件循环再停硬件（切页用，避免卡顿）
     */
    virtual void stopPreview(bool deferHardware = false) = 0;

    /**
     * @brief 工步链路上抛角色预览（入参须为已 debayer 的可显示 ImageInfo，插件内不再 debayer）
     * @param imageRole 调用方自定义，如 "Fov" / "Marker1"；订阅方按 role 过滤
     * @param maxDisplayEdge 长边像素上限，默认 1280
     */
    virtual void publishRoleImageFromInfo(const QString &captureName,
                                          const QString &imageRole,
                                          const std::shared_ptr<common::device::ImageInfo> &rgb,
                                          const QString &cameraName = {},
                                          int maxDisplayEdge        = 1280) = 0;

    /** 已有 QImage 时的角色上抛（内部仅降采样 + emit，供 publishRoleImageFromInfo 复用） */
    virtual void publishRoleImage(const QString &captureName,
                                  const QString &imageRole,
                                  QImage image,
                                  const QString &cameraName = {},
                                  int maxDisplayEdge        = 1280) = 0;

    virtual bool    isPreviewActive() const = 0;
    virtual QString activeCaptureName() const = 0;

    /** 显示控件 detach 时调用：deferHardware 停预览，避免阻塞 UI 线程 */
    virtual void onDisplayDetached() = 0;

signals:
    /** 预览 / 单帧：全分辨率或 FrameDispatcher 合并后的 QImage */
    void frameReady(QImage image);

    /**
     * @brief 角色桥接：工步 FOV / Marker 等 debayer 结果经降采样后上抛
     * @param imageRole 与 publishRoleImageFromInfo 入参一致
     */
    void roleImageReady(const QString &captureName,
                        const QString &cameraName,
                        const QString &imageRole,
                        QImage image);

    void previewActiveChanged(bool active);
    void captureError(const QString &message);
    void offline(const QString &captureName);
};
