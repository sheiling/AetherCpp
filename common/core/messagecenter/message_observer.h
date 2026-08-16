#pragma once

#include "messagecenter_global.h"

#include <QObject>
#include <QVariant>

namespace MessageCenter {

class MESSAGECENTER_EXPORT MessageObserver : public QObject
{
    Q_OBJECT

public:
    MessageObserver() = default;

signals:
    void OnNotify(const QVariantList &data);
};

} // namespace MessageCenter
