/**
 * @file    DbManager.cpp
 * @brief   DbManager 单例实现
 */
#include "DbManager.h"

#include <QxOrm.h>
#include <QxDao/QxSession.h>
#include <QxDao/QxSqlDatabase.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

namespace orm {

// ─────────────────────────────────────────────────────────────────────────────
// Private Implementation (Pimpl)
// ─────────────────────────────────────────────────────────────────────────────

struct DbManager::Impl
{
    DbConfig config;
    bool     connected  = false;
    QString  lastError;
    QMutex   mutex;
};

// ─────────────────────────────────────────────────────────────────────────────
// 单例
// ─────────────────────────────────────────────────────────────────────────────

DbManager &DbManager::instance()
{
    static DbManager inst;
    return inst;
}

DbManager::DbManager()
    : d(std::make_unique<Impl>())
{
}

DbManager::~DbManager()
{
    closeAll();
}

// ─────────────────────────────────────────────────────────────────────────────
// 初始化
// ─────────────────────────────────────────────────────────────────────────────

bool DbManager::init(const DbConfig &config)
{
    QMutexLocker locker(&d->mutex);
    d->config    = config;
    d->connected = false;
    d->lastError.clear();

    auto *qxDb = qx::QxSqlDatabase::getSingleton();

    // 设置驱动
    qxDb->setDriverName(config.resolvedDriverName());

    // 通用连接参数
    qxDb->setDatabaseName(config.database);

    if (config.driver != DbDriver::SQLite) {
        qxDb->setHostName(config.host);
        qxDb->setPort(config.port);
        qxDb->setUserName(config.username);
        qxDb->setPassword(config.password);
    }

    if (!config.connectOptions.isEmpty())
        qxDb->setConnectOptions(config.connectOptions);

    // 调试追踪
    qxDb->setTraceSqlQuery(config.traceSql);
    qxDb->setTraceSqlBoundValuesOnError(config.traceSqlError);

    // 试连接验证
    QSqlError err;
    QSqlDatabase db = qx::QxSqlDatabase::getDatabase(err);
    if (err.isValid()) {
        d->lastError = err.text();
        qWarning() << "[DbManager] init failed:" << d->lastError
                   << " config:" << config.toString();
        return false;
    }

    d->connected = true;
    qDebug() << "[DbManager] init OK:" << config.toString();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态查询
// ─────────────────────────────────────────────────────────────────────────────

bool DbManager::isConnected() const
{
    QMutexLocker locker(&d->mutex);
    return d->connected;
}

QString DbManager::lastError() const
{
    QMutexLocker locker(&d->mutex);
    return d->lastError;
}

DbConfig DbManager::config() const
{
    QMutexLocker locker(&d->mutex);
    return d->config;
}

QSqlDatabase DbManager::database()
{
    return qx::QxSqlDatabase::getDatabase();
}

void DbManager::closeAll()
{
    qx::QxSqlDatabase::closeAllDatabases();
    QMutexLocker locker(&d->mutex);
    d->connected = false;
    qDebug() << "[DbManager] all connections closed.";
}

// ─────────────────────────────────────────────────────────────────────────────
// 事务
// ─────────────────────────────────────────────────────────────────────────────

bool DbManager::transaction(const std::function<bool(qx::QxSession &)> &fn)
{
    qx::QxSession session;

    bool ok = fn(session); // NOLINT

    if (!ok || !session.isValid()) {
        session.rollback();
        if (!session.isValid()) {
            QMutexLocker locker(&d->mutex);
            d->lastError = session.lastError().text();
            qWarning() << "[DbManager] transaction rollback:"
                       << d->lastError;
        } else {
            qWarning() << "[DbManager] transaction rolled back by caller.";
        }
        return false;
    }

    session.commit();
    return true;
}

} // namespace orm
