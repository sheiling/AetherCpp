#pragma once

/**
 * @file image_info_display.h
 * @brief UI 层 ImageInfo ↔ QImage 转换（全工程唯一 QImage 入口之一）
 *
 * 使用场景：
 * - HomeViewModel：ScanFlowProgressBridge signal → HomeRuntimeState.preview（QImage）
 * - RecipeConfigViewModel：ICameraService::frameReady(QImage) → 预览刷新 / 扫码
 * - RecipeConfigMarkView：Mark 计算 / ModelGenerator 请求 / 结果缩略图
 *
 * 命名空间 aoi::image_display 刻意与 aoi::ui（Themed 控件）分离，避免符号冲突。
 */

#include <common/core/device/imageCapture/image_capture.h>

#include <QImage>

#include <memory>

namespace aoi::image_display {

/**
 * @brief RGB24 ImageInfo → QImage
 * @param maxDisplayEdge 长边上限；0 表示不缩放（模板缩略图用 0，主页预览默认 1280）
 */
QImage imageInfoToQImage(const std::shared_ptr<common::device::ImageInfo> &info,
                         int maxDisplayEdge = 1280);

/** QImage（任意格式）→ RGB24 ImageInfo；相机 frameReady 边界调用 */
std::shared_ptr<common::device::ImageInfo> qImageToRgbImageInfo(const QImage &image);

} // namespace aoi::image_display
