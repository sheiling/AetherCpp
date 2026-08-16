#pragma once

#include "extensionsystem/iplugin.h"

class MainWindow;

/**
 * @brief 核心插件 — 负责创建主窗口并完成导航
 *
 * initialize():
 *   创建 MainWindow，将其注册到对象池（通过 Q_INTERFACES 实现 IMainWindowService）。
 *   其他插件在自己的 initialize() 中从对象池获取 IMainWindowService，注册页面。
 *
 * extensionsInitialized():
 *   所有插件的 initialize() 均已完成，调用 MainWindow::finalizeNavigation()
 *   将已注册的页面按 order 排序加入导航，并显示主窗口。
 */
class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "coreplugin.json")

public:
    CorePlugin();
    ~CorePlugin() override;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;
    bool delayedInitialize() override;
    ShutdownFlag aboutToShutdown() override;

private:
    MainWindow *m_mainWindow = nullptr;
};
