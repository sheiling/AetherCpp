#pragma once

#include "messagecenter_global.h"
#include "message_define.h"

#include <QObject>
#include <QString>
#include <QVariant>

namespace MessageCenter {

namespace Internal { class MessageCenterPrivate; }

class MESSAGECENTER_EXPORT Center : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------
    // 通知/观察者
    // ------------------------------------------------
    static void Attach(const QString &notifyKey, const QObject *receiver, const char *method);
    static void Detach(const QString &notifyKey, const QObject *receiver, const char *method);

    static void Notify(const QString &notifyKey, const QVariantList &data);
    static void Notify(const QString &notifyKey, const QVariant &d1);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6, const QVariant &d7);
    static void Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6, const QVariant &d7, const QVariant &d8);

private:
    Center();
    ~Center() override;

    static Internal::MessageCenterPrivate *d();
};

} // namespace MessageCenter
