#pragma once

#include "bootstrap_config.h"

#include <functional>

class QApplication;
class SplashScreen;

namespace ExtensionSystem {
class PluginManager;
}

/// 贯穿启动各阶段的只读/共享状态（由 ApplicationBootstrap 在 run() 内填充）。
class BootstrapContext {
public:
    BootstrapContext(int *argc, char **argv, const BootstrapConfig *config)
        : argc(argc)
        , argv(argv)
        , config(config)
    {}

    int *argc = nullptr;
    char **argv = nullptr;
    const BootstrapConfig *config = nullptr;
    QApplication *application = nullptr;
    ExtensionSystem::PluginManager *pluginManager = nullptr;

    /// 由 SplashScreenModule 创建；PluginLoaderModule 在插件加载完成后 finish 主窗口并释放。
    SplashScreen *splashScreen = nullptr;
    /// 释放启动画面（须在 MessageCenter 观察者清理前调用）。
    std::function<void()> releaseSplashScreen;
};
