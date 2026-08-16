#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief 设置页插件 — 注册带四个子页面的 "settings" 容器页
 */
class SettingsPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "settingsplugin.json")

public:
    SettingsPlugin() = default;
    ~SettingsPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
