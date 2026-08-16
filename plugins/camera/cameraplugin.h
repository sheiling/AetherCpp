#pragma once

/**
 * @file cameraplugin.h
 * @brief Camera 插件入口：注册 ICameraService 到 ExtensionSystem
 *
 * 生命周期：
 * - initialize：创建 CameraServiceImpl 并 addObject（采集器尚未绑定）
 * - delayedInitialize：refreshCaptures，DeviceManager 就绪后 wire DispCB
 * - aboutToShutdown：shutdown + removeObject
 */

#include "extensionsystem/iplugin.h"

namespace camera_plugin {

class CameraServiceImpl;

} // namespace camera_plugin

class CameraPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "cameraplugin.json")

public:
    CameraPlugin()           = default;
    ~CameraPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
    bool delayedInitialize() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    camera_plugin::CameraServiceImpl *m_service = nullptr;
};
