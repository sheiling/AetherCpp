#include "cam_async_config.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>

#include <fstream>

namespace cam_async {

namespace {

std::string deviceConfigFilePath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("config/device_config.json"))
        .toStdString();
}

nlohmann::json loadDeviceConfigCameraObject()
{
    const std::string configPath = deviceConfigFilePath();
    std::ifstream file(configPath);
    if (!file.is_open()) {
        qWarning() << "[CamAsync::DeviceConfigReader] Cannot open device config:"
                   << QString::fromStdString(configPath);
        return {};
    }

    nlohmann::json root;
    try {
        file >> root;
    } catch (const std::exception &e) {
        qWarning() << "[CamAsync::DeviceConfigReader] device_config.json parse error:" << e.what();
        return {};
    }

    if (!root.is_object()) {
        qWarning() << "[CamAsync::DeviceConfigReader] device_config.json root must be an object.";
        return {};
    }

    const nlohmann::json &deviceManager = root.value("DeviceManager", nlohmann::json::object());
    const nlohmann::json &camera = deviceManager.value("Camera", nlohmann::json::object());
    return camera.is_object() ? camera : nlohmann::json::object();
}

} // namespace

std::vector<std::string> DeviceConfigReader::loadCameraKeys()
{
    const nlohmann::json camera = loadDeviceConfigCameraObject();
    std::vector<std::string> keys;
    keys.reserve(camera.size());
    for (auto it = camera.begin(); it != camera.end(); ++it) {
        keys.push_back(it.key());
    }
    return keys;
}

std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app->bound<common::device::DeviceManager>())
            return nullptr;
        return app->make<common::device::DeviceManager>();
    } catch (const std::exception &e) {
        qWarning() << "[CamAsync] DeviceManager unavailable:" << e.what();
        return nullptr;
    }
}

} // namespace cam_async
