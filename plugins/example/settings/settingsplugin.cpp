#include "settingsplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"

#include <QDebug>

using namespace ExtensionSystem;

bool SettingsPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "SettingsPlugin: IMainWindowService not found.";
        qWarning() << "[SettingsPlugin] IMainWindowService not found!";
        return false;
    }

    // 注册带二级导航的设置容器页（order=30）
    PageWithSubNav *container = svc->addContainerPage("settings", "nav.settings", 30);

    container->addSubPage("general", "sub.general", new SubPage("sub.general"));
    container->addSubPage("account", "sub.account", new SubPage("sub.account"));
    container->addSubPage("privacy", "sub.privacy", new SubPage("sub.privacy"));
    container->addSubPage("about",       "sub.about",       new SubPage("sub.about"));

    qDebug() << "[SettingsPlugin] initialize: Settings pages registered.";
    return true;
}
