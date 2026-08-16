#include "service/perm_catalog_registrar.h"

#include "data/domain/domain_types.h"
#include "model/role_model.h"
#include "repository/datastore.h"
#include "service/perm_system_constants.h"
#include "uibase/permission/permservice.h"

#include <QDebug>

namespace permission {
namespace {

bool findParentId(IDataStore *store, const QString &parentKey, qint64 *parentIdOut)
{
    if (!store || !parentIdOut || parentKey.isEmpty())
        return false;

    PermissionRecord parent;
    if (!store->permissions().findByKey(parentKey, parent))
        return false;

    *parentIdOut = parent.id;
    return parent.id > 0;
}

} // namespace

PermCatalogRegistrar::PermCatalogRegistrar(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

bool PermCatalogRegistrar::isReady() const
{
    return m_store != nullptr;
}

bool PermCatalogRegistrar::registerNav(const QString &key, const QString &displayName, int sortOrder)
{
    return upsertResource(PermResourceType::Nav, key, 0, QString::fromUtf8(kPermPageKeyRoot),
                          displayName, sortOrder);
}

bool PermCatalogRegistrar::registerSubPage(const QString &key, const QString &parentNavKey,
                                           const QString &displayName, int sortOrder)
{
    qint64 parentId = 0;
    if (!findParentId(m_store, parentNavKey, &parentId)) {
        qWarning() << "[PermCatalogRegistrar] parent nav not found for sub_page" << key
                   << "parent" << parentNavKey;
        return false;
    }
    return upsertResource(PermResourceType::SubPage, key, parentId, parentNavKey, displayName,
                          sortOrder);
}

bool PermCatalogRegistrar::registerAction(const QString &key, const QString &pageKey,
                                            const QString &displayName)
{
    qint64 parentId = 0;
    if (!findParentId(m_store, pageKey, &parentId)) {
        qWarning() << "[PermCatalogRegistrar] parent page not found for action" << key << "page"
                   << pageKey;
        return false;
    }
    return upsertResource(PermResourceType::Action, key, parentId, pageKey, displayName, 0);
}

bool PermCatalogRegistrar::upsertResource(const QString &resourceType, const QString &key,
                                          qint64 parentId, const QString &pageKey,
                                          const QString &displayName, int sortOrder)
{
    if (!m_store || key.isEmpty())
        return false;

    PermissionRecord existing;
    const QString      owner = PermService::instance()->catalogOwnerPlugin();
    if (m_store->permissions().findByKey(key, existing))
        return true;

    PermissionRecord rec;
    rec.parentId     = parentId;
    rec.key          = key;
    rec.resourceType = resourceType;
    rec.pageKey      = pageKey;
    rec.displayName  = displayName;
    rec.source       = QStringLiteral("plugin");
    rec.ownerPlugin  = owner;
    rec.sortOrder    = sortOrder;
    rec.enabled      = true;

    QString err;
    if (!m_store->permissions().insert(rec, &err)) {
        if (err == QStringLiteral("key_duplicate"))
            return true;
        qWarning() << "[PermCatalogRegistrar] insert failed for" << key << err;
        return false;
    }

    const OperationResult syncResult = syncSuperAdminGrantsFromCatalog(m_store);
    if (!syncResult.ok) {
        qWarning() << "[PermCatalogRegistrar] syncSuperAdminGrantsFromCatalog failed:"
                   << syncResult.message;
    }
    return true;
}

} // namespace permission
