#pragma once

/**
 * @file camera_types.h
 * @brief Camera 插件内部帧描述与回调类型
 *
 * CameraFrame 在 ImageCapture DispCB → FrameDispatcher → ICameraService::frameReady 链路中传递。
 * algoInfo 保留 debayer 后 ImageInfo 引用；displayImage 在投递 UI 前由 FrameDispatcher 填充。
 */

#include <common/core/device/imageCapture/image_capture.h>

#include <QImage>
#include <QString>

#include <functional>
#include <memory>

namespace camera_plugin {

/** 帧来源语义，决定 DispCB 是否进入 FrameDispatcher / frameReady */
enum class CameraImageKind {
    LivePreview, ///< continuousCapture + execDispFuncCB
    SingleShot,  ///< captureImage(true) + execDispFuncCB
    RawCapture,  ///< execDispFuncCB=false 原图入队；当前不经 DispCB 上抛 UI
};

struct CameraFrame {
    QString                                    captureName;
    QString                                    cameraName;
    CameraImageKind                            kind = CameraImageKind::LivePreview;
    uint64_t                                   frameIndex = 0;
    QImage                                     displayImage; ///< UI 用，RGB888 或 Grayscale8
    std::shared_ptr<common::device::ImageInfo> algoInfo;
};

using FrameHandler = std::function<void(CameraFrame frame)>;

} // namespace camera_plugin
