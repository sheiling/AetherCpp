#include "capture_device_access.h"

#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/utility/diagnostics/timing_debug.h>

namespace aoi::capture {
namespace {

/** 从 Container 获取 DeviceManager；未装配或解析失败返回 nullptr */
std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app || !app->bound<common::device::DeviceManager>()) {
            return nullptr;
        }
        return app->make<common::device::DeviceManager>();
    } catch (...) {
        return nullptr;
    }
}

} // namespace

common::device::ImageCapture *resolveImageCapture(const QString &captureName)
{
    const auto dm = tryGetDeviceManager();
    if (!dm || captureName.isEmpty()) {
        return nullptr;
    }
    const auto capture = dm->ImageCaptureHandler(captureName.toStdString());
    return capture ? capture.get() : nullptr;
}

bool snapRawFromCapture(common::device::ImageCapture *capture, int exposureUs,
                        std::shared_ptr<common::device::ImageInfo> *outRaw,
                        QString *errorOut)
{
    if (!capture || !capture->isReady()) {
        if (errorOut) {
            *errorOut = QStringLiteral("采集器未就绪");
        }
        return false;
    }

    {
        common::utility::TimingDebugScope prepTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "工步.采图准备",
            capture->captureName());
        if (exposureUs > 0) {
            capture->setExposureTime(exposureUs);
        }
        capture->clearImageDeque();
    }

    capture->captureImage(false);

    std::shared_ptr<common::device::ImageInfo> raw = capture->getImage();
    if (!raw || !raw->imageMat || AlgoMatIsEmpty(raw->imageMat.get())) {
        if (errorOut) {
            *errorOut = QStringLiteral("原图采图超时或帧为空");
        }
        return false;
    }

    if (outRaw) {
        *outRaw = std::move(raw);
    }
    return true;
}

} // namespace aoi::capture
