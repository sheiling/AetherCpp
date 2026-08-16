#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief 首页插件 — 注册 "home" 页面到主窗口导航
 */
class HomePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "homeplugin.json")

public:
    HomePlugin() = default;
    ~HomePlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
