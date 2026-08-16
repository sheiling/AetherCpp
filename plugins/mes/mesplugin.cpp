#include "mesplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "ui/view/mes_config_view.h"

#include <QDebug>

using namespace ExtensionSystem;

bool MesPlugin::initialize(QString *errorString)
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = QStringLiteral(
                "MesPlugin: IMainWindowService not found. "
                "Make sure CorePlugin is loaded first.");
        qWarning() << "[MesPlugin] IMainWindowService not found!";
        return false;
    }

    svc->addPage(QStringLiteral("mes.config"),
                 QStringLiteral("nav.lighten.mes"),
                 new mes::MesConfigView(),
                 27);

    qDebug() << "[MesPlugin] MES config page registered.";
    return true;
}
