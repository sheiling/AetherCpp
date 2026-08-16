#pragma once

#include "extensionsystem/iplugin.h"

class GraphicsViewPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "graphicsviewplugin.json")

public:
    GraphicsViewPlugin() = default;
    ~GraphicsViewPlugin() override = default;

    bool initialize(QString* errorString) override;
    void extensionsInitialized() override {}
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    class GraphicsViewImpl *m_impl = nullptr;
};
