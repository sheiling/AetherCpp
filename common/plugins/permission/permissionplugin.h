#pragma once

#include "extensionsystem/iplugin.h"
#include "model/auth_session.h"

#include <memory>

namespace permission {
class PermPermissionBackend;
class PermCatalogRegistrar;
class SessionIdleGuard;
}

/**
 * @brief 权限管理插件 — 注册权限管理页面到主窗口导航
 *
 * 后续可在此插件中扩展角色管理、菜单权限配置、用户授权等业务功能。
 */
class PermissionPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "permissionplugin.json")

public:
    PermissionPlugin();
    ~PermissionPlugin() override;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;

private:
    void setupTitleBarLogin();
    void showLoginDialog();
    void setupPermService();
    void setupSessionIdleGuard();
    void runCatalogReconcile();
    void resetTitleBarUserDisplay();
    void applyTitleBarUserDisplay(const permission::AuthSession &session);
    void onSessionIdleTimeout();

    std::unique_ptr<permission::PermPermissionBackend> m_permBackend;
    std::unique_ptr<permission::PermCatalogRegistrar>    m_catalogRegistrar;
    std::unique_ptr<permission::SessionIdleGuard>        m_idleGuard;
};
