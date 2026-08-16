/**
 * @file inspection_history_repository.cpp
 */

#include "inspection_history_repository.h"

#include "repository/aoi_db_paths.h"
#include "repository/orm/schema_initializer.h"
#include "service/inspection/history/inspection_history_mapper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace aoi::inspection {
namespace {

QString defaultDbPath()
{
    return repository::defaultRecordDbPath();
}

QString sqlEscapeLike(const QString &s)
{
    QString out = s;
    out.replace(QLatin1Char('%'), QStringLiteral("\\%"));
    out.replace(QLatin1Char('_'), QStringLiteral("\\_"));
    return out;
}

struct SqlWhere
{
    QString clause;
    QVariantList binds;
};

SqlWhere buildWhere(const InspectionHistoryQuery &query)
{
    SqlWhere w;
    QStringList parts;

    if (!query.productName.trimmed().isEmpty()) {
        parts.append(QStringLiteral("product_name LIKE ? ESCAPE '\\'"));
        w.binds.append(QStringLiteral("%%1%").arg(sqlEscapeLike(query.productName.trimmed())));
    }
    if (!query.sn.trimmed().isEmpty()) {
        parts.append(QStringLiteral("sn LIKE ? ESCAPE '\\'"));
        w.binds.append(QStringLiteral("%%1%").arg(sqlEscapeLike(query.sn.trimmed())));
    }
    if (!query.resultFilter.trimmed().isEmpty() && query.resultFilter != QStringLiteral("全部")) {
        int code = -1;
        if (query.resultFilter == QStringLiteral("OK"))
            code = static_cast<int>(InspectionHistoryResult::Ok);
        else if (query.resultFilter == QStringLiteral("NG"))
            code = static_cast<int>(InspectionHistoryResult::Ng);
        else if (query.resultFilter == QStringLiteral("RW"))
            code = static_cast<int>(InspectionHistoryResult::Rw);
        if (code >= 0) {
            parts.append(QStringLiteral("inspection_result = ?"));
            w.binds.append(code);
        }
    }
    if (!query.defectTypeFilter.trimmed().isEmpty() && query.defectTypeFilter != QStringLiteral("全部")) {
        bool ok   = false;
        const int idx = query.defectTypeFilter.toInt(&ok);
        if (ok && idx >= 0 && idx < kInspectionDefectTypeCount) {
            parts.append(QStringLiteral("defect_count_%1 > 0").arg(idx));
        }
    }
    if (query.timeFrom.isValid()) {
        parts.append(QStringLiteral("detect_time >= ?"));
        w.binds.append(query.timeFrom.toString(Qt::ISODate));
    }
    if (query.timeTo.isValid()) {
        parts.append(QStringLiteral("detect_time <= ?"));
        w.binds.append(query.timeTo.toString(Qt::ISODate));
    }

    if (!parts.isEmpty())
        w.clause = QStringLiteral(" WHERE ") + parts.join(QStringLiteral(" AND "));
    return w;
}

void bindQueryValues(QSqlQuery &q, const QVariantList &binds, int startIndex = 0)
{
    for (int i = 0; i < binds.size(); ++i)
        q.bindValue(startIndex + i, binds.at(i));
}

InspectionHistoryRow rowFromQuery(const QSqlQuery &query)
{
    InspectionHistoryRow row;
    row.id                     = query.value(QStringLiteral("id")).toLongLong();
    row.productName            = query.value(QStringLiteral("product_name")).toString();
    row.sn                     = query.value(QStringLiteral("sn")).toString();
    row.detectTime             = query.value(QStringLiteral("detect_time")).toString();
    row.result                 = static_cast<InspectionHistoryResult>(query.value(QStringLiteral("inspection_result")).toInt());
    row.inspectTimes           = query.value(QStringLiteral("inspect_times")).toInt();
    row.rwCount                = query.value(QStringLiteral("rw_count")).toInt();
    row.defectPartitionsJson   = query.value(QStringLiteral("defect_partitions")).toString();
    row.defectInfoFullFileName = query.value(QStringLiteral("defect_info_full_file_name")).toString();
    row.validDefectType        = static_cast<InspectionDefectTypeMask>(query.value(QStringLiteral("valid_defect_type")).toULongLong());
    row.append                 = query.value(QStringLiteral("append")).toInt() != 0;
    row.chipGroupRowCount      = query.value(QStringLiteral("chip_group_row_count")).toInt();
    row.chipGroupColCount      = query.value(QStringLiteral("chip_group_col_count")).toInt();
    row.dieRowCount            = query.value(QStringLiteral("die_row_count")).toInt();
    row.dieColCount            = query.value(QStringLiteral("die_col_count")).toInt();
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        row.defectCounts[i] = query.value(QStringLiteral("defect_count_%1").arg(i)).toInt();
    return row;
}

bool bindInsertRow(QSqlQuery &q, const InspectionHistoryRow &row)
{
    q.bindValue(QStringLiteral(":product_name"), row.productName);
    q.bindValue(QStringLiteral(":sn"), row.sn);
    q.bindValue(QStringLiteral(":detect_time"), row.detectTime);
    q.bindValue(QStringLiteral(":inspection_result"), static_cast<int>(row.result));
    q.bindValue(QStringLiteral(":inspect_times"), row.inspectTimes);
    q.bindValue(QStringLiteral(":rw_count"), row.rwCount);
    q.bindValue(QStringLiteral(":defect_partitions"), row.defectPartitionsJson);
    q.bindValue(QStringLiteral(":defect_info_full_file_name"), row.defectInfoFullFileName);
    q.bindValue(QStringLiteral(":valid_defect_type"), static_cast<qulonglong>(row.validDefectType));
    q.bindValue(QStringLiteral(":append"), row.append ? 1 : 0);
    q.bindValue(QStringLiteral(":chip_group_row_count"), row.chipGroupRowCount);
    q.bindValue(QStringLiteral(":chip_group_col_count"), row.chipGroupColCount);
    q.bindValue(QStringLiteral(":die_row_count"), row.dieRowCount);
    q.bindValue(QStringLiteral(":die_col_count"), row.dieColCount);
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        q.bindValue(QStringLiteral(":defect_count_%1").arg(i), row.defectCounts[i]);
    return true;
}

QString insertColumnsSql()
{
    QStringList cols = {QStringLiteral("product_name"),
                        QStringLiteral("sn"),
                        QStringLiteral("detect_time"),
                        QStringLiteral("inspection_result"),
                        QStringLiteral("inspect_times"),
                        QStringLiteral("rw_count"),
                        QStringLiteral("defect_partitions"),
                        QStringLiteral("defect_info_full_file_name"),
                        QStringLiteral("valid_defect_type"),
                        QStringLiteral("append"),
                        QStringLiteral("chip_group_row_count"),
                        QStringLiteral("chip_group_col_count"),
                        QStringLiteral("die_row_count"),
                        QStringLiteral("die_col_count")};
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        cols.append(QStringLiteral("defect_count_%1").arg(i));
    return cols.join(QLatin1Char(','));
}

QString insertPlaceholdersSql()
{
    QStringList ph;
    ph << QStringLiteral(":product_name") << QStringLiteral(":sn") << QStringLiteral(":detect_time")
       << QStringLiteral(":inspection_result") << QStringLiteral(":inspect_times") << QStringLiteral(":rw_count")
       << QStringLiteral(":defect_partitions") << QStringLiteral(":defect_info_full_file_name")
       << QStringLiteral(":valid_defect_type") << QStringLiteral(":append") << QStringLiteral(":chip_group_row_count")
       << QStringLiteral(":chip_group_col_count") << QStringLiteral(":die_row_count") << QStringLiteral(":die_col_count");
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        ph.append(QStringLiteral(":defect_count_%1").arg(i));
    return ph.join(QLatin1Char(','));
}

QString updateSetSql()
{
    QStringList sets = {QStringLiteral("product_name = :product_name"),
                        QStringLiteral("sn = :sn"),
                        QStringLiteral("detect_time = :detect_time"),
                        QStringLiteral("inspection_result = :inspection_result"),
                        QStringLiteral("inspect_times = :inspect_times"),
                        QStringLiteral("rw_count = :rw_count"),
                        QStringLiteral("defect_partitions = :defect_partitions"),
                        QStringLiteral("defect_info_full_file_name = :defect_info_full_file_name"),
                        QStringLiteral("valid_defect_type = :valid_defect_type"),
                        QStringLiteral("append = :append"),
                        QStringLiteral("chip_group_row_count = :chip_group_row_count"),
                        QStringLiteral("chip_group_col_count = :chip_group_col_count"),
                        QStringLiteral("die_row_count = :die_row_count"),
                        QStringLiteral("die_col_count = :die_col_count")};
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        sets.append(QStringLiteral("defect_count_%1 = :defect_count_%1").arg(i));
    return sets.join(QStringLiteral(", "));
}

} // namespace

