#pragma once

#include "pluginspec.h"

#include <QSet>
#include <QStringList>
#include <QObject>
#include <QScopedPointer>
#include <QReadWriteLock>

QT_BEGIN_NAMESPACE
class QTime;
class QTimer;
class QEventLoop;
QT_END_NAMESPACE

namespace ExtensionSystem {

class PluginManager;

namespace Internal {

class PluginSpecPrivate;

class PluginManagerPrivate : public QObject
{
    Q_OBJECT
public:
    PluginManagerPrivate(PluginManager *pluginManager);
    ~PluginManagerPrivate() override;

    // Object pool operations
    void addObject(QObject *obj);
    void removeObject(QObject *obj);

    // Plugin operations
    void loadPlugins();
    void shutdown();
    void setPluginPaths(const QStringList &paths);
    QList<PluginSpec *> loadQueue();
    void loadPlugin(PluginSpec *spec, PluginSpec::State destState);
    void resolveDependencies();

    QHash<QString, QList<PluginSpec *>> pluginCategories;
    QList<PluginSpec *> pluginSpecs;
    QStringList pluginPaths;
    QString pluginIID;
    QList<QObject *> allObjects; // ### make this a QList<QPointer<QObject> > > ?
    // delayed initialization
    QTimer *delayedInitializeTimer = nullptr;
    QList<PluginSpec *> delayedInitializeQueue;
    // ansynchronous shutdown
    QList<PluginSpec *> asynchronousPlugins; // plugins that have requested async shutdown
    QEventLoop *shutdownEventLoop = nullptr; // used for async shutdown

    PluginSpec *pluginByName(const QString &name) const;

    mutable QReadWriteLock m_lock;

    bool m_isInitializationDone = false;

private:
    PluginManager *q;

    void nextDelayedInitialize();
    void asyncShutdownFinished();

    void readPluginPaths();
    bool loadQueue(PluginSpec *spec,
            QList<PluginSpec *> &queue,
            QList<PluginSpec *> &circularityCheckQueue);
    void stopAll();
    void deleteAll();
};

} // namespace Internal
} // namespace ExtensionSystem
