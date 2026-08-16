#include "motioncontrolplugin.h"

#include "service/motion_control_service_impl.h"
#include "ui/motion_axis_settings_view.h"
#include "ui/motion_axis_debug_view.h"
#include "ui/motion_io_settings_view.h"
#include "motion_action_keys.h"
#include "imotion_control_service.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginspec.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "motioncontrol_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"

#include <QDebug>

using namespace ExtensionSystem;

bool MotionControlPlugin::initialize(QString *errorString)
{
    LM->registerPluginTranslator(QStringLiteral("motion_control"), QStringLiteral("motion_control"));

    m_service = new motion_control_plugin::MotionControlServiceImpl(this);
    PluginManager::addObject(m_service);
    motion_control::setPluginName(pluginSpec()->name());
    qDebug() << "[MotionControlPlugin] IMotionControlService registered (init deferred)";
    Q_UNUSED(errorString);
    return true;
}

void MotionControlPlugin::extensionsInitialized()
{
    // 须在 CorePlugin::finalizeNavigation() 之前注册（Running 阶段逆序执行，本插件先于 Core）。
    registerMotionPages();
}

bool MotionControlPlugin::delayedInitialize()
{
    if (!m_service)
        return false;

    QString loadError;
    if (!m_service->initialize(&loadError)) {
        qWarning() << "[MotionControlPlugin]" << loadError;
        return false;
    }

    qDebug() << "[MotionControlPlugin] ready, axes:" << m_service->axisNames().size()
             << "ios:" << m_service->ioNames().size();

    syncMotionSubNav();
    connect(m_service, &motion_control_plugin::IMotionControlService::catalogChanged, this,
            &MotionControlPlugin::syncMotionSubNav);

    return false;
}

void MotionControlPlugin::registerMotionPages()
{
    const PermCatalogScope catalogScope(motion_control::pluginName());

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        qWarning() << "[MotionControlPlugin] IMainWindowService not found, skip UI pages";
        return;
    }

    PageWithSubNav *container = svc->addContainerPage(QStringLiteral("motion.nav"),
                                                      QStringLiteral("motion.nav"), 23);
    m_motionNavContainer      = container;
    PermService::instance()->registerNav(QStringLiteral("motion.nav"),
                                         QStringLiteral("motion.nav"), 23);

    container->addSubPage(QStringLiteral("motion.sub.axis_motion"),
                          QStringLiteral("motion.sub.axis_motion"),
                          new motion_control_plugin::MotionAxisDebugView());
    PermService::instance()->registerSubPage(QStringLiteral("motion.sub.axis_motion"),
                                             QStringLiteral("motion.nav"),
                                             QStringLiteral("motion.sub.axis_motion"), 1);

    container->addSubPage(QStringLiteral("motion.sub.io_params"),
                          QStringLiteral("motion.sub.io_params"),
                          new motion_control_plugin::MotionIoSettingsView());
    PermService::instance()->registerSubPage(QStringLiteral("motion.sub.io_params"),
                                             QStringLiteral("motion.nav"),
                                             QStringLiteral("motion.sub.io_params"), 2);

    container->addSubPage(QStringLiteral("motion.sub.axis_params"),
                          QStringLiteral("motion.sub.axis_params"),
                          new motion_control_plugin::MotionAxisSettingsView());
    PermService::instance()->registerSubPage(QStringLiteral("motion.sub.axis_params"),
                                             QStringLiteral("motion.nav"),
                                             QStringLiteral("motion.sub.axis_params"), 3);

    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::IoEdit),
        QStringLiteral("motion.sub.io_params"), QStringLiteral("motion.action.io_edit"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::IoWrite),
        QStringLiteral("motion.sub.io_params"), QStringLiteral("motion.action.io_write"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::AxisSave),
        QStringLiteral("motion.sub.axis_params"), QStringLiteral("motion.action.axis_save"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::AxisAdd),
        QStringLiteral("motion.sub.axis_params"), QStringLiteral("motion.action.axis_add"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::AxisDelete),
        QStringLiteral("motion.sub.axis_params"), QStringLiteral("motion.action.axis_delete"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::IoAdd),
        QStringLiteral("motion.sub.io_params"), QStringLiteral("motion.action.io_add"));
    PermService::instance()->registerAction(
        QString::fromUtf8(motion_control_plugin::MotionActionKeys::IoDelete),
        QStringLiteral("motion.sub.io_params"), QStringLiteral("motion.action.io_delete"));

    qDebug() << "[MotionControlPlugin] Motion container pages registered.";
}

void MotionControlPlugin::syncMotionSubNav()
{
    if (!m_motionNavContainer || !m_service)
        return;

    const bool hasAxis = !m_service->axisNames().isEmpty();
    const bool hasIo   = !m_service->ioNames().isEmpty();

    m_motionNavContainer->setSubItemVisible(QStringLiteral("motion.sub.axis_motion"), hasAxis);
    m_motionNavContainer->setSubItemVisible(QStringLiteral("motion.sub.axis_params"), hasAxis);
    m_motionNavContainer->setSubItemVisible(QStringLiteral("motion.sub.io_params"), hasIo);
}

IPlugin::ShutdownFlag MotionControlPlugin::aboutToShutdown()
{
    if (m_service) {
        PluginManager::removeObject(m_service);
        m_service->shutdown();
        m_service->deleteLater();
        m_service = nullptr;
    }
    return IPlugin::SynchronousShutdown;
}
