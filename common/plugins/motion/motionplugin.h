#pragma once

#include "extensionsystem/iplugin.h"

/** 运控调试 UI 插件（顶级页插在虚拟像素与帮助之间） */
class MotionPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "motionplugin.json")

public:
    MotionPlugin()  = default;
    ~MotionPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
