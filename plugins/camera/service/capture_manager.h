#pragma once

/**
 * @file capture_manager.h
 * @brief 封装 DeviceManager 中 ImageCapture 实例的发现与访问
 *
 * refreshFromDeviceManager 在 delayedInitialize 调用，仅保留相机 grab 已就绪的采集器。
 * shutdown 时对所有已知采集器 stopCapture。
 */

#include <common/core/device/imageCapture/image_capture.h>

#include <memory>
#include <string>
#include <vector>

namespace common {
namespace device {
class DeviceManager;
}
} // namespace common

namespace camera_plugin {

class CaptureManager
{
public:
    /** 从 Container 获取 DeviceManager，枚举 ImageCapture 并过滤未就绪项 */
    bool refreshFromDeviceManager(std::string *errorOut = nullptr);

    std::vector<std::string> captureNames() const;
    bool                     hasCapture(const std::string &name) const;
    common::device::ImageCapture *capture(const std::string &name);
    const common::device::ImageCapture *capture(const std::string &name) const;

    void shutdown();

private:
    std::shared_ptr<common::device::DeviceManager> m_deviceManager_;
    std::vector<std::string>                       m_captureNames_;
};

} // namespace camera_plugin
