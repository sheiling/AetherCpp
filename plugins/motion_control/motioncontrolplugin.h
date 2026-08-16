#pragma once

#include "extensionsystem/iplugin.h"

class PageWithSubNav;

namespace motion_control_plugin {
class MotionControlServiceImpl;
}

class MotionControlPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "motioncontrolplugin.json")

public:
    MotionControlPlugin()           = default;
    ~MotionControlPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;
    bool delayedInitialize() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    void registerMotionPages();
    void syncMotionSubNav();

    motion_control_plugin::MotionControlServiceImpl *m_service = nullptr;
    PageWithSubNav                                    *m_motionNavContainer = nullptr;
};
