#include "permissionplugin.h"

#include "model/auth_service.h"
#include "model/auth_session.h"
#include "repository/datastore.h"
#include "repository/storageconfig.h"
#include "service/perm_system_constants.h"
#include "session/session_idle_guard.h"
#include "ui/view/audit_log/audit_log_management_view.h"
#include "ui/view/login/login_dialog.h"
#include "ui/view/perm_resource/permission_resource_management_view.h"
#include "ui/view/role_manage/role_management_view.h"
#include "ui/view/user_manage/user_management_view.h"

#include "extensionsystem/pluginmanager.h"
#include "service/audit_log_writer.h"
#include "service/perm_permission_backend.h"
#include "service/perm_catalog_registrar.h"
#include "service/perm_catalog_reconcile.h"
#include "service/iplugin_availability.h"
#include "command_audit_registry.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"

#include "ui/view/perm_text.h"

#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QTimer>

using namespace ExtensionSystem;

namespace {

/// 测试切换：true = MySQL，false = SQLite（改完需重新编译）
constexpr bool kUseMysqlStorage = false;

} // namespace

PermissionPlugin::PermissionPlugin() = default;

PermissionPlugin::~PermissionPlugin() = default;

bool PermissionPlugin::initialize(QString *errorString)
{
    LM->registerPluginTranslator(QStringLiteral("permission"), QStringLiteral("permission"));

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = QStringLiteral(
                "PermissionPlugin: IMainWindowService not found. "
                "Make sure CorePlugin is loaded first.");
        qWarning() << "[PermissionPlugin] IMainWindowService not found!";
        return false;
    }

    if (kUseMysqlStorage) {
        // 本机 MySQL 连接参数（按你的环境改密码等）
        permission::DataStore::configure(permission::StorageConfig::mysql(
            QStringLiteral("ess_permission"), QStringLiteral("127.0.0.1"), 3306,
            QStringLiteral("root"), QStringLiteral("123456"))); // 密码按本机 MySQL 修改
    } else {
        QString dbPath = QDir(QCoreApplication::applicationDirPath())
                             .filePath(QStringLiteral("data/permission.db"));
        dbPath = QDir::cleanPath(dbPath);
        permission::DataStore::configure(permission::StorageConfig::sqlite(dbPath));
    }

    QString storageErr;
    if (!permission::DataStore::initialize(&storageErr)) {
        qWarning() << "[PermissionPlugin] Permission storage init failed:" << storageErr;
        if (errorString)
            *errorString = storageErr;
        return false;
    }

    const auto &activeCfg = permission::DataStore::currentConfig();
    if (activeCfg.backend == permission::StorageBackend::MySql) {
        qDebug() << "[PermissionPlugin] Permission storage ready, backend: mysql"
                 << "database=" << activeCfg.dbConfig.database
                 << "host=" << activeCfg.dbConfig.host << "port=" << activeCfg.dbConfig.port;
    } else {
        qDebug() << "[PermissionPlugin] Permission storage ready, backend: sqlite"
                 << "sqliteFile=" << activeCfg.sqliteFilePath;
    }

    PageWithSubNav *container = svc->addContainerPage("permission", "nav.permission", 35);
    container->addSubPage("perm_users", "sub.perm.users", new permission::UserManagementView());
    container->addSubPage("perm_roles", "sub.perm.roles", new permission::RoleManagementView());
    container->addSubPage("perm_resources", "sub.perm.resources",
                          new permission::PermissionResourceManagementView());
    container->addSubPage("perm_audit", "sub.perm.audit",
                          new permission::AuditLogManagementView());

    qDebug() << "[PermissionPlugin] Permission management pages registered.";

    permission::AuditLogWriter::registerCallbacks();
    setupPermService();
    return true;
}

void PermissionPlugin::extensionsInitialized()
{
    setupTitleBarLogin();
    setupSessionIdleGuard();
    resetTitleBarUserDisplay();

    qDebug() << "[PermissionPlugin] extensionsInitialized. PermService ready="
             << PermService::instance()->isReady();

    // 延后到事件循环：等全部插件 extensionsInitialized（含 MotionControl 注册页）完成后再对齐目录
    QTimer::singleShot(0, this, [this]() { runCatalogReconcile(); });
}

void PermissionPlugin::runCatalogReconcile()
{
    permission::IDataStore *store = permission::DataStore::instance();
    if (!store) {
        qWarning() << "[PermissionPlugin] Catalog reconcile skipped: no data store";
        return;
    }

    permission::PluginAvailabilityFromManager availability;
    const permission::CatalogReconcileResult result =
        permission::reconcilePermissionCatalog(store, availability);
    Q_UNUSED(result);
}

