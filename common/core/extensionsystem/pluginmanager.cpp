#include "pluginmanager.h"
#include "pluginmanager_p.h"
#include "pluginspec.h"
#include "pluginspec_p.h"
#include "iplugin.h"

#include "utils/algorithm.h"
#include "utils/stringutils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QLibraryInfo>
#include <QMetaProperty>
#include <QTextStream>
#include <QTime>
#include <QWriteLocker>
#include <QDebug>
#include <QTimer>
#include <QSysInfo>

#include <functional>
#include <exception>

const int DELAYED_INITIALIZE_INTERVAL = 20; // ms

enum { debugLeaks = 0 };

namespace {

void pumpGuiEventsDuringPluginLoad()
{
    if (QCoreApplication *app = QCoreApplication::instance()) {
        try {
            app->processEvents(QEventLoop::AllEvents, 50);
        } catch (const std::exception &ex) {
            qWarning() << "[PluginManager] processEvents during plugin load:" << ex.what();
        } catch (...) {
            qWarning() << "[PluginManager] processEvents during plugin load: unknown exception";
        }
    }
}

} // namespace


namespace ExtensionSystem {

using namespace Internal;
using namespace Utils;

static Internal::PluginManagerPrivate *d = nullptr;
static PluginManager *m_instance = nullptr;

/*!
    Gets the unique plugin manager instance.
*/
PluginManager *PluginManager::instance()
{
    return m_instance;
}

/*!
    Creates a plugin manager. Should be done only once per application.
*/
PluginManager::PluginManager()
{
    m_instance = this;
    d = new PluginManagerPrivate(this);
}

/*!
    \internal
*/
PluginManager::~PluginManager()
{
    delete d;
    d = nullptr;
}

/*!
    Adds the object \a obj to the object pool, so it can be retrieved
    again from the pool by type.

    The plugin manager does not do any memory management - added objects
    must be removed from the pool and deleted manually by whoever is responsible for the object.

    Emits the objectAdded() signal.

    \sa PluginManager::removeObject()
    \sa PluginManager::getObject()
    \sa PluginManager::getObjectByName()
*/
void PluginManager::addObject(QObject *obj)
{
    d->addObject(obj);
}

/*!
    Emits aboutToRemoveObject() and removes the object \a obj from the object pool.
    \sa PluginManager::addObject()
*/
void PluginManager::removeObject(QObject *obj)
{
    d->removeObject(obj);
}

/*!
    Retrieves the list of all objects in the pool, unfiltered.

    Usually, clients do not need to call this function.

    \sa PluginManager::getObject()
*/
QList<QObject *> PluginManager::allObjects()
{
    return d->allObjects;
}

QReadWriteLock *PluginManager::listLock()
{
    return &d->m_lock;
}

/*!
    Tries to load all the plugins that were previously found when
    setting the plugin search paths. The plugin specs of the plugins
    can be used to retrieve error and state information about individual plugins.

    \sa setPluginPaths()
    \sa plugins()
*/
void PluginManager::loadPlugins()
{
    d->loadPlugins();
}

/*!
    Returns true if any plugin has errors even though it is enabled.
    Most useful to call after loadPlugins().
*/
bool PluginManager::hasError()
{
    return Utils::anyOf(plugins(), [](PluginSpec *spec) {
        // only show errors on startup if plugin is enabled.
        return spec->hasError();
    });
}

/*!
    Shuts down and deletes all plugins.
*/
void PluginManager::shutdown()
{
    d->shutdown();
}

/*!
    The list of paths were the plugin manager searches for plugins.

    \sa setPluginPaths()
*/
QStringList PluginManager::pluginPaths()
{
    return d->pluginPaths;
}

/*!
    Sets the plugin search paths, i.e. the file system paths where the plugin manager
    looks for plugin descriptions. All given \a paths and their sub directory trees
    are searched for plugin xml description files.

    \sa pluginPaths()
    \sa loadPlugins()
*/
void PluginManager::setPluginPaths(const QStringList &paths)
{
    d->setPluginPaths(paths);
}

/*!
    The IID that valid plugins must have.

    \sa setPluginIID()
*/
QString PluginManager::pluginIID()
{
    return d->pluginIID;
}

/*!
    Sets the IID that valid plugins must have. Only plugins with this IID are loaded, others are
    silently ignored.

    At the moment this must be called before setPluginPaths() is called.
    // ### let this + setPluginPaths read the plugin meta data lazyly whenever loadPlugins() or plugins() is called.
*/
void PluginManager::setPluginIID(const QString &iid)
{
    d->pluginIID = iid;
}

/*!
    List of all plugin specifications that have been found in the plugin search paths.
    This list is valid directly after the setPluginPaths() call.
    The plugin specifications contain the information from the plugins' xml description files
    and the current state of the plugins. If a plugin's library has been already successfully loaded,
    the plugin specification has a reference to the created plugin instance as well.

    \sa setPluginPaths()
*/
const QList<PluginSpec *> PluginManager::plugins()
{
    return d->pluginSpecs;
}

QHash<QString, QList<PluginSpec *> > PluginManager::pluginCollections()
{
    return d->pluginCategories;
}

/*!
    Returns a list of plugins in load order.
*/
QList<PluginSpec *> PluginManager::loadQueue()
{
    return d->loadQueue();
}

//============PluginManagerPrivate===========

/*!
    \internal
*/
void PluginManagerPrivate::nextDelayedInitialize()
{
    while (!delayedInitializeQueue.isEmpty()) {
        PluginSpec *spec = delayedInitializeQueue.takeFirst();
        // profilingReport(">delayedInitialize", spec);
        bool delay = spec->d->delayedInitialize();
        // profilingReport("<delayedInitialize", spec);
        if (delay)
            break; // do next delayedInitialize after a delay
    }
    if (delayedInitializeQueue.isEmpty()) {
        m_isInitializationDone = true;
        delete delayedInitializeTimer;
        delayedInitializeTimer = nullptr;
        // profilingSummary();
        emit q->initializationDone();
    } else {
        delayedInitializeTimer->start();
    }
}

/*!
    \internal
*/
PluginManagerPrivate::PluginManagerPrivate(PluginManager *pluginManager) :
    q(pluginManager)
{
}


/*!
    \internal
*/
PluginManagerPrivate::~PluginManagerPrivate()
{
    qDeleteAll(pluginSpecs);
}

/*!
    \internal
*/
void PluginManagerPrivate::stopAll()
{
    if (delayedInitializeTimer && delayedInitializeTimer->isActive()) {
        delayedInitializeTimer->stop();
        delete delayedInitializeTimer;
        delayedInitializeTimer = nullptr;
    }
    QList<PluginSpec *> queue = loadQueue();
    foreach (PluginSpec *spec, queue) {
        loadPlugin(spec, PluginSpec::Stopped);
    }
}

/*!
    \internal
*/
void PluginManagerPrivate::deleteAll()
{
    Utils::reverseForeach(loadQueue(), [this](PluginSpec *spec) {
        loadPlugin(spec, PluginSpec::Deleted);
    });
}

/*!
    \internal
*/
void PluginManagerPrivate::addObject(QObject *obj)
{
    {
        QWriteLocker lock(&m_lock);
        if (obj == nullptr) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add null object";
            return;
        }
        if (allObjects.contains(obj)) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add duplicate object";
            return;
        }

