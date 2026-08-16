#pragma once

/**
 * @file plugin_name_slot.h
 * @brief 各插件 DLL 内缓存 pluginSpec()->name()，供 initialize 之外只读
 *
 * 每个业务插件在根目录增加 `*_plugin_name.h`：
 * @code
 * #include "extensionsystem/plugin_name_slot.h"
 * ESS_DEFINE_PLUGIN_NAME(aoi)
 * @endcode
 *
 * 插件入口（initialize / extensionsInitialized 注册页）首行：
 * @code
 * aoi::setPluginName(pluginSpec()->name());
 * const PermCatalogScope scope(aoi::pluginName());
 * @endcode
 *
 * onLoad 等延迟注册处同样使用 aoi::pluginName()，与 plugin.json Name 一致。
 */

#include <QString>

#define ESS_DEFINE_PLUGIN_NAME(Namespace)                                       \
    namespace Namespace {                                                       \
    inline QString &pluginNameSlot()                                            \
    {                                                                           \
        static QString s_name;                                                  \
        return s_name;                                                          \
    }                                                                           \
    inline void setPluginName(const QString &name) { pluginNameSlot() = name; } \
    inline QString pluginName() { return pluginNameSlot(); }                    \
    } /* namespace */
