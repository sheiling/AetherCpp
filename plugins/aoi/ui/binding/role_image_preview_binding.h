#pragma once

#include "component/image/image_preview.h"
#include "icamera_service.h"

#include <QObject>

namespace aoi::ui_binding {

/**
 * @brief 将 ImagePreview 绑定到相机插件 roleImageReady 信号
 *
 * 收到图像后先匹配 imageRole；cameraName 非空时再匹配相机名（capture 配置的 cameraName）。
 */
void bindRoleImagePreview(ui::ImagePreview *preview,
                          ICameraService *cameraService,
                          const QString &imageRole,
                          const QString &cameraName = QString(),
                          QObject *context            = nullptr);

} // namespace aoi::ui_binding
