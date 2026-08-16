#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief AOI 固晶检测 UI 插件
 *
 * 向主窗口注册六个顶级页面，UI 对齐 ui_aoi HTML 原型。
 */
class AoiPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "aoiplugin.json")

public:
    AoiPlugin()  = default;
    ~AoiPlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;
    /** loadPlugins 全部完成后由 PluginManager 延迟调用，用于检测引擎预热 */
    bool delayedInitialize() override;
};
