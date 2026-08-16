/**
 * @file helpplugin.cpp
 * @brief HelpPlugin 初始化 — 向主窗口注册帮助页（版本信息 + PDF 文档）
 */

#include "helpplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"
#include "ui/view/help_view.h"

#include <QDebug>

using namespace ExtensionSystem;

bool HelpPlugin::initialize(QString *errorString)
{
    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString) {
            *errorString = QStringLiteral(
                "HelpPlugin: IMainWindowService not found. "
                "Make sure CorePlugin is loaded first.");
        }
        qWarning() << "[HelpPlugin] IMainWindowService not found!";
        return false;
    }

    svc->addPage(QStringLiteral("help"), QStringLiteral("nav.help"), new help::HelpView(), 26);
    PermService::instance()->registerNav(QStringLiteral("nav.help"), TR("nav.help"), 26);

    qDebug() << "[HelpPlugin] Help page registered (version info + PDF manuals).";
    return true;
}

IPlugin::ShutdownFlag HelpPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}
