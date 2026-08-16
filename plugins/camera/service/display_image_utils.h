#pragma once

/**
 * @file display_image_utils.h
 * @brief 角色图像上抛前的 QImage 降采样（工步 FOV/Marker 大图减轻 UI 绑定开销）
 */

#include <QImage>

namespace camera_plugin {

/**
 * @brief 长边限制在 maxEdge 内，保持宽高比
 * @param maxEdge <=0 时原样返回
 */
QImage downscaleForDisplay(const QImage &source, int maxEdge);

} // namespace camera_plugin
