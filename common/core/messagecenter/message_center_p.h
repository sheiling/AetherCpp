#pragma once

#include "message_observer.h"

#include <QMap>
#include <QObject>
#include <QVariant>

namespace MessageCenter {

namespace Internal {

class MessageCenterPrivate : public QObject
{
public:
    MessageCenterPrivate();
    ~MessageCenterPrivate() override;

    void attach(const QString &notifyKey, const QObject *receiver, const char *method);
    void detach(const QString &notifyKey, const QObject *receiver, const char *method);
    void notify(const QString &notifyKey, const QVariantList &data);

    QMap<QString, MessageObserver *> m_mapObservers;
};

} // namespace Internal
} // namespace MessageCenter
