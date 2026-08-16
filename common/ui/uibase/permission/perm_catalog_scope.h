#pragma once

#include "uibase_global.h"

#include <QString>

/**
 * @brief 权限目录注册作用域：将后续 registerNav/SubPage/Action 归属到指定插件
 *
 * 在业务插件 initialize()（或 extensionsInitialized 注册页）入口构造：
 * @code
 * PermCatalogScope scope(pluginSpec()->name());
 * PermService::instance()->registerNav(...);
 * @endcode
 *
 * 延迟注册（如 PageWidget::onLoad 创建的 ViewModel）使用插件内 `setPluginName` /
 * `pluginName()`（见 `extensionsystem/plugin_name_slot.h` 与 `*_plugin_name.h`）。
 *
 * owner 与 PluginSpec::name() / plugin.json 的 Name 一致，供 Catalog Reconcile 对齐。
 */
class UIBASE_EXPORT PermCatalogScope
{
public:
    explicit PermCatalogScope(const QString &ownerPlugin);
    ~PermCatalogScope();

    PermCatalogScope(const PermCatalogScope &)            = delete;
    PermCatalogScope &operator=(const PermCatalogScope &) = delete;

private:
    bool m_active = false;
};
