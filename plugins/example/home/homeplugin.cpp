#include "homeplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"

#include <QDebug>

using namespace ExtensionSystem;

bool HomePlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    // 从对象池获取主窗口服务（由 CorePlugin 注册）
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "HomePlugin: IMainWindowService not found. "
                           "Make sure CorePlugin is loaded first.";
        qWarning() << "[HomePlugin] IMainWindowService not found!";
        return false;
    }

    // 注册首页（SimplePage，order=10，排在最左边）
    auto *homePage = new SimplePage("page.home.content");
    svc->addPage("home", "nav.home", homePage, 10);

    qDebug() << "[HomePlugin] initialize: Home page registered.";
    return true;
}
