#pragma once

#include "iperminterface.h"
#include "ipermcatalogregistrar.h"
#include "uibase_global.h"

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

class PermCatalogScope;

/**
 * @brief 全局权限运行时服务
 *
 * 运行时以 perm_permission.key 全局唯一标识资源（nav / sub_page / action 均如此）。
 * page_key、resource_type 仅用于配置期树形维护与筛选，不参与 can() 判定。
 */
class UIBASE_EXPORT PermService : public QObject
{
    Q_OBJECT

    friend class PermCatalogScope;

public:
    static PermService *instance();

    void setBackend(IPermInterface *backend);
    IPermInterface *backend() const { return m_backend; }

    void setCatalogRegistrar(IPermCatalogRegistrar *registrar);
    IPermCatalogRegistrar *catalogRegistrar() const { return m_catalogRegistrar; }

    bool isActive() const { return true; }
    bool isReady() const;
    bool isCatalogReady() const;

    qint64 effectiveUserId() const { return m_effectiveUserId; }
    qint64 effectiveRoleId() const { return m_effectiveRoleId; }

    void reloadForUser(qint64 userId);
    void reloadAsMinimalUser();
    void logout();
    void clearEffectiveGrants();

    /// 是否在 grants 中（key 为资源表全局唯一 key）
    bool can(const QString &resourceKey) const;

    /// 顶栏 nav：key 通常为 addPage 的 titleKey，如 nav.permission
    bool canNav(const QString &navTitleKey) const { return can(navTitleKey); }

    /// 子页：key 通常为 addSubPage 的 titleKey，如 sub.perm.users
    bool canSubPage(const QString &subPageKey) const { return can(subPageKey); }

    /// 操作：key 为全局唯一 action key，如 perm.users.query
    bool canAction(const QString &actionResourceKey) const { return can(actionResourceKey); }

    /// 注册顶栏 nav（Permission 未就绪时入队，插件 initialize 顺序无关）
    bool registerNav(const QString &key, const QString &displayName, int sortOrder = 0);

    /// 注册二级子页
    bool registerSubPage(const QString &key, const QString &parentNavKey, const QString &displayName,
                         int sortOrder = 0);

    /// 注册操作资源（key 须与 Command::setName 一致）
    bool registerAction(const QString &key, const QString &pageKey, const QString &displayName);

    /// 当前目录注册归属插件（PermCatalogScope 压栈顶）；空表示未设置
    QString catalogOwnerPlugin() const;

signals:
    void permissionsChanged();
    void sessionEnded();

private:
    explicit PermService(QObject *parent = nullptr);

    void applyGrants(const QSet<QString> &keys);
    void flushPendingCatalog();
    void pushCatalogOwnerPlugin(const QString &ownerPlugin);
    void popCatalogOwnerPlugin();

    struct PendingCatalogEntry {
        enum class Kind { Nav, SubPage, Action };
        Kind    kind;
        QString key;
        QString displayName;
        QString refKey;
        int     sortOrder = 0;
    };

    IPermInterface        *m_backend          = nullptr;
    IPermCatalogRegistrar *m_catalogRegistrar = nullptr;
    QVector<PendingCatalogEntry> m_pendingCatalog;
    QVector<QString>           m_catalogOwnerStack;
    qint64                 m_effectiveUserId = 0;
    qint64                 m_effectiveRoleId = 0;
    QSet<QString>          m_grants;
};

#define PS (PermService::instance())
