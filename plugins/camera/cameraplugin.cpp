/**
 * @file cameraplugin.cpp
 * @brief Camera 插件 ExtensionSystem 生命周期实现
 */

#include "cameraplugin.h"

#include "service/camera_service_impl.h"

#include "extensionsystem/pluginmanager.h"

#include <QDebug>

using namespace ExtensionSystem;

bool CameraPlugin::initialize(QString *errorString)
{
    m_service = new camera_plugin::CameraServiceImpl(this);
    PluginManager::addObject(m_service);
    qDebug() << "[CameraPlugin] ICameraService registered (captures init deferred)";
    Q_UNUSED(errorString);
    return true;
}

bool CameraPlugin::delayedInitialize()
{
    if (!m_service) {
        return false;
    }

    QString loadError;
    if (!m_service->refreshCaptures(&loadError)) {
        qWarning() << "[CameraPlugin]" << loadError;
        return false;
    }

    qDebug() << "[CameraPlugin] CaptureManager ready, captures:" << m_service->captureNames();
    return false;
}

IPlugin::ShutdownFlag CameraPlugin::aboutToShutdown()
{
    if (m_service) {
        PluginManager::removeObject(m_service);
        m_service->shutdown();
        m_service->deleteLater();
        m_service = nullptr;
    }
    return IPlugin::SynchronousShutdown;
}
