#pragma once

#include <QSet>
#include <QString>

namespace permission {

/**
 * @brief 插件可用性策略（Catalog Reconcile 用）
 *
 * - deployedPluginNames()：插件目录扫描到的 DLL（PluginSpec 存在），不要求 Running
 * - activePluginNames()：state==Running，用于 re-enable 等运行时语义
 * - disabledPluginNames()：Phase 2 配置显式禁用
 */
class IPluginAvailability
{
public:
    virtual ~IPluginAvailability() = default;

    /// 本次部署包内扫描到的插件（含加载/初始化失败但未从目录移除的 DLL）
    virtual QSet<QString> deployedPluginNames() const = 0;
    virtual QSet<QString> activePluginNames() const   = 0;
    virtual QSet<QString> disabledPluginNames() const = 0;
};

/// 从 ExtensionSystem::PluginManager 收集插件名
class PluginAvailabilityFromManager : public IPluginAvailability
{
public:
    QSet<QString> deployedPluginNames() const override;
    QSet<QString> activePluginNames() const override;
    QSet<QString> disabledPluginNames() const override;
};

} // namespace permission
