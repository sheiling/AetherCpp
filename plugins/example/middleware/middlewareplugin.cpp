#include "middlewareplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "productview.h"

#include <QDebug>

using namespace ExtensionSystem;

bool MiddlewarePlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "MiddlewarePlugin: IMainWindowService not found.";
        qWarning() << "[MiddlewarePlugin] IMainWindowService not found!";
        return false;
    }

    // 注册 Middleware 示例页面（order=60，排在 Mvvm 页面之后）
    auto *productView = new ProductView();
    svc->addPage("middleware", "nav.middleware", productView, 60);

    qDebug() << "[MiddlewarePlugin] initialize: ProductView page registered.";
    return true;
}
