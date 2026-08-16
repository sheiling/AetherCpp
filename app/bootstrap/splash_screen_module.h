#pragma once

#include "i_bootstrap_module.h"

#include <memory>

class SplashScreen;

/// 启动画面模块（priority 9，PluginLoader 之前）。
/// 内置图：app/resources/image/app_splash.ico（qrc :/resources/image/app_splash.ico）
class SplashScreenModule final : public IBootstrapModule {
public:
    SplashScreenModule();
    ~SplashScreenModule() override;
    QString name() const override;
    int priority() const override;
    bool onBoot(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;

private:
    void releaseSplash(BootstrapContext &ctx);

    std::unique_ptr<SplashScreen> splash_;
};
