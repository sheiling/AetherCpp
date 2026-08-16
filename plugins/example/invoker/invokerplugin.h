#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief InvokerPlugin — Invoker 使用示例插件
 *
 * 职责：
 *  1. 创建 CalculatorService 并注册到 PluginManager 对象池
 *  2. 注册带二级导航的容器页 "invoker"（order=70），包含两个子页面：
 *       - "invoker_basic" — 基础 invoke<void/QString/int> 用法
 *       - "invoker_args"  — 多参数 & Invoker<> 类模板用法
 */
class InvokerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "invokerplugin.json")

public:
    InvokerPlugin() = default;
    ~InvokerPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    class CalculatorService *m_calcService = nullptr;
};
