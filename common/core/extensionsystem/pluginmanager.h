#pragma once

#include "extensionsystem_global.h"

#include <QObject>
#include <QStringList>
#include <QReadLocker>
#include <QReadWriteLock>

QT_BEGIN_NAMESPACE
class QTextStream;
class QSettings;
QT_END_NAMESPACE

namespace ExtensionSystem {
class IPlugin;
class PluginSpec;

namespace Internal { class PluginManagerPrivate; }

class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager *instance();

    PluginManager();
    ~PluginManager() override;

    // ------------------------------------------------
    // 对象池操作
    // ------------------------------------------------
    static void addObject(QObject *obj);
    static void removeObject(QObject *obj);
    static QList<QObject *> allObjects();
    static QReadWriteLock *listLock();

    /// 按类型从对象池中获取第一个匹配对象（QObject 或其子类）
    template <typename T> static T *getObject()
    {
        QReadLocker lock(listLock());
        const QList<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = qobject_cast<T *>(obj))
                return result;
        }
        return nullptr;
    }

    /// 按纯虚接口从对象池中获取第一个匹配对象（非 QObject，不使用 Q_DECLARE_INTERFACE）
    template <typename T> static T *getInterface()
    {
        QReadLocker lock(listLock());
        const QList<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = dynamic_cast<T *>(obj))
                return result;
        }
        return nullptr;
    }

    template <typename T, typename Predicate>
    static T *getObject(Predicate predicate)
    {
        QReadLocker lock(listLock());
        const QList<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = qobject_cast<T *>(obj))
                if (predicate(result))
                    return result;
        }
        return nullptr;
    }

    /// 按类型从对象池中获取所有匹配对象
    template <typename T> static QList<T *> getObjects()
    {
        QReadLocker lock(listLock());
        QList<T *> results;
        const QList<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = qobject_cast<T *>(obj))
                results.append(result);
        }
        return results;
    }

    static QObject *getObjectByName(const QString &name);

    // ------------------------------------------------
    // 插件操作
    // ------------------------------------------------
    static QList<PluginSpec *> loadQueue();
    static void loadPlugins();
    static QStringList pluginPaths();
    static void setPluginPaths(const QStringList &paths);
    static QString pluginIID();
    static void setPluginIID(const QString &iid);
    static const QList<PluginSpec *> plugins();
    static QHash<QString, QList<PluginSpec *>> pluginCollections();
    static bool hasError();
    static bool isInitializationDone();

    void shutdown();

signals:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);
    void pluginsChanged();
    void initializationDone();

    friend class Internal::PluginManagerPrivate;
};

} // namespace ExtensionSystem
