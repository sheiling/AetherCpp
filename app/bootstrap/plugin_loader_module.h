#pragma once

#include "i_bootstrap_module.h"
#include "extensionsystem/pluginmanager.h"

#include <memory>
#include <QString>

/// 根据 BootstrapConfig 设置插件 IID 和搜索路径（在启动第一阶段中）。
class PluginLoaderModule final : public IBootstrapModule {
public:
    ~PluginLoaderModule() override;

    QString name() const override;
    int priority() const override;

    bool onRegister(BootstrapContext &ctx) override;
    bool onBoot(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;

private:
    std::unique_ptr<ExtensionSystem::PluginManager> pluginManager_;
};
