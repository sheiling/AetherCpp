#pragma once

#include "repository/interface/idatastore.h"
#include "uibase/permission/ipermcatalogregistrar.h"

namespace permission {

class PermCatalogRegistrar : public IPermCatalogRegistrar
{
public:
    explicit PermCatalogRegistrar(IDataStore *store = nullptr);

    bool isReady() const override;

    bool registerNav(const QString &key, const QString &displayName, int sortOrder = 0) override;

    bool registerSubPage(const QString &key, const QString &parentNavKey, const QString &displayName,
                         int sortOrder = 0) override;

    bool registerAction(const QString &key, const QString &pageKey,
                        const QString &displayName) override;

private:
    bool upsertResource(const QString &resourceType, const QString &key, qint64 parentId,
                        const QString &pageKey, const QString &displayName, int sortOrder);

    IDataStore *m_store = nullptr;
};

} // namespace permission
