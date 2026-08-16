#pragma once

/**
 * @file service_access.h
 * @brief 获取已加载 VisionAlgo 插件注册的全局 IVisionAlgoService 及启动自检辅助函数
 */

#include <ivision_algo_service.h>
#include <vision_algo_plugin_global.h>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>

#include <QString>

namespace vision_algo {

/** 从 ExtensionSystem 获取 VisionAlgo 服务实例；插件未加载时返回 nullptr */
inline IVisionAlgoService *service()
{
    return ExtensionSystem::PluginManager::getInterface<IVisionAlgoService>();
}

/**
 * 启动期综合自检：服务就绪、固晶单元注册、model/marker SDK 加载状态。
 * @return 空字符串表示无问题；否则为多行中文/翻译错误说明。
 */
VISION_ALGO_EXPORT QString startupIssue();

/**
 * model_processor SDK 专项自检。
 * @return 空表示 SDK 可用；否则为错误说明（可含插件未加载信息）。
 */
VISION_ALGO_EXPORT QString modelSdkIssue();

/**
 * model_processor 与 marker_detector SDK 联合自检（配方 Mark 流程用）。
 * @return 空表示两者均可用。
 */
VISION_ALGO_EXPORT QString markModelSdkIssue();

/**
 * 将配方中的 .tpl 相对路径解析为运行时 .model 绝对路径。
 * @param templateFile 配方内模板路径（可为 .tpl 或已解析路径）
 */
VISION_ALGO_EXPORT std::string resolveModelFilePath(const std::string &templateFile);

} // namespace vision_algo
