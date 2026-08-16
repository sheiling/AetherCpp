#pragma once

#ifdef COMMON_ORM_ENABLED

#include <QxOrm.h>
#include <QtCore/QString>

struct UserAccount
{
    long    id            = 0;
    QString username;
    QString passwordHash;
    QString displayName;
    QString department;
    int     status        = 1;
    QString lastLoginAt;
    QString createdAt;
    QString updatedAt;
    QString remark;
    bool    isHidden    = false;
};

QX_REGISTER_HPP(UserAccount, qx::trait::no_base_class_defined, 0)

namespace qx {
template <> void register_class(QxClass<UserAccount> &t);
}

#endif // COMMON_ORM_ENABLED
