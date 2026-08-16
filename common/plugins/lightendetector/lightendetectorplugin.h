#pragma once

/**
 * @file lightedetectorplugin.h
 * @brief LightenDetector 插件入口类声明。
 *
 * 实现 ExtensionSystem::IPlugin 接口，在应用启动时完成插件初始化与步骤注册。
 */

#include "extensionsystem/iplugin.h"

// LightenDetector 插件入口，由 Qt 插件机制加载。
class LightenDetectorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "lightendetectorplugin.json")

public:
    LightenDetectorPlugin() = default;
    ~LightenDetectorPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
