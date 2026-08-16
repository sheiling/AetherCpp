#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief MVVM 示例插件 — 注册 UserView 页面（MVVM 架构演示）
 *
 * 该插件将 MVVM 框架（ViewModel/Command/DataBinding）和
 * UserModel/UserViewModel/UserView 一起编译进同一个 DLL。
 */
class MvvmPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "mvvmplugin.json")

public:
    MvvmPlugin() = default;
    ~MvvmPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
