#pragma once

#include "extensionsystem_global.h"

#include <QString>
#include <QHash>
#include <QVector>
#include <QJsonObject>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringList>
#else
QT_BEGIN_NAMESPACE
class QStringList;
class QRegExp;
QT_END_NAMESPACE
#endif

namespace ExtensionSystem {

namespace Internal {

class PluginSpecPrivate;
class PluginManagerPrivate;

} // Internal

class IPlugin;

struct EXTENSIONSYSTEM_EXPORT PluginDependency
{
    enum Type {
        Required,
        Optional,
        Test
    };

    PluginDependency() : type(Required) {}

    QString name;
    QString version;
    Type type;
    bool operator==(const PluginDependency &other) const;
    QString toString() const;
};

EXTENSIONSYSTEM_EXPORT uint qHash(const ExtensionSystem::PluginDependency &value);

class EXTENSIONSYSTEM_EXPORT PluginSpec
{
public:
    enum State { Invalid, Read, Resolved, Loaded, Initialized, Running, Stopped, Deleted};

    ~PluginSpec();

    // 元数据 (Read 状态后有效)
    QString name() const;
    QString version() const;
    QString description() const;
    QString category() const;
    bool isRequired() const;
    QVector<PluginDependency> dependencies() const;
    QJsonObject metaData() const;

    // 文件信息 (Read 状态后有效)
    QString location() const;
    QString filePath() const;

    bool provides(const QString &pluginName, const QString &version) const;

    // 依赖关系 (Resolved 状态后有效)
    QHash<PluginDependency, PluginSpec *> dependencySpecs() const;
    bool requiresAny(const QSet<PluginSpec *> &plugins) const;

    // 插件实例 (Loaded 状态后有效)
    IPlugin *plugin() const;

    // 状态
    State state() const;
    bool hasError() const;
    QString errorString() const;

private:
    PluginSpec();

    Internal::PluginSpecPrivate *d;
    friend class Internal::PluginManagerPrivate;
    friend class Internal::PluginSpecPrivate;
};

} // namespace ExtensionSystem
