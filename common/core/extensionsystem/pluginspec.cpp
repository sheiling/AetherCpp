#include "pluginspec.h"

#include "pluginspec_p.h"
#include "iplugin.h"
#include "iplugin_p.h"
#include "pluginmanager.h"
#include "messagecenter/message_center.h"

#include "utils/algorithm.h"
#include "utils/stringutils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPluginLoader>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#include <exception>


using namespace ExtensionSystem;
using namespace ExtensionSystem::Internal;

/*!
    \internal
*/
uint ExtensionSystem::qHash(const PluginDependency &value)
{
    return qHash(value.name);
}

/*!
    \internal
*/
bool PluginDependency::operator==(const PluginDependency &other) const
{
    return name == other.name && version == other.version && type == other.type;
}

static QString typeString(PluginDependency::Type type)
{
    switch (type) {
    case PluginDependency::Optional:
        return QString(", optional");
    case PluginDependency::Test:
        return QString(", test");
    case PluginDependency::Required:
    default:
        return QString();
    }
}

QString PluginDependency::toString() const
{
    return name + " (" + version + typeString(type) + ")";
}

/*!
    \internal
*/
PluginSpec::PluginSpec()
    : d(new PluginSpecPrivate(this))
{
}

/*!
    \internal
*/
PluginSpec::~PluginSpec()
{
    delete d;
    d = nullptr;
}

/*!
    The plugin name. This is valid after the PluginSpec::Read state is reached.
*/
QString PluginSpec::name() const
{
    return d->name;
}

/*!
    The plugin version. This is valid after the PluginSpec::Read state is reached.
*/
QString PluginSpec::version() const
{
    return d->version;
}

/*!
    The plugin description. This is valid after the PluginSpec::Read state is reached.
*/
QString PluginSpec::description() const
{
    return d->description;
}

/*!
    The category that the plugin belongs to. Categories are groups of plugins which allow for keeping them together in the UI.
    Returns an empty string if the plugin does not belong to a category.
*/
QString PluginSpec::category() const
{
    return d->category;
}

bool PluginSpec::isRequired() const
{
    return d->required;
}

/*!
    The plugin dependencies. This is valid after the PluginSpec::Read state is reached.
*/
QVector<PluginDependency> PluginSpec::dependencies() const
{
    return d->dependencies;
}

QJsonObject PluginSpec::metaData() const
{
    return d->metaData;
}

/*!
    The absolute path to the directory containing the plugin XML description file
    this PluginSpec corresponds to.
*/
QString PluginSpec::location() const
{
    return d->location;
}

/*!
    The absolute path to the plugin XML description file (including the file name)
    this PluginSpec corresponds to.
*/
QString PluginSpec::filePath() const
{
    return d->filePath;
}

/*!
    The state in which the plugin currently is.
    See the description of the PluginSpec::State enum for details.
*/
PluginSpec::State PluginSpec::state() const
{
    return d->state;
}

/*!
    Returns whether an error occurred while reading/starting the plugin.
*/
bool PluginSpec::hasError() const
{
    return d->hasError;
}

/*!
    Detailed, possibly multi-line, error description in case of an error.
*/
QString PluginSpec::errorString() const
{
    return d->errorString;
}

/*!
    Returns whether this plugin can be used to fill in a dependency of the given
    \a pluginName and \a version.

        \sa PluginSpec::dependencies()
*/
bool PluginSpec::provides(const QString &pluginName, const QString &version) const
{
    return d->provides(pluginName, version);
}

/*!
    The corresponding IPlugin instance, if the plugin library has already been successfully loaded,
    i.e. the PluginSpec::Loaded state is reached.
*/
IPlugin *PluginSpec::plugin() const
{
    return d->plugin;
}

/*!
    Returns the list of dependencies, already resolved to existing plugin specs.
    Valid if PluginSpec::Resolved state is reached.

    \sa PluginSpec::dependencies()
*/
QHash<PluginDependency, PluginSpec *> PluginSpec::dependencySpecs() const
{
    return d->dependencySpecs;
}