void PermissionPlugin::setupPermService()
{
    permission::AuthService::clearSession();

    m_permBackend = std::make_unique<permission::PermPermissionBackend>();
    m_catalogRegistrar = std::make_unique<permission::PermCatalogRegistrar>();
    PermService *ps = PermService::instance();
    ps->setBackend(m_permBackend.get());
    ps->setCatalogRegistrar(m_catalogRegistrar.get());
    ps->reloadAsMinimalUser();
    qDebug() << "[PermissionPlugin] PermService backend registered, minimal user loaded (no login session).";
}

void PermissionPlugin::setupSessionIdleGuard()
{
    m_idleGuard = std::make_unique<permission::SessionIdleGuard>(this);
    m_idleGuard->setIdleTimeoutMs(permission::kSessionIdleTimeoutMs);
    m_idleGuard->installGlobal();

    connect(m_idleGuard.get(), &permission::SessionIdleGuard::idleTimeout,
            this, &PermissionPlugin::onSessionIdleTimeout);

    PermService *ps = PermService::instance();
    connect(ps, &PermService::sessionEnded, this, [this]() {
        if (m_idleGuard)
            m_idleGuard->setSessionActive(false);
    });

    qDebug() << "[PermissionPlugin] Session idle guard installed, timeout ms="
             << permission::kSessionIdleTimeoutMs;
}

void PermissionPlugin::resetTitleBarUserDisplay()
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc)
        return;
    QWidget *mw = svc->mainWindowWidget();
    if (!mw)
        return;
    mw->setProperty("currentUserDisplayName", QVariant());
    QMetaObject::invokeMethod(mw, "updateCurrentUserLabel", Qt::QueuedConnection);
}

void PermissionPlugin::applyTitleBarUserDisplay(const permission::AuthSession &session)
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc)
        return;
    QWidget *mw = svc->mainWindowWidget();
    if (!mw)
        return;
    const QString name =
        session.displayName.isEmpty() ? session.username : session.displayName;
    mw->setProperty("currentUserDisplayName", name);
    QMetaObject::invokeMethod(mw, "updateCurrentUserLabel", Qt::QueuedConnection);
}

void PermissionPlugin::onSessionIdleTimeout()
{
    const permission::AuthSession session = permission::AuthService::currentSession();
    if (!session.isValid())
        return;

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    QWidget            *parent = svc ? svc->mainWindowWidget() : nullptr;

    ManualAuditEvent audit;
    audit.actionType  = QStringLiteral("auth");
    audit.actionName  = QStringLiteral("idle_timeout_logout");
    audit.target      = QStringLiteral("user:%1").arg(session.username);
    audit.moduleCode  = QStringLiteral("auth");
    permission::AuditLogWriter::appendManualAudit(audit);

    PermService::instance()->logout();
    resetTitleBarUserDisplay();

    QMessageBox::information(parent,
                             PTXT("perm.login.session_timeout_title"),
                             PTXT("perm.login.session_idle_logout"));
    qDebug() << "[PermissionPlugin] Session idle timeout: logged out to minimal user.";
}

void PermissionPlugin::setupTitleBarLogin()
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc)
        return;

    QWidget *mw = svc->mainWindowWidget();
    if (!mw)
        return;

    auto *loginBtn = mw->findChild<QPushButton *>(QStringLiteral("loginButton"));
    if (!loginBtn) {
        qWarning() << "[PermissionPlugin] Title bar login button not found.";
        return;
    }

    connect(loginBtn, &QPushButton::clicked, mw, [this]() { showLoginDialog(); });
    qDebug() << "[PermissionPlugin] Title bar login button connected.";
}

void PermissionPlugin::showLoginDialog()
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    QWidget            *parent = svc ? svc->mainWindowWidget() : nullptr;

    permission_ui::LoginDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted) {
        qDebug() << "[PermissionPlugin] Login dialog closed without signing in.";
        return;
    }

    const permission::AuthSession session = permission::AuthService::currentSession();
    if (!session.isValid()) {
        qWarning() << "[PermissionPlugin] Login dialog accepted but session invalid.";
        return;
    }

    applyTitleBarUserDisplay(session);

    if (m_idleGuard)
        m_idleGuard->setSessionActive(true);

    qDebug() << "[PermissionPlugin] Login succeeded, user=" << session.username;
}