        allObjects.append(obj);
    }
    emit q->objectAdded(obj);
}

/*!
    \internal
*/
void PluginManagerPrivate::removeObject(QObject *obj)
{
    if (obj == nullptr) {
        qWarning() << "PluginManagerPrivate::removeObject(): trying to remove null object";
        return;
    }

    if (!allObjects.contains(obj)) {
        qWarning() << "PluginManagerPrivate::removeObject(): object not in list:"
            << obj << obj->objectName();
        return;
    }

    emit q->aboutToRemoveObject(obj);
    QWriteLocker lock(&m_lock);
    allObjects.removeAll(obj);
}

/*!
    \internal
*/
void PluginManagerPrivate::loadPlugins()
{
    QList<PluginSpec *> queue = loadQueue();
    // Utils::setMimeStartupPhase(MimeStartupPhase::PluginsLoading);
    foreach (PluginSpec *spec, queue) {
        loadPlugin(spec, PluginSpec::Loaded);
        pumpGuiEventsDuringPluginLoad();
    }
    // Utils::setMimeStartupPhase(MimeStartupPhase::PluginsInitializing);
    foreach (PluginSpec *spec, queue) {
        loadPlugin(spec, PluginSpec::Initialized);
        pumpGuiEventsDuringPluginLoad();
    }

    // Utils::setMimeStartupPhase(MimeStartupPhase::PluginsDelayedInitializing);
    Utils::reverseForeach(queue, [this](PluginSpec *spec) {
        loadPlugin(spec, PluginSpec::Running);
        if (spec->state() == PluginSpec::Running) {
            delayedInitializeQueue.append(spec);
        } else {
            // Plugin initialization failed, so cleanup after it
            spec->d->kill();
        }
        pumpGuiEventsDuringPluginLoad();
    });
    emit q->pluginsChanged();
    // Utils::setMimeStartupPhase(MimeStartupPhase::UpAndRunning);

    delayedInitializeTimer = new QTimer;
    delayedInitializeTimer->setInterval(DELAYED_INITIALIZE_INTERVAL);
    delayedInitializeTimer->setSingleShot(true);
    connect(delayedInitializeTimer, &QTimer::timeout,
            this, &PluginManagerPrivate::nextDelayedInitialize);
    delayedInitializeTimer->start();
}