bool PluginSpec::requiresAny(const QSet<PluginSpec *> &plugins) const
{
    return Utils::anyOf(d->dependencySpecs.keys(), [this, &plugins](const PluginDependency &dep) {
        return dep.type == PluginDependency::Required
               && plugins.contains(d->dependencySpecs.value(dep));
    });
}

//==========PluginSpecPrivate==================

namespace {
    const char PLUGIN_METADATA[] = "MetaData";
    const char PLUGIN_NAME[] = "Name";
    const char PLUGIN_VERSION[] = "Version";
    const char PLUGIN_REQUIRED[] = "Required";
    const char DESCRIPTION[] = "Description";
    const char CATEGORY[] = "Category";
    const char DEPENDENCIES[] = "Dependencies";
    const char DEPENDENCY_NAME[] = "Name";
    const char DEPENDENCY_VERSION[] = "Version";
    const char DEPENDENCY_TYPE[] = "Type";
    const char DEPENDENCY_TYPE_SOFT[] = "optional";
    const char DEPENDENCY_TYPE_HARD[] = "required";
    const char DEPENDENCY_TYPE_TEST[] = "test";
}
/*!
    \internal
*/
PluginSpecPrivate::PluginSpecPrivate(PluginSpec *spec)
    : q(spec)
{
}

