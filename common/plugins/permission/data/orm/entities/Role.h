#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QxOrm.h>
#include <QtCore/QString>

struct Role
{
    long    id          = 0;
    QString code;
    QString name;
    QString description;
    bool    isSystem    = false;
    bool    isHidden    = false;
    QString tagColor;
    int     sortOrder   = 0;
    bool    enabled     = true;
    QString createdAt;
    QString updatedAt;
};

QX_REGISTER_HPP(Role, qx::trait::no_base_class_defined, 0)

namespace qx {
template <> void register_class(QxClass<Role> &t);
}

#endif // COMMON_ORM_ENABLED
