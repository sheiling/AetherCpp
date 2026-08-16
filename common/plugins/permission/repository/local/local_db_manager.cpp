/**
 * @file    local_db_manager.cpp
 * @brief   DbManager 单例实现（直接 QSqlDatabase，无 QxOrm 依赖）
 */
#include "local_db_manager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

namespace orm {

// ─────────────────────────────────────────────────────────────────────────────
// 连接名称常量
// ─────────────────────────────────────────────────────────────────────────────
static const char *kConnectionName = "permission_connection";

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

    QSqlDatabase db = QSqlDatabase::addDatabase(
        config.resolvedDriverName(),
        QString::fromLatin1(kConnectionName));

    db.setDatabaseName(config.database);

    if (config.driver != DbDriver::SQLite) {
        db.setHostName(config.host);
        db.setPort(config.port);
        db.setUserName(config.username);
        db.setPassword(config.password);
    }

    if (!config.connectOptions.isEmpty())
        db.setConnectOptions(config.connectOptions);

    // 试连接验证
    if (!db.open()) {
        d->lastError = db.lastError().text();
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
    return QSqlDatabase::database(QString::fromLatin1(kConnectionName));
}

void DbManager::closeAll()
{
    {
        QSqlDatabase db = QSqlDatabase::database(
            QString::fromLatin1(kConnectionName));
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(QString::fromLatin1(kConnectionName));

    QMutexLocker locker(&d->mutex);
    d->connected = false;
    qDebug() << "[DbManager] all connections closed.";
}

} // namespace orm
