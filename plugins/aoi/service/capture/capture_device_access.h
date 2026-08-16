#pragma once

/**
 * @file capture_device_access.h
 * @brief 工步层直访 DeviceManager::ImageCapture（不经相机插件采图/debayer）
 *
 * 流程位置：Marker / FOV 采图底层；上层 `CaptureMotionGateway` 封装运动+曝光后调用。
 * 图像格式：出参为相机原图（Bayer/Mono），debayer 由 ImageCapture::makeRgbForProcess 完成。
 */

#include <common/core/device/imageCapture/image_capture.h>

#include <QString>

#include <memory>

namespace aoi::capture {

/**
 * @brief 按采集器名称解析 ImageCapture 实例
 * @param captureName 系统配置 common.captureName
 * @return 就绪的 ImageCapture*；未绑定或未找到时 nullptr
 */
common::device::ImageCapture *resolveImageCapture(const QString &captureName);

/**
 * @brief 工步单帧原图采集（captureImage(false) + getImage）
 * @param capture    ImageCapture 实例，须 isReady()
 * @param exposureUs 曝光 (μs)；>0 时先 setExposureTime
 * @param outRaw     成功时写入原图 shared_ptr
 * @param errorOut   失败时写入可读错误信息
 */
bool snapRawFromCapture(common::device::ImageCapture *capture, int exposureUs,
                        std::shared_ptr<common::device::ImageInfo> *outRaw,
                        QString *errorOut = nullptr);

} // namespace aoi::capture
