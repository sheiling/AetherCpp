#pragma once

#include "pluginspec.h"
#include "iplugin.h"

#include <QJsonObject>
#include <QObject>
#include <QPluginLoader>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QStringList>
#include <QVector>
#include <QXmlStreamReader>

namespace ExtensionSystem {

class IPlugin;
class PluginManager;

namespace Internal {

class PluginSpecPrivate : public QObject
{
    Q_OBJECT

public:
    PluginSpecPrivate(PluginSpec *spec);

    bool read(const QString &fileName);
    bool provides(const QString &pluginName, const QString &version) const;
    bool resolveDependencies(const QList<PluginSpec *> &specs);
    bool loadLibrary();
    bool initializePlugin();
    bool initializeExtensions();
    bool delayedInitialize();
    IPlugin::ShutdownFlag stop();
    void kill();

    QPluginLoader loader;

    QString name;
    QString version;
    bool required = false;
    QString description;
    QString category;
    QVector<PluginDependency> dependencies;
    QJsonObject metaData;
    QString location;
    QString filePath;

    QHash<PluginDependency, PluginSpec *> dependencySpecs;
    IPlugin *plugin = nullptr;

    PluginSpec::State state = PluginSpec::Invalid;
    bool hasError = false;
    QString errorString;

    static bool isValidVersion(const QString &version);
    static int versionCompare(const QString &version1, const QString &version2);


    bool readMetaData(const QJsonObject &pluginMetaData);

private:
    PluginSpec *q;

    bool reportError(const QString &err);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRegularExpression versionRegExp();
#else
    static const QRegExp &versionRegExp();
#endif
};

} // namespace Internal
} // namespace ExtensionSystem
