#include "role_image_preview_binding.h"

#include <common/utility/diagnostics/timing_debug.h>

namespace aoi::ui_binding {

void bindRoleImagePreview(ui::ImagePreview *preview,
                          ICameraService *cameraService,
                          const QString &imageRole,
                          const QString &cameraName,
                          QObject *context)
{
    if (!preview || !cameraService || imageRole.isEmpty()) {
        return;
    }

    QObject *owner = context ? context : preview;
    QObject::connect(
        cameraService, &ICameraService::roleImageReady, owner,
        [preview, imageRole, cameraName](const QString & /*captureName*/,
                                         const QString &signalCameraName,
                                         const QString &signalRole, QImage image) {
            if (signalRole != imageRole || image.isNull()) {
                return;
            }
            if (!cameraName.isEmpty() && !signalCameraName.isEmpty()
                && signalCameraName != cameraName) {
                return;
            }
            common::utility::TimingDebugScope uiTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "UI.预览控件刷新",
                imageRole.toStdString() + " " + std::to_string(image.width()) + "x"
                    + std::to_string(image.height()));
            preview->setImage(image);
        },
        Qt::QueuedConnection);
}

} // namespace aoi::ui_binding
