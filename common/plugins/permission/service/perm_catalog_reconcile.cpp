#include "service/perm_catalog_reconcile.h"

#include "data/domain/domain_types.h"
#include "model/role_model.h"

#include <QDebug>
#include <QHash>
#include <QSet>

#include <algorithm>

namespace permission {
namespace {

int deleteOrderForType(const QString &resourceType)
{
    if (resourceType == PermResourceType::Action)
        return 0;
    if (resourceType == PermResourceType::SubPage)
        return 1;
    return 2;
}

QVector<PermissionRecord> pluginOwnedRecords(const QVector<PermissionRecord> &all)
{
    QVector<PermissionRecord> out;
    out.reserve(all.size());
    for (const PermissionRecord &rec : all) {
        if (rec.source == QStringLiteral("plugin") && !rec.ownerPlugin.isEmpty())
            out.append(rec);
    }
    return out;
}

bool hardDeleteRecords(IDataStore *store, QVector<PermissionRecord> records, int *deletedOut)
{
    if (!store || records.isEmpty())
        return true;

    std::sort(records.begin(), records.end(),
              [](const PermissionRecord &a, const PermissionRecord &b) {
                  const int oa = deleteOrderForType(a.resourceType);
                  const int ob = deleteOrderForType(b.resourceType);
                  if (oa != ob)
                      return oa < ob;
                  return a.id < b.id;
              });

    int deleted = 0;
    for (const PermissionRecord &rec : records) {
        if (rec.id <= 0)
            continue;
        QString err;
        if (!store->permissions().remove(rec.id, &err)) {
            qWarning() << "[CatalogReconcile] remove failed id" << rec.id << rec.key << err;
            return false;
        }
        ++deleted;
    }
    if (deletedOut)
        *deletedOut += deleted;
    return true;
}

} // namespace

CatalogReconcileResult reconcilePermissionCatalog(IDataStore *store,
                                                  const IPluginAvailability &availability)
{
    CatalogReconcileResult result;
    if (!store)
        return result;

    const QSet<QString> deployed  = availability.deployedPluginNames();
    const QSet<QString> active    = availability.activePluginNames();
    const QSet<QString> disabled  = availability.disabledPluginNames();

    PermissionQueryFilter filter;
    filter.includeDisabled = true;
    const QVector<PermissionRecord> owned =
        pluginOwnedRecords(store->permissions().list(filter));

    QHash<QString, QVector<PermissionRecord>> byOwner;
    for (const PermissionRecord &rec : owned)
        byOwner[rec.ownerPlugin].append(rec);

    bool catalogChanged = false;

    for (auto it = byOwner.constBegin(); it != byOwner.constEnd(); ++it) {
        const QString &owner = it.key();
        if (owner.isEmpty())
            continue;

        if (!deployed.contains(owner)) {
            int deleted = 0;
            if (!hardDeleteRecords(store, it.value(), &deleted))
                break;
            result.hardDeleted += deleted;
            catalogChanged = deleted > 0;
            qDebug() << "[CatalogReconcile] removed" << deleted
                     << "resources for undeployed plugin" << owner;
            continue;
        }

        // 已部署但本次未 Running（加载/初始化失败）：保留目录与角色授权
        if (!active.contains(owner))
            continue;

        if (disabled.contains(owner)) {
            for (const PermissionRecord &rec : it.value()) {
                if (!rec.enabled)
                    continue;
                PermissionRecord updated = rec;
                updated.enabled          = false;
                QString                  err;
                if (!store->permissions().update(updated, &err)) {
                    qWarning() << "[CatalogReconcile] soft-disable failed" << rec.key << err;
                    continue;
                }
                ++result.softDisabled;
                catalogChanged = true;
            }
            continue;
        }

        for (const PermissionRecord &rec : it.value()) {
            if (rec.enabled)
                continue;
            PermissionRecord updated = rec;
            updated.enabled          = true;
            QString                  err;
            if (!store->permissions().update(updated, &err)) {
                qWarning() << "[CatalogReconcile] re-enable failed" << rec.key << err;
                continue;
            }
            ++result.reEnabled;
            catalogChanged = true;
        }
    }

    if (catalogChanged) {
        const OperationResult syncResult = syncSuperAdminGrantsFromCatalog(store);
        if (!syncResult.ok) {
            qWarning() << "[CatalogReconcile] syncSuperAdminGrantsFromCatalog failed:"
                       << syncResult.message;
        }
    }

    if (result.hardDeleted > 0 || result.softDisabled > 0 || result.reEnabled > 0) {
        qDebug() << "[CatalogReconcile] done: hardDeleted=" << result.hardDeleted
                 << "softDisabled=" << result.softDisabled << "reEnabled=" << result.reEnabled;
    }

    return result;
}

} // namespace permission
