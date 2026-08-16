#pragma once

/**
 * @file lightenplugin.h
 * @brief 点亮检测配置界面插件入口
 */

#include "extensionsystem/iplugin.h"

class LightenInspectionHistoryServiceImpl;
class LightenProductionKpiServiceImpl;

// Lighten 插件图像根目录路径变更事件键（MessageCenter::Center::Notify / Attach / Detach）；参数：QVariantList[0] = 图像根目录路径（QString）
#define EVENT_LIGHTEN_IMG_PATH "event-lighten-img-path"
// 系统配置通用设置中对插次数变更；参数：QVariantList[0]=当前使用次数, [1]=新件使用寿命次数
#define EVENT_LIGHTEN_GENERAL_SETTINGS_CHANGED "event-lighten-general-settings-changed"
// 系统配置保存成功（含检测项开关）；无参数
#define EVENT_LIGHTEN_SYSTEM_CONFIG_SAVED "event-lighten-system-config-saved"

/**
 * @brief 点亮检测配置界面插件
 *
 * 向主窗口注册配置页面：首页 / 系统配置 / 产品配方 / 检测结果 / 虚拟像素 / 帮助。
 * 产线 IO 工步见 step/；检测算法与检测工步由 LightenDetector 功能插件提供。
 */
class LightenPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID FILE "lightenplugin.json")

public:
    LightenPlugin()  = default;
    ~LightenPlugin() override = default;

    /** 向主窗口注册首页、系统配置、配方、检测结果、虚拟像素、帮助等页面 */
    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
    /** 延迟初始化相机异步采集模块 cam_async */
    bool delayedInitialize() override;
    /** 插件关闭前释放 cam_async 资源 */
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    LightenInspectionHistoryServiceImpl *m_inspectionHistoryService = nullptr;
    LightenProductionKpiServiceImpl *m_productionKpiService = nullptr;
};

