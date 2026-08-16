#pragma once

/**
 * @file visionalgoplugin.h
 * @brief VisionAlgo Qt 插件入口（ExtensionSystem::IPlugin）
 *
 * initialize 时创建 VisionAlgoServiceImpl、注册 i18n 并将服务加入 PluginManager 全局对象表。
 */

#include "extensionsystem/iplugin.h"

namespace vision_algo {
class VisionAlgoServiceImpl;
}

class VisionAlgoPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "visionalgoplugin.json")

public:
    VisionAlgoPlugin()  = default;
    ~VisionAlgoPlugin() override;

    /** 注册翻译、创建 IVisionAlgoService 并 addObject */
    bool initialize(QString *errorString) override;

    /** 全部插件加载完成后的钩子（当前无逻辑） */
    void extensionsInitialized() override;

private:
    vision_algo::VisionAlgoServiceImpl *m_service = nullptr; ///< 服务实例，生命周期随插件
};