/*!
    \internal
    Returns false if the file does not represent a Qt Creator plugin.
*/
bool PluginSpecPrivate::read(const QString &fileName)
{
    qDebug() << "Reading meta data of" << fileName;
    name
        = version
        = description
        = category
        = location
        = QString();
    state = PluginSpec::Invalid;
    hasError = false;
    errorString.clear();
    dependencies.clear();
    metaData = QJsonObject();
    QFileInfo fileInfo(fileName);
    location = fileInfo.absolutePath();
    filePath = fileInfo.absoluteFilePath();
    loader.setFileName(filePath);
    if (loader.fileName().isEmpty()) {
        qDebug() << "Cannot open file";
        return false;
    }

    if (!readMetaData(loader.metaData()))
        return false;

    state = PluginSpec::Read;
    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::reportError(const QString &err)
{
    errorString = err;
    hasError = true;
    return true;
}

static inline QString msgValueMissing(const char *key)
{
    return QCoreApplication::translate("PluginSpec", "\"%1\" is missing").arg(QLatin1String(key));
}

static inline QString msgValueIsNotAString(const char *key)
{
    return QCoreApplication::translate("PluginSpec", "Value for key \"%1\" is not a string")
            .arg(QLatin1String(key));
}

static inline QString msgValueIsNotABool(const char *key)
{
    return QCoreApplication::translate("PluginSpec", "Value for key \"%1\" is not a bool")
            .arg(QLatin1String(key));
}

static inline QString msgValueIsNotAObjectArray(const char *key)
{
    return QCoreApplication::translate("PluginSpec", "Value for key \"%1\" is not an array of objects")
            .arg(QLatin1String(key));
}

static inline QString msgValueIsNotAMultilineString(const char *key)
{
    return QCoreApplication::translate("PluginSpec", "Value for key \"%1\" is not a string and not an array of strings")
            .arg(QLatin1String(key));
}

static inline QString msgInvalidFormat(const char *key, const QString &content)
{
    return QCoreApplication::translate("PluginSpec", "Value \"%2\" for key \"%1\" has invalid format")
            .arg(QLatin1String(key), content);
}

bool PluginSpecPrivate::readMetaData(const QJsonObject &pluginMetaData)
{
    QJsonValue value;

    // Qt5 QPluginLoader::metaData() returns {"MetaData": {...}, "IID": "xxx"}
    // Qt6 QPluginLoader::metaData() returns {"MetaData": {...}}  (no top-level "IID")
    // In both cases the IID is also enforced at compile-time by Q_PLUGIN_METADATA(IID ...).
    value = pluginMetaData.value(QLatin1String("IID"));
    if (value.isString()) {
        if (value.toString() != PluginManager::pluginIID()) {
            return false;
        }
    }
    // Qt6 omits top-level "IID" — rely on compile-time Q_PLUGIN_METADATA validation

    value = pluginMetaData.value(QLatin1String(PLUGIN_METADATA));
    if (!value.isObject()) {
        // Qt6 may embed the JSON directly without wrapping in "MetaData"
        if (pluginMetaData.contains(QLatin1String(PLUGIN_NAME))) {
            metaData = pluginMetaData;
        } else {
            return reportError(tr("Plugin meta data not found"));
        }
    } else {
        metaData = value.toObject();
    }

    value = metaData.value(QLatin1String(PLUGIN_NAME));
    if (value.isUndefined())
        return reportError(msgValueMissing(PLUGIN_NAME));
    if (!value.isString())
        return reportError(msgValueIsNotAString(PLUGIN_NAME));
    name = value.toString();

    value = metaData.value(QLatin1String(PLUGIN_VERSION));
    if (value.isUndefined())
        return reportError(msgValueMissing(PLUGIN_VERSION));
    if (!value.isString())
        return reportError(msgValueIsNotAString(PLUGIN_VERSION));
    version = value.toString();
    if (!isValidVersion(version))
        return reportError(msgInvalidFormat(PLUGIN_VERSION, version));

    value = metaData.value(QLatin1String(PLUGIN_REQUIRED));
    if (!value.isUndefined() && !value.isBool())
        return reportError(msgValueIsNotABool(PLUGIN_REQUIRED));
    required = value.toBool(false);


    value = metaData.value(QLatin1String(DESCRIPTION));
    if (!value.isUndefined() && !Utils::readMultiLineString(value, &description))
        return reportError(msgValueIsNotAString(DESCRIPTION));


    value = metaData.value(QLatin1String(CATEGORY));
    if (!value.isUndefined() && !value.isString())
        return reportError(msgValueIsNotAString(CATEGORY));
    category = value.toString();

    value = metaData.value(QLatin1String(DEPENDENCIES));
    if (!value.isUndefined() && !value.isArray())
        return reportError(msgValueIsNotAObjectArray(DEPENDENCIES));
    if (!value.isUndefined()) {
        QJsonArray array = value.toArray();
        foreach (const QJsonValue &v, array) {
            if (!v.isObject())
                return reportError(msgValueIsNotAObjectArray(DEPENDENCIES));
            QJsonObject dependencyObject = v.toObject();
            PluginDependency dep;
            value = dependencyObject.value(QLatin1String(DEPENDENCY_NAME));
            if (value.isUndefined())
                return reportError(tr("Dependency: %1").arg(msgValueMissing(DEPENDENCY_NAME)));
            if (!value.isString())
                return reportError(tr("Dependency: %1").arg(msgValueIsNotAString(DEPENDENCY_NAME)));
            dep.name = value.toString();
            value = dependencyObject.value(QLatin1String(DEPENDENCY_VERSION));
            if (!value.isUndefined() && !value.isString())
                return reportError(tr("Dependency: %1").arg(msgValueIsNotAString(DEPENDENCY_VERSION)));
            dep.version = value.toString();
            if (!isValidVersion(dep.version))
                return reportError(tr("Dependency: %1").arg(msgInvalidFormat(DEPENDENCY_VERSION,
                                                                             dep.version)));
            dep.type = PluginDependency::Required;
            value = dependencyObject.value(QLatin1String(DEPENDENCY_TYPE));
            if (!value.isUndefined() && !value.isString())
                return reportError(tr("Dependency: %1").arg(msgValueIsNotAString(DEPENDENCY_TYPE)));
            if (!value.isUndefined()) {
                const QString typeValue = value.toString();
                if (typeValue.toLower() == QLatin1String(DEPENDENCY_TYPE_HARD)) {
                    dep.type = PluginDependency::Required;
                } else if (typeValue.toLower() == QLatin1String(DEPENDENCY_TYPE_SOFT)) {
                    dep.type = PluginDependency::Optional;
                } else if (typeValue.toLower() == QLatin1String(DEPENDENCY_TYPE_TEST)) {
                    dep.type = PluginDependency::Test;
                } else {
                    return reportError(tr("Dependency: \"%1\" must be \"%2\" or \"%3\" (is \"%4\").")
                                       .arg(QLatin1String(DEPENDENCY_TYPE),
                                            QLatin1String(DEPENDENCY_TYPE_HARD),
                                            QLatin1String(DEPENDENCY_TYPE_SOFT),
                                            typeValue));
                }
            }
            dependencies.append(dep);
        }
    }

    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::provides(const QString &pluginName, const QString &pluginVersion) const
{
    if (QString::compare(pluginName, name, Qt::CaseInsensitive) != 0)
        return false;
    // return (versionCompare(version, pluginVersion) >= 0) && (versionCompare(compatVersion, pluginVersion) <= 0);
    return true;
}

/*!
    \internal
*/
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QRegularExpression PluginSpecPrivate::versionRegExp()
{
    static const QRegularExpression reg(QLatin1String("([0-9]+)(?:[.]([0-9]+))?(?:[.]([0-9]+))?(?:_([0-9]+))?"));
    return reg;
}
#else
const QRegExp &PluginSpecPrivate::versionRegExp()
{
    static const QRegExp reg(QLatin1String("([0-9]+)(?:[.]([0-9]+))?(?:[.]([0-9]+))?(?:_([0-9]+))?"));
    return reg;
}
#endif
/*!
    \internal
*/
bool PluginSpecPrivate::isValidVersion(const QString &version)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return versionRegExp().match(version).hasMatch();
#else
    return versionRegExp().exactMatch(version);
#endif
}

/*!
    \internal
*/
int PluginSpecPrivate::versionCompare(const QString &version1, const QString &version2)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression reg1 = versionRegExp();
    QRegularExpression reg2 = versionRegExp();
    QRegularExpressionMatch match1 = reg1.match(version1);
    QRegularExpressionMatch match2 = reg2.match(version2);
    if (!match1.hasMatch())
        return 0;
    if (!match2.hasMatch())
        return 0;
    int number1;
    int number2;
    for (int i = 0; i < 4; ++i) {
        number1 = match1.captured(i+1).toInt();
        number2 = match2.captured(i+1).toInt();
        if (number1 < number2)
            return -1;
        if (number1 > number2)
            return 1;
    }
    return 0;
#else
    QRegExp reg1 = versionRegExp();
    QRegExp reg2 = versionRegExp();
    if (!reg1.exactMatch(version1))
        return 0;
    if (!reg2.exactMatch(version2))
        return 0;
    int number1;
    int number2;
    for (int i = 0; i < 4; ++i) {
        number1 = reg1.cap(i+1).toInt();
        number2 = reg2.cap(i+1).toInt();
        if (number1 < number2)
            return -1;
        if (number1 > number2)
            return 1;
    }
    return 0;
#endif
}

