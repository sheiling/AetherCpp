#include "graphicsviewplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "ui/graphicsviewimpl.h"

#include <QDebug>

using namespace ExtensionSystem;

bool GraphicsViewPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    m_impl = new GraphicsViewImpl();
    PluginManager::addObject(m_impl);
    qDebug() << "[GraphicsViewPlugin] GraphicsViewImpl registered to object pool.";
    return true;
}

IPlugin::ShutdownFlag GraphicsViewPlugin::aboutToShutdown()
{
    if (m_impl) {
        PluginManager::removeObject(m_impl);
        delete m_impl;
        m_impl = nullptr;
    }
    return SynchronousShutdown;
}
