#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief MES 对接插件
 *
 * 职责：
 *  1. 注册 MES 配置页面到主窗口导航
 *  2. 提供 MES 服务（HTTP 通信、过站校验、状态/数据上报）
 */
class MesPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "mesplugin.json")

public:
    MesPlugin()  = default;
    ~MesPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