InspectionHistoryRepository &InspectionHistoryRepository::instance()
{
    static InspectionHistoryRepository repo;
    return repo;
}

InspectionHistoryRepository::InspectionHistoryRepository()
    : m_dbPath(defaultDbPath())
    , m_connName(QStringLiteral("aoi_inspection_history_%1").arg(reinterpret_cast<quintptr>(this)))
{
}

InspectionHistoryRepository::~InspectionHistoryRepository()
{
    if (QSqlDatabase::contains(m_connName)) {
        QSqlDatabase db = QSqlDatabase::database(m_connName, false);
        if (db.isOpen())
            db.close();
        QSqlDatabase::removeDatabase(m_connName);
    }
}

bool InspectionHistoryRepository::isReady() const
{
    if (!QSqlDatabase::contains(m_connName))
        return false;
    const QSqlDatabase db = QSqlDatabase::database(m_connName, false);
    return db.isValid() && db.isOpen();
}

bool InspectionHistoryRepository::ensureConnection()
{
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

bool InspectionHistoryRepository::ensureSchema()
{
    if (m_schemaReady)
        return true;
    if (!ensureConnection())
        return false;

    QSqlDatabase db = QSqlDatabase::database(m_connName, false);
    m_schemaReady   = repository::SchemaInitializer::applyInspectionHistorySchema(db, nullptr);
    return m_schemaReady;
}

qint64 InspectionHistoryRepository::appendRecord(const DefectInfoList &list, QString *errorOut)
{
    if (!ensureConnection() || !ensureSchema()) {
        if (errorOut)
            *errorOut = QStringLiteral("inspection history database not ready");
        return -1;
    }

    InspectionHistoryRow row = rowFromDefectInfoList(list);
    if (row.sn.trimmed().isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("SN is empty");
        return -1;
    }

    QSqlDatabase db = QSqlDatabase::database(m_connName, false);
    QSqlQuery    q(db);

    q.prepare(QStringLiteral("SELECT COUNT(*) FROM aoi_inspection_history WHERE sn = ?"));
    q.addBindValue(row.sn);
    if (!q.exec() || !q.next()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        return -1;
    }
    row.inspectTimes = q.value(0).toInt() + 1;

    q.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM aoi_inspection_history WHERE sn = ? AND inspection_result = ?"));
    q.addBindValue(row.sn);
    q.addBindValue(static_cast<int>(InspectionHistoryResult::Rw));
    int rwHist = 0;
    if (q.exec() && q.next())
        rwHist = q.value(0).toInt();
    row.rwCount = rwHist + (row.result == InspectionHistoryResult::Rw ? 1 : 0);

    const QString sql = QStringLiteral("INSERT INTO aoi_inspection_history (%1) VALUES (%2)")
                            .arg(insertColumnsSql(), insertPlaceholdersSql());
    q.prepare(sql);
    bindInsertRow(q, row);
    if (!q.exec()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

bool InspectionHistoryRepository::updateRecord(qint64 id, const DefectInfoList &list, QString *errorOut)
{
    if (id < 1)
        return updateRecordBySnTime(list.SN, list.detectTime, list, errorOut);
    if (!ensureConnection() || !ensureSchema()) {
        if (errorOut)
            *errorOut = QStringLiteral("inspection history database not ready");
        return false;
    }

    InspectionHistoryRow row = rowFromDefectInfoList(list);
    row.id = id;

    QSqlQuery q(QSqlDatabase::database(m_connName, false));
    q.prepare(QStringLiteral("SELECT inspect_times, rw_count FROM aoi_inspection_history WHERE id = ?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        row.inspectTimes = q.value(0).toInt();
        const int storedRw = q.value(1).toInt();
        if (row.result == InspectionHistoryResult::Rw && storedRw == 0)
            row.rwCount = 1;
        else
            row.rwCount = storedRw;
    }

    const QString sql =
        QStringLiteral("UPDATE aoi_inspection_history SET %1 WHERE id = :row_id").arg(updateSetSql());
    q.prepare(sql);
    bindInsertRow(q, row);
    q.bindValue(QStringLiteral(":row_id"), id);
    if (!q.exec()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool InspectionHistoryRepository::updateRecordBySnTime(const QString &sn, const QString &detectTime,
                                                      const DefectInfoList &list, QString *errorOut)
{
    if (!ensureConnection() || !ensureSchema()) {
        if (errorOut)
            *errorOut = QStringLiteral("inspection history database not ready");
        return false;
    }

    QSqlQuery findQ(QSqlDatabase::database(m_connName, false));
    findQ.prepare(QStringLiteral("SELECT id FROM aoi_inspection_history WHERE sn = ? AND detect_time = ?"));
    findQ.addBindValue(sn);
    findQ.addBindValue(detectTime);
    if (!findQ.exec() || !findQ.next()) {
        if (errorOut)
            *errorOut = QStringLiteral("history row not found for sn/time");
        return false;
    }
    return updateRecord(findQ.value(0).toLongLong(), list, errorOut);
}

InspectionHistoryPage InspectionHistoryRepository::queryPage(const InspectionHistoryQuery &query,
                                                             QString *errorOut) const
{
    InspectionHistoryPage page;
    page.pageSize  = qMax(1, query.pageSize);
    page.pageIndex = qMax(0, query.pageIndex);

    if (!const_cast<InspectionHistoryRepository *>(this)->ensureConnection()
        || !const_cast<InspectionHistoryRepository *>(this)->ensureSchema()) {
        if (errorOut)
            *errorOut = QStringLiteral("inspection history database not ready");
        return page;
    }

    const SqlWhere where = buildWhere(query);
    QSqlDatabase   db    = QSqlDatabase::database(m_connName, false);
    QSqlQuery      q(db);

    q.prepare(QStringLiteral("SELECT COUNT(*) FROM aoi_inspection_history") + where.clause);
    bindQueryValues(q, where.binds);
    if (!q.exec() || !q.next()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        return page;
    }

    page.totalCount = q.value(0).toInt();
    page.totalPages = qMax(1, (page.totalCount + page.pageSize - 1) / page.pageSize);
    page.pageIndex  = qBound(0, page.pageIndex, page.totalPages - 1);

    const int offset = page.pageIndex * page.pageSize;
    q.prepare(QStringLiteral("SELECT * FROM aoi_inspection_history") + where.clause
              + QStringLiteral(" ORDER BY detect_time DESC LIMIT %1 OFFSET %2")
                    .arg(page.pageSize)
                    .arg(offset));
    bindQueryValues(q, where.binds);
    if (!q.exec()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        page.rows.clear();
        return page;
    }

    while (q.next())
        page.rows.append(rowFromQuery(q));
    return page;
}

InspectionHistoryStats InspectionHistoryRepository::queryStats(const InspectionHistoryQuery &query,
                                                               QString *errorOut) const
{
    InspectionHistoryStats stats;
    if (!const_cast<InspectionHistoryRepository *>(this)->ensureConnection()
        || !const_cast<InspectionHistoryRepository *>(this)->ensureSchema()) {
        if (errorOut)
            *errorOut = QStringLiteral("inspection history database not ready");
        return stats;
    }

    const SqlWhere where = buildWhere(query);
    QSqlQuery      q(QSqlDatabase::database(m_connName, false));
    q.prepare(QStringLiteral(
                  "SELECT inspection_result, COUNT(*) AS c FROM aoi_inspection_history")
              + where.clause + QStringLiteral(" GROUP BY inspection_result"));
    bindQueryValues(q, where.binds);
    if (!q.exec()) {
        if (errorOut)
            *errorOut = q.lastError().text();
        return stats;
    }

    while (q.next()) {
        const int  code = q.value(0).toInt();
        const int  c    = q.value(1).toInt();
        stats.totalCount += c;
        switch (static_cast<InspectionHistoryResult>(code)) {
        case InspectionHistoryResult::Ok:
            stats.okCount += c;
            break;
        case InspectionHistoryResult::Ng:
            stats.ngCount += c;
            break;
        case InspectionHistoryResult::Rw:
            stats.rwCount += c;
            break;
        }
    }
    stats.passRatePercent =
        stats.totalCount > 0 ? (100.0 * stats.okCount / stats.totalCount) : 0.0;
    return stats;
}

bool InspectionHistoryRepository::exportToCsv(const InspectionHistoryQuery &query, const QString &filePath,
                                              QString *errorOut) const
{
    InspectionHistoryQuery all = query;
    all.pageIndex              = 0;
    all.pageSize               = 1000000;
    const InspectionHistoryPage page = queryPage(all, errorOut);
    if (errorOut && !errorOut->isEmpty())
        return false;
    return writeInspectionHistoryCsv(page.rows, filePath, errorOut);
}

} // namespace aoi::inspection
