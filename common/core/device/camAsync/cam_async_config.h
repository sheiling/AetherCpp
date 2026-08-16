#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QString>

namespace common::device {
class DeviceManager;
}

namespace cam_async {

/**
 * @brief 读取 exe 目录 param/device_config.json 中的相机配置
 */
class DeviceConfigReader
{
public:
    static std::vector<std::string> loadCameraKeys();
};

/** 从 IoC 容器安全解析 DeviceManager；未注册或解析失败时返回 nullptr */
std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager();

} // namespace cam_async