/*!
    \internal
*/
void PluginManagerPrivate::shutdown()
{
    stopAll();
    if (!asynchronousPlugins.isEmpty()) {
        shutdownEventLoop = new QEventLoop;
        shutdownEventLoop->exec();
    }
    deleteAll();
    if (!allObjects.isEmpty()) {
        qDebug() << "There are" << allObjects.size() << "objects left in the plugin manager pool.";
        // Do not stream allObjects directly — qDebug() dereferences each QObject*,
        // which crashes if a plugin deleted the object without calling removeObject().
        for (QObject *obj : allObjects)
            qDebug() << "  pool entry:" << static_cast<void *>(obj);
    }
}

/*!
    \internal
*/
void PluginManagerPrivate::asyncShutdownFinished()
{
    auto *plugin = qobject_cast<IPlugin *>(sender());
    asynchronousPlugins.removeAll(plugin->pluginSpec());
    if (asynchronousPlugins.isEmpty())
        shutdownEventLoop->exit();
}

/*!
    \internal
*/
QList<PluginSpec *> PluginManagerPrivate::loadQueue()
{
    QList<PluginSpec *> queue;
    foreach (PluginSpec *spec, pluginSpecs) {
        QList<PluginSpec *> circularityCheckQueue;
        loadQueue(spec, queue, circularityCheckQueue);
    }
    return queue;
}

/*!
    \internal
*/
bool PluginManagerPrivate::loadQueue(PluginSpec *spec, QList<PluginSpec *> &queue,
        QList<PluginSpec *> &circularityCheckQueue)
{
    if (queue.contains(spec))
        return true;
    // check for circular dependencies
    if (circularityCheckQueue.contains(spec)) {
        spec->d->hasError = true;
        spec->d->errorString = PluginManager::tr("Circular dependency detected:");
        spec->d->errorString += QLatin1Char('\n');
        int index = circularityCheckQueue.indexOf(spec);
        for (int i = index; i < circularityCheckQueue.size(); ++i) {
            spec->d->errorString.append(PluginManager::tr("%1 (%2) depends on")
                .arg(circularityCheckQueue.at(i)->name()).arg(circularityCheckQueue.at(i)->version()));
            spec->d->errorString += QLatin1Char('\n');
        }
        spec->d->errorString.append(PluginManager::tr("%1 (%2)").arg(spec->name()).arg(spec->version()));
        return false;
    }
    circularityCheckQueue.append(spec);
    // check if we have the dependencies
    if (spec->state() == PluginSpec::Invalid || spec->state() == PluginSpec::Read) {
        queue.append(spec);
        return false;
    }

    // add dependencies
    QHashIterator<PluginDependency, PluginSpec *> it(spec->dependencySpecs());
    while (it.hasNext()) {
        it.next();
        // Skip test dependencies since they are not real dependencies but just force-loaded
        // plugins when running tests
        if (it.key().type == PluginDependency::Test)
            continue;
        PluginSpec *depSpec = it.value();
        if (!loadQueue(depSpec, queue, circularityCheckQueue)) {
            spec->d->hasError = true;
            spec->d->errorString =
                PluginManager::tr("Cannot load plugin because dependency failed to load: %1 (%2)\nReason: %3")
                    .arg(depSpec->name()).arg(depSpec->version()).arg(depSpec->errorString());
            return false;
        }
    }
    // add self
    queue.append(spec);
    return true;
}