/*!
    \internal
*/
bool PluginSpecPrivate::resolveDependencies(const QList<PluginSpec *> &specs)
{
    if (hasError)
        return false;
    if (state == PluginSpec::Resolved)
        state = PluginSpec::Read; // Go back, so we just re-resolve the dependencies.
    if (state != PluginSpec::Read) {
        errorString = QCoreApplication::translate("PluginSpec", "Resolving dependencies failed because state != Read");
        hasError = true;
        return false;
    }
    QHash<PluginDependency, PluginSpec *> resolvedDependencies;
    foreach (const PluginDependency &dependency, dependencies) {
        PluginSpec * const found = Utils::findOrDefault(specs, [&dependency](PluginSpec *spec) {
            return spec->provides(dependency.name, dependency.version);
        });
        if (!found) {
            if (dependency.type == PluginDependency::Required) {
                hasError = true;
                if (!errorString.isEmpty())
                    errorString.append(QLatin1Char('\n'));
                errorString.append(QCoreApplication::translate("PluginSpec", "Could not resolve dependency '%1(%2)'")
                    .arg(dependency.name).arg(dependency.version));
            }
            continue;
        }
        resolvedDependencies.insert(dependency, found);
    }
    if (hasError)
        return false;

    dependencySpecs = resolvedDependencies;

    state = PluginSpec::Resolved;

    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::loadLibrary()
{
    if (hasError)
        return false;
    if (state != PluginSpec::Resolved) {
        if (state == PluginSpec::Loaded)
            return true;
        errorString = QCoreApplication::translate("PluginSpec", "Loading the library failed because state != Resolved");
        hasError = true;
        return false;
    }

    MessageCenter::Center::Notify(EVENT_SPLASH_MESSAGE,
                                  QStringLiteral("%1加载中……").arg(q->name()));

    try {
        if (!loader.load()) {
            hasError    = true;
            errorString = QDir::toNativeSeparators(filePath)
                          + QString::fromLatin1(": ") + loader.errorString();
            return false;
        }
    } catch (const std::exception &ex) {
        hasError    = true;
        errorString = QDir::toNativeSeparators(filePath) + QString::fromLatin1(": ")
                      + QString::fromUtf8(ex.what());
        return false;
    } catch (...) {
        hasError    = true;
        errorString = QDir::toNativeSeparators(filePath)
                      + QString::fromLatin1(": unknown exception during DLL load");
        return false;
    }
    auto *pluginObject = qobject_cast<IPlugin*>(loader.instance());
    if (!pluginObject) {
        hasError = true;
        errorString = QCoreApplication::translate("PluginSpec", "Plugin is not valid (does not derive from IPlugin)");
        loader.unload();
        return false;
    }
    state = PluginSpec::Loaded;
    plugin = pluginObject;
    plugin->d->pluginSpec = q;
    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::initializePlugin()
{
    if (hasError)
        return false;
    if (state != PluginSpec::Loaded) {
        if (state == PluginSpec::Initialized)
            return true;
        errorString = QCoreApplication::translate("PluginSpec", "Initializing the plugin failed because state != Loaded");
        hasError = true;
        return false;
    }
    if (!plugin) {
        errorString = QCoreApplication::translate("PluginSpec", "Internal error: have no plugin instance to initialize");
        hasError = true;
        return false;
    }
    QString err;
    try {
        if (!plugin->initialize(&err)) {
            errorString = QCoreApplication::translate("PluginSpec", "Plugin initialization failed: %1").arg(err);
            hasError    = true;
            return false;
        }
    } catch (const std::exception &ex) {
        errorString = QString::fromUtf8(ex.what());
        hasError    = true;
        return false;
    } catch (...) {
        errorString = QCoreApplication::translate("PluginSpec", "Plugin initialization failed: unknown exception");
        hasError    = true;
        return false;
    }
    state = PluginSpec::Initialized;
    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::initializeExtensions()
{
    if (hasError)
        return false;
    if (state != PluginSpec::Initialized) {
        if (state == PluginSpec::Running)
            return true;
        errorString = QCoreApplication::translate("PluginSpec", "Cannot perform extensionsInitialized because state != Initialized");
        hasError = true;
        return false;
    }
    if (!plugin) {
        errorString = QCoreApplication::translate("PluginSpec", "Internal error: have no plugin instance to perform extensionsInitialized");
        hasError = true;
        return false;
    }
    try {
        plugin->extensionsInitialized();
    } catch (const std::exception &ex) {
        errorString = QString::fromUtf8(ex.what());
        hasError    = true;
        return false;
    } catch (...) {
        errorString = QCoreApplication::translate("PluginSpec",
                                                  "extensionsInitialized failed: unknown exception");
        hasError    = true;
        return false;
    }
    state = PluginSpec::Running;
    return true;
}

/*!
    \internal
*/
bool PluginSpecPrivate::delayedInitialize()
{
    if (hasError)
        return false;
    if (state != PluginSpec::Running)
        return false;
    if (!plugin) {
        errorString = QCoreApplication::translate("PluginSpec", "Internal error: have no plugin instance to perform delayedInitialize");
        hasError = true;
        return false;
    }
    return plugin->delayedInitialize();
}

/*!
    \internal
*/
IPlugin::ShutdownFlag PluginSpecPrivate::stop()
{
    if (!plugin)
        return IPlugin::SynchronousShutdown;
    state = PluginSpec::Stopped;
    return plugin->aboutToShutdown();
}

/*!
    \internal
*/
void PluginSpecPrivate::kill()
{
    if (!plugin)
        return;
    delete plugin;
    plugin = nullptr;
    state = PluginSpec::Deleted;
}
