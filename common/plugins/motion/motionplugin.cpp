#include "motionplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginspec.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "motion_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"

#include "motion_debug_view.h"

#include <QDebug>

using namespace ExtensionSystem;

bool MotionPlugin::initialize(QString *errorString)
{
    motion::setPluginName(pluginSpec()->name());
    const PermCatalogScope catalogScope(motion::pluginName());

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = QStringLiteral(
                "MotionPlugin: IMainWindowService not found. "
                "Make sure CorePlugin is loaded first.");
        qWarning() << "[MotionPlugin] IMainWindowService not found!";
        return false;
    }

    svc->addPage(QStringLiteral("motion.motion_debug"),
                 QStringLiteral("nav.lighten.motion_debug"),
                 new motion::MotionDebugView(),
                 25);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.motion_debug"),
                                         TR("nav.lighten.motion_debug"), 25);
    PermService::instance()->registerAction(QStringLiteral("motionDebug.ioRead"),
                                            QStringLiteral("nav.lighten.motion_debug"),
                                            QStringLiteral("读入IO"));
    PermService::instance()->registerAction(QStringLiteral("motionDebug.ioWrite"),
                                            QStringLiteral("nav.lighten.motion_debug"),
                                            QStringLiteral("写出IO"));

    qDebug() << "[MotionPlugin] Motion debug page registered.";
    return true;
}