/*!
    \internal
*/
void PluginManagerPrivate::loadPlugin(PluginSpec *spec, PluginSpec::State destState)
{
    if (spec->hasError() || spec->state() != destState - 1)
        return;

    switch (destState) {
    case PluginSpec::Running:
        // profilingReport(">initializeExtensions", spec);
        spec->d->initializeExtensions();
        // profilingReport("<initializeExtensions", spec);
        return;
    case PluginSpec::Deleted:
        // profilingReport(">delete", spec);
        spec->d->kill();
        // profilingReport("<delete", spec);
        return;
    default:
        break;
    }
    // check if dependencies have loaded without error
    QHashIterator<PluginDependency, PluginSpec *> it(spec->dependencySpecs());
    while (it.hasNext()) {
        it.next();
        if (it.key().type != PluginDependency::Required)
            continue;
        PluginSpec *depSpec = it.value();
        if (depSpec->state() != destState) {
            spec->d->hasError = true;
            spec->d->errorString =
                PluginManager::tr("Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depSpec->name()).arg(depSpec->version()).arg(depSpec->errorString());
            return;
        }
    }
    switch (destState) {
    case PluginSpec::Loaded:
        // profilingReport(">loadLibrary", spec);
        spec->d->loadLibrary();
        // profilingReport("<loadLibrary", spec);
        break;
    case PluginSpec::Initialized:
        // profilingReport(">initializePlugin", spec);
        spec->d->initializePlugin();
        // profilingReport("<initializePlugin", spec);
        break;
    case PluginSpec::Stopped:
        // profilingReport(">stop", spec);
        if (spec->d->stop() == IPlugin::AsynchronousShutdown) {
            asynchronousPlugins << spec;
            connect(spec->plugin(), &IPlugin::asynchronousShutdownFinished,
                    this, &PluginManagerPrivate::asyncShutdownFinished);
        }
        // profilingReport("<stop", spec);
        break;
    default:
        break;
    }
}

/*!
    \internal
*/
void PluginManagerPrivate::setPluginPaths(const QStringList &paths)
{
    qDebug() << "Plugin search paths:" << paths;
    pluginPaths = paths;
    readPluginPaths();
}

static QStringList pluginFiles(const QStringList &pluginPaths)
{
    QStringList pluginFiles;
    QStringList searchPaths = pluginPaths;
    while (!searchPaths.isEmpty()) {
        const QDir dir(searchPaths.takeFirst());
        const QFileInfoList entries = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : entries) {
            QString filePath = fileInfo.absoluteFilePath();
            // 只加载动态库文件
            if (!QLibrary::isLibrary(filePath)) {
                continue;
            }
                      
            pluginFiles << filePath;           
        }
    }
    return pluginFiles;
}

/*!
    \internal
*/
void PluginManagerPrivate::readPluginPaths()
{
    qDeleteAll(pluginSpecs);
    pluginSpecs.clear();
    pluginCategories.clear();

    // default
    pluginCategories.insert(QString(), QList<PluginSpec *>());

    foreach (const QString &pluginFile, pluginFiles(pluginPaths)) {
        auto *spec = new PluginSpec;
        if (!spec->d->read(pluginFile)) { // not a Qt Creator plugin
            delete spec;
            continue;
        }

        pluginCategories[spec->category()].append(spec);
        pluginSpecs.append(spec);
    }
    resolveDependencies();
    // ensure deterministic plugin load order by sorting
    Utils::sort(pluginSpecs, &PluginSpec::name);
    emit q->pluginsChanged();
}

void PluginManagerPrivate::resolveDependencies()
{
    foreach (PluginSpec *spec, pluginSpecs)
        spec->d->resolveDependencies(pluginSpecs);
}


PluginSpec *PluginManagerPrivate::pluginByName(const QString &name) const
{
    return Utils::findOrDefault(pluginSpecs, [name](PluginSpec *spec) { return spec->name() == name; });
}

bool PluginManager::isInitializationDone()
{
    return d->m_isInitializationDone;
}

/*!
    Retrieves one object with \a name from the object pool.
    \sa addObject()
*/

QObject *PluginManager::getObjectByName(const QString &name)
{
    QReadLocker lock(&d->m_lock);
    return Utils::findOrDefault(allObjects(), [&name](const QObject *obj) {
        return obj->objectName() == name;
    });
}

} // ExtensionSystem
