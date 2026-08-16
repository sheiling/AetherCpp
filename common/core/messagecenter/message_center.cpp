#include "message_center.h"
#include "message_center_p.h"

namespace MessageCenter {

static Internal::MessageCenterPrivate *s_d = nullptr;

Center::Center()
{
    s_d = new Internal::MessageCenterPrivate();
}

Center::~Center()
{
    delete s_d;
    s_d = nullptr;
}

Internal::MessageCenterPrivate *Center::d()
{
    static Center inst;
    Q_UNUSED(inst);
    return s_d;
}

void Center::Attach(const QString &notifyKey, const QObject *receiver, const char *method)
{
    d()->attach(notifyKey, receiver, method);
}

void Center::Detach(const QString &notifyKey, const QObject *receiver, const char *method)
{
    d()->detach(notifyKey, receiver, method);
}

void Center::Notify(const QString &notifyKey, const QVariantList &data)
{
    d()->notify(notifyKey, data);
}

void Center::Notify(const QString &notifyKey, const QVariant &d1)
{
    Notify(notifyKey, QVariantList{d1});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2)
{
    Notify(notifyKey, QVariantList{d1, d2});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3)
{
    Notify(notifyKey, QVariantList{d1, d2, d3});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4)
{
    Notify(notifyKey, QVariantList{d1, d2, d3, d4});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5)
{
    Notify(notifyKey, QVariantList{d1, d2, d3, d4, d5});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6)
{
    Notify(notifyKey, QVariantList{d1, d2, d3, d4, d5, d6});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6, const QVariant &d7)
{
    Notify(notifyKey, QVariantList{d1, d2, d3, d4, d5, d6, d7});
}

void Center::Notify(const QString &notifyKey, const QVariant &d1, const QVariant &d2, const QVariant &d3, const QVariant &d4, const QVariant &d5, const QVariant &d6, const QVariant &d7, const QVariant &d8)
{
    Notify(notifyKey, QVariantList{d1, d2, d3, d4, d5, d6, d7, d8});
}

namespace Internal {

MessageCenterPrivate::MessageCenterPrivate() = default;

MessageCenterPrivate::~MessageCenterPrivate() = default;

void MessageCenterPrivate::attach(const QString &notifyKey, const QObject *receiver, const char *method)
{
    if (m_mapObservers.contains(notifyKey)) {
        connect(m_mapObservers[notifyKey], SIGNAL(OnNotify(const QVariantList &)), receiver, method);
        return;
    }

    auto *obs = new MessageObserver;
    connect(obs, SIGNAL(OnNotify(const QVariantList &)), receiver, method);
    m_mapObservers.insert(notifyKey, obs);
}

void MessageCenterPrivate::detach(const QString &notifyKey, const QObject *receiver, const char *method)
{
    if (m_mapObservers.contains(notifyKey)) {
        disconnect(m_mapObservers[notifyKey], SIGNAL(OnNotify(const QVariantList &)), receiver, method);
    }
}

void MessageCenterPrivate::notify(const QString &notifyKey, const QVariantList &data)
{
    if (!m_mapObservers.contains(notifyKey))
        return;

    emit m_mapObservers[notifyKey]->OnNotify(data);
}

} // namespace Internal
} // namespace MessageCenter
