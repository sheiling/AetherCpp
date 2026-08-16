#include "service/iplugin_availability.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginspec.h"

namespace permission {

QSet<QString> PluginAvailabilityFromManager::deployedPluginNames() const
{
    QSet<QString> names;
    for (ExtensionSystem::PluginSpec *spec : ExtensionSystem::PluginManager::plugins()) {
        if (!spec)
            continue;
        const QString name = spec->name();
        if (!name.isEmpty())
            names.insert(name);
    }
    return names;
}

QSet<QString> PluginAvailabilityFromManager::activePluginNames() const
{
    QSet<QString> names;
    for (ExtensionSystem::PluginSpec *spec : ExtensionSystem::PluginManager::plugins()) {
        if (!spec || spec->state() != ExtensionSystem::PluginSpec::Running)
            continue;
        const QString name = spec->name();
        if (!name.isEmpty())
            names.insert(name);
    }
    return names;
}

QSet<QString> PluginAvailabilityFromManager::disabledPluginNames() const
{
    // Phase 2：从 app.toml [plugins].disabled 或 PluginManager 禁用列表读取
    return {};
}

} // namespace permission
