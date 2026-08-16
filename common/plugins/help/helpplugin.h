#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @file helpplugin.h
 * @brief 帮助插件 — 版本信息、用户/维护手册、帮助文档（内嵌 MuPDF 渲染）
 */
class HelpPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "helpplugin.json")

public:
    HelpPlugin() = default;
    ~HelpPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;
};
