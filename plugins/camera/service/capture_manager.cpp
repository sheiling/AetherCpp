#include "capture_manager.h"

#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>

#include <Logger.h>

#include <algorithm>

namespace camera_plugin {

namespace {

std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app || !app->bound<common::device::DeviceManager>()) {
            return nullptr;
        }
        return app->make<common::device::DeviceManager>();
    } catch (const std::exception &e) {
        LOG_WARN("[CaptureManager] DeviceManager unavailable: {}", e.what());
        return nullptr;
    }
}

} // namespace

bool CaptureManager::refreshFromDeviceManager(std::string *errorOut)
{
    const auto dm = tryGetDeviceManager();
    if (!dm) {
        if (errorOut) {
            *errorOut = "DeviceManager not available";
        }
        return false;
    }

    m_deviceManager_.reset();
    m_captureNames_.clear();

    for (const auto &captureName : dm->getImageCaptureNameList()) {
        const auto capture = dm->ImageCaptureHandler(captureName);
        if (!capture) {
            LOG_WARN("[CaptureManager] skip missing capture {}", captureName);
            continue;
        }
        // 无 grab 回调的采集器无法预览/单帧，跳过
        if (!capture->ensureGrabCallbackRegistered()) {
            LOG_WARN("[CaptureManager] skip capture {} (camera not ready)", captureName);
            continue;
        }
        m_captureNames_.push_back(captureName);
    }

    if (m_captureNames_.empty()) {
        if (errorOut) {
            *errorOut = "No image capture available in DeviceManager";
        }
        return false;
    }

    m_deviceManager_ = std::move(dm);
    return true;
}

std::vector<std::string> CaptureManager::captureNames() const
{
    return m_captureNames_;
}

bool CaptureManager::hasCapture(const std::string &name) const
{
    return std::find(m_captureNames_.begin(), m_captureNames_.end(), name) != m_captureNames_.end();
}

common::device::ImageCapture *CaptureManager::capture(const std::string &name)
{
    if (!m_deviceManager_) {
        return nullptr;
    }
    return m_deviceManager_->ImageCaptureHandler(name).get();
}

const common::device::ImageCapture *CaptureManager::capture(const std::string &name) const
{
    if (!m_deviceManager_) {
        return nullptr;
    }
    return m_deviceManager_->ImageCaptureHandler(name).get();
}

void CaptureManager::shutdown()
{
    if (m_deviceManager_) {
        for (const auto &name : m_captureNames_) {
            if (const auto capture = m_deviceManager_->ImageCaptureHandler(name)) {
                capture->stopCapture();
            }
        }
    }
    m_deviceManager_.reset();
    m_captureNames_.clear();
}

} // namespace camera_plugin
