#include "param_change_db_store.h"

#include "data/orm/entities/RevisionHistory.h"
#include "repository/aoi_db_paths.h"

#include <Orm.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace aoi {
namespace {

QString defaultDbPath()
{
    return repository::defaultRecordDbPath();
}

QString nowIso() { return QDateTime::currentDateTime().toString(Qt::ISODateWithMs); }

bool ensureAuditTable(QSqlDatabase &db)
{
    // 幂等建表，允许首次运行自动初始化。
    QSqlQuery q(db);
    return q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS aoi_param_change_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "occurred_at TEXT NOT NULL,"
        "module_code TEXT NOT NULL,"
        "document_type TEXT NOT NULL,"
        "document_id TEXT NULL,"
        "source_page TEXT NULL,"
        "full_label TEXT NOT NULL,"
        "label_pattern TEXT NOT NULL,"
        "before_value TEXT NULL,"
        "after_value TEXT NULL,"
        "value_type TEXT NULL,"
        "detail_json TEXT NULL"
        ")"))
           && q.exec(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_aoi_param_change_time "
               "ON aoi_param_change_log(occurred_at)"))
           && q.exec(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_aoi_param_change_module "
               "ON aoi_param_change_log(module_code, occurred_at)"))
           && q.exec(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_aoi_param_change_pattern "
               "ON aoi_param_change_log(label_pattern, occurred_at)"));
}

QString detailJson(const parammeta::ParamChangeRecord &rec)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("fullLabel"), rec.fullLabel);
    obj.insert(QStringLiteral("labelPattern"), rec.labelPattern);
    obj.insert(QStringLiteral("before"), rec.beforeValue);
    obj.insert(QStringLiteral("after"), rec.afterValue);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

} // namespace

ParamChangeDbStore::ParamChangeDbStore(const QString &dbPath)
    : m_dbPath(dbPath.trimmed().isEmpty() ? defaultDbPath() : dbPath.trimmed())
    , m_connName(QStringLiteral("aoi_param_change_%1").arg(reinterpret_cast<quintptr>(this)))
{
}

ParamChangeDbStore::~ParamChangeDbStore()
{
    // 释放本 store 专属连接，避免连接名泄漏。
    if (QSqlDatabase::contains(m_connName)) {
        QSqlDatabase db = QSqlDatabase::database(m_connName, false);
        if (db.isOpen())
            db.close();
        QSqlDatabase::removeDatabase(m_connName);
    }
}

bool ParamChangeDbStore::ensureConnection()
{
    // 懒加载连接：首次保存时创建并打开数据库连接。
    if (QSqlDatabase::contains(m_connName)) {
        QSqlDatabase db = QSqlDatabase::database(m_connName, false);
        if (db.isOpen())
            return true;
    }

    QFileInfo info(m_dbPath);
    if (!QDir().mkpath(info.dir().path()))
        return false;

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connName);
    db.setDatabaseName(m_dbPath);
    return db.open();
}

bool ParamChangeDbStore::ensureSchema()
{
    // 表结构只初始化一次，减少每次写入开销。
    if (m_schemaReady)
        return true;
    QSqlDatabase db = QSqlDatabase::database(m_connName, false);
    m_schemaReady   = db.isValid() && db.isOpen() && ensureAuditTable(db);
    return m_schemaReady;
}

void ParamChangeDbStore::save(const ParamAuditContext &ctx,
                              const QVector<parammeta::ParamChangeRecord> &changes)
{
    // 存储器只处理落库，不做差异解析。
    if (changes.isEmpty())
        return;
    if (!ensureConnection())
        return;
    if (!ensureSchema())
        return;

    QSqlDatabase db = QSqlDatabase::database(m_connName, false);
    if (!db.isValid() || !db.isOpen())
        return;

    orm::Repository<RevisionHistory> repo;
    for (const parammeta::ParamChangeRecord &change : changes) {
        RevisionHistory entity;
        entity.occurredAt   = nowIso();
        entity.moduleCode   = ctx.moduleCode;
        entity.documentType = ctx.documentType;
        entity.documentId   = ctx.documentId;
        entity.sourcePage   = ctx.sourcePage;
        entity.fullLabel    = change.fullLabel;
        entity.labelPattern =
            parammeta::normalizeMapPattern(parammeta::normalizeArrayPattern(change.labelPattern));
        entity.beforeValue = change.beforeValue;
        entity.afterValue  = change.afterValue;
        entity.valueType   = change.valueType;
        entity.detailJson  = detailJson(change);
        repo.insert(entity, &db);
    }
}

} // namespace aoi
