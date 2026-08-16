#include "permservice.h"

#include <QDebug>

namespace {

constexpr auto kMinimalRole = "minimal";

} // namespace

PermService *PermService::instance()
{
    static PermService inst;
    return &inst;
}

PermService::PermService(QObject *parent)
    : QObject(parent)
{
}

void PermService::setBackend(IPermInterface *backend)
{
    if (m_backend == backend)
        return;
    m_backend = backend;
    clearEffectiveGrants();
}

bool PermService::isReady() const
{
    return m_backend && m_backend->isReady();
}

bool PermService::isCatalogReady() const
{
    return m_catalogRegistrar && m_catalogRegistrar->isReady();
}

void PermService::setCatalogRegistrar(IPermCatalogRegistrar *registrar)
{
    if (m_catalogRegistrar == registrar)
        return;
    m_catalogRegistrar = registrar;
    flushPendingCatalog();
}

bool PermService::registerNav(const QString &key, const QString &displayName, int sortOrder)
{
    if (key.isEmpty())
        return false;
    if (m_catalogRegistrar && m_catalogRegistrar->isReady())
        return m_catalogRegistrar->registerNav(key, displayName, sortOrder);

    PendingCatalogEntry entry;
    entry.kind        = PendingCatalogEntry::Kind::Nav;
    entry.key         = key;
    entry.displayName = displayName;
    entry.sortOrder   = sortOrder;
    m_pendingCatalog.append(entry);
    return true;
}

bool PermService::registerSubPage(const QString &key, const QString &parentNavKey,
                                  const QString &displayName, int sortOrder)
{
    if (key.isEmpty() || parentNavKey.isEmpty())
        return false;
    if (m_catalogRegistrar && m_catalogRegistrar->isReady())
        return m_catalogRegistrar->registerSubPage(key, parentNavKey, displayName, sortOrder);

    PendingCatalogEntry entry;
    entry.kind        = PendingCatalogEntry::Kind::SubPage;
    entry.key         = key;
    entry.displayName = displayName;
    entry.refKey      = parentNavKey;
    entry.sortOrder   = sortOrder;
    m_pendingCatalog.append(entry);
    return true;
}

bool PermService::registerAction(const QString &key, const QString &pageKey,
                                 const QString &displayName)
{
    if (key.isEmpty() || pageKey.isEmpty())
        return false;
    if (m_catalogRegistrar && m_catalogRegistrar->isReady())
        return m_catalogRegistrar->registerAction(key, pageKey, displayName);

    PendingCatalogEntry entry;
    entry.kind        = PendingCatalogEntry::Kind::Action;
    entry.key         = key;
    entry.displayName = displayName;
    entry.refKey      = pageKey;
    m_pendingCatalog.append(entry);
    return true;
}

void PermService::flushPendingCatalog()
{
    if (!m_catalogRegistrar || !m_catalogRegistrar->isReady() || m_pendingCatalog.isEmpty())
        return;

    const QVector<PendingCatalogEntry> batch = m_pendingCatalog;
    m_pendingCatalog.clear();

    for (const PendingCatalogEntry &entry : batch) {
        switch (entry.kind) {
        case PendingCatalogEntry::Kind::Nav:
            m_catalogRegistrar->registerNav(entry.key, entry.displayName, entry.sortOrder);
            break;
        case PendingCatalogEntry::Kind::SubPage:
            m_catalogRegistrar->registerSubPage(entry.key, entry.refKey, entry.displayName,
                                                entry.sortOrder);
            break;
        case PendingCatalogEntry::Kind::Action:
            m_catalogRegistrar->registerAction(entry.key, entry.refKey, entry.displayName);
            break;
        }
    }
}

QString PermService::catalogOwnerPlugin() const
{
    return m_catalogOwnerStack.isEmpty() ? QString() : m_catalogOwnerStack.constLast();
}

void PermService::pushCatalogOwnerPlugin(const QString &ownerPlugin)
{
    if (!ownerPlugin.isEmpty())
        m_catalogOwnerStack.append(ownerPlugin);
}

void PermService::popCatalogOwnerPlugin()
{
    if (!m_catalogOwnerStack.isEmpty())
        m_catalogOwnerStack.removeLast();
}

void PermService::reloadForUser(qint64 userId)
{
    if (!m_backend || !m_backend->isReady() || userId <= 0) {
        clearEffectiveGrants();
        return;
    }

    qint64 roleId = 0;
    if (!m_backend->primaryRoleIdForUser(userId, &roleId) || roleId <= 0) {
        qWarning() << "[PermService] No primary role for user" << userId;
        clearEffectiveGrants();
        return;
    }

    m_effectiveUserId = userId;
    m_effectiveRoleId = roleId;
    applyGrants(m_backend->loadGrantKeysForRole(roleId));

    qDebug() << "[PermService] reloadForUser" << userId << "role" << roleId << "grants"
             << m_grants.size();
    emit permissionsChanged();
}

void PermService::logout()
{
    if (m_backend)
        m_backend->clearAuthSession();
    reloadAsMinimalUser();
    emit sessionEnded();
    qDebug() << "[PermService] User logged out, switched to minimal user grants.";
}

void PermService::reloadAsMinimalUser()
{
    if (!m_backend || !m_backend->isReady()) {
        clearEffectiveGrants();
        return;
    }

    qint64 userId = 0;
    if (!m_backend->findUserIdByRoleCode(QString::fromUtf8(kMinimalRole), &userId) || userId <= 0) {
        qWarning() << "[PermService] Minimal user not found for role code" << kMinimalRole;
        clearEffectiveGrants();
        return;
    }

    reloadForUser(userId);
}

void PermService::clearEffectiveGrants()
{
    const bool hadState = m_effectiveUserId > 0 || !m_grants.isEmpty();
    m_effectiveUserId = 0;
    m_effectiveRoleId = 0;
    m_grants.clear();
    if (hadState)
        emit permissionsChanged();
}

bool PermService::can(const QString &resourceKey) const
{
    if (!isReady() || resourceKey.isEmpty())
        return false;
    return m_grants.contains(resourceKey);
}

void PermService::applyGrants(const QSet<QString> &keys)
{
    m_grants = keys;
}
