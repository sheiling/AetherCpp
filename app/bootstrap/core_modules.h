#pragma once

#include "bootstrap_config.h"
#include "i_bootstrap_module.h"

#include <memory>

class ApplicationBootstrap;

class LoggerModule final : public IBootstrapModule {
public:
    QString name() const override;
    int priority() const override;
    bool onRegister(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;
};

#if COMMON_ORM_ENABLED
class OrmModule final : public IBootstrapModule {
public:
    QString name() const override;
    int priority() const override;
    bool onRegister(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;
};
#endif

class DeviceManagerModule final : public IBootstrapModule {
public:
    QString name() const override;
    int priority() const override;
    bool onRegister(BootstrapContext &ctx) override;
    bool onBoot(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;
};

class SchedulerModule final : public IBootstrapModule {
public:
    QString name() const override;
    int priority() const override;
    bool onRegister(BootstrapContext &ctx) override;
    bool onBoot(BootstrapContext &ctx) override;
    void onShutdown(BootstrapContext &ctx) override;
};

/// 加载 config/app.toml（在 QApplication 创建前调用，argv[0] 用于解析 exe 目录）。
/// 成功后会将 [app] name / version 写回 cfg（供 QApplication 与主窗口标题使用）。
bool loadBootstrapAppConfig(BootstrapConfig &cfg, const char *argv0);

/// 关闭 config 自动重载（在 bootstrap.run 返回后调用）。
void shutdownBootstrapAppConfig(const BootstrapConfig &cfg);

/// 注册默认核心模块（Logger / Orm / DeviceManager / Scheduler / PluginLoader / SplashScreen）
void registerDefaultCoreModules(ApplicationBootstrap &bootstrap, const BootstrapConfig &config);

/// 插件加载完成后加载 Scheduler 流程 JSON（工步须在 Aoi 等插件 DLL 中已注册）
void loadSchedulerFlowsAfterPlugins();
