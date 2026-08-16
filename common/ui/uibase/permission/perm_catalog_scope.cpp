#include "perm_catalog_scope.h"

#include "permservice.h"

PermCatalogScope::PermCatalogScope(const QString &ownerPlugin)
{
    if (ownerPlugin.isEmpty())
        return;
    PermService::instance()->pushCatalogOwnerPlugin(ownerPlugin);
    m_active = true;
}

PermCatalogScope::~PermCatalogScope()
{
    if (m_active)
        PermService::instance()->popCatalogOwnerPlugin();
}
