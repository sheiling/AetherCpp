#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QxOrm.h>
#include <QtCore/QString>

struct PermissionResource
{
    long    id           = 0;
    long    parentId     = 0;
    QString key;
    QString resourceType;
    QString pageKey;
    QString displayName;
    QString description;
    QString source;
    QString ownerPlugin;
    int     sortOrder    = 0;
    bool    enabled      = true;
    QString createdAt;
    QString updatedAt;
};

QX_REGISTER_HPP(PermissionResource, qx::trait::no_base_class_defined, 0)

namespace qx {
template <> void register_class(QxClass<PermissionResource> &t);
}

#endif // COMMON_ORM_ENABLED
