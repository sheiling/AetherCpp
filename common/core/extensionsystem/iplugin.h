#pragma once

#include "extensionsystem_global.h"

#include <QObject>
#include <QtPlugin>

namespace ExtensionSystem {

namespace Internal {
    class IPluginPrivate;
    class PluginSpecPrivate;
}

class PluginManager;
class PluginSpec;

class EXTENSIONSYSTEM_EXPORT IPlugin : public QObject
{
    Q_OBJECT

public:
    enum ShutdownFlag {
        SynchronousShutdown,
        AsynchronousShutdown
    };

    IPlugin();
    ~IPlugin() override;

    // 插件生命周期（必须实现）
    virtual bool initialize(QString *errorString) = 0;
    virtual void extensionsInitialized() = 0;

    // 插件生命周期（可选实现）
    virtual bool delayedInitialize() { return false; }
    virtual ShutdownFlag aboutToShutdown() { return SynchronousShutdown; }

    PluginSpec *pluginSpec() const;

signals:
    void asynchronousShutdownFinished();

private:
    Internal::IPluginPrivate *d;

    friend class Internal::PluginSpecPrivate;
};

} // namespace ExtensionSystem
