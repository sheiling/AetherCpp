#pragma once

#include "extensionsystem/iplugin.h"
#include "container/application.h"

#include <memory>

/**
 * @brief ContainerPlugin — Container IoC 服务容器使用示例插件
 *
 * 职责：
 *  1. 获取全局 Container::Application 实例，注册以下 ServiceProvider：
 *       - AppLoggerProvider  — 注册 IAppLogger 类型绑定 + "logger" 命名绑定
 *       - AppStorageProvider — 注册 IAppStorage 单例绑定 + "storage" 命名绑定
 *  2. 调用 bootstrap()（registerProviders → bootProviders）
 *  3. 注册带二级导航的容器页 "container"（order=80），包含两个子页面：
 *       - "container_type"  — 类型绑定演示（bind/singleton/instance/extend/contextual/tag）
 *       - "container_named" — 命名绑定演示（singletonNamed/makeNamed/call/get/set）
 */
class ContainerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "containerplugin.json")

public:
    ContainerPlugin();
    ~ContainerPlugin() override;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}

private:
    /// 全局 Container::Application 实例（生命周期为进程级）
    Container::Application* app_ = nullptr;
};
