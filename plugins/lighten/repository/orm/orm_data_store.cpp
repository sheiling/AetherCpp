/**
 * @file orm_data_store.cpp
 * @brief OrmDataStore 生命周期与 SQL 仓储实现
 */

#include "repository/orm/orm_data_store.h"

#include "repository/filter_utils.h"
#include "repository/orm/db_connection.h"
#include "repository/orm/entity_mapper.h"
#include "repository/orm/schema_initializer.h"

#include <interface/ilighten_production_kpi_service.h>

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/** @brief OrmDataStore 与 SQL 历史/KPI 仓储实现 */
namespace lighten {
namespace {

QString appendValidationError(const InspectionHistoryRecord &record)
{
    if (record.model.trimmed().isEmpty())
        return QStringLiteral("product model is required");
    if (record.serialNumber.trimmed().isEmpty())
        return QStringLiteral("board serial number is required");
    return {};
}

void bindHistoryRowValues(QSqlQuery &query, const LightenInspectionHistoryRow &row)
{
    query.bindValue(QStringLiteral(":product_model"), row.product_model);
    query.bindValue(QStringLiteral(":board_sn"), row.board_sn);
    query.bindValue(QStringLiteral(":inspection_result"), row.inspection_result);
    query.bindValue(QStringLiteral(":inspect_times"), row.inspect_times);
    query.bindValue(QStringLiteral(":rw_count"), row.rw_count);
    query.bindValue(QStringLiteral(":inspect_duration_ms"), row.inspect_duration_ms);
    query.bindValue(QStringLiteral(":dead_led_r"), row.dead_led_r);
    query.bindValue(QStringLiteral(":dead_led_g"), row.dead_led_g);
    query.bindValue(QStringLiteral(":dead_led_b"), row.dead_led_b);
    query.bindValue(QStringLiteral(":dim_led_r"), row.dim_led_r);
    query.bindValue(QStringLiteral(":dim_led_g"), row.dim_led_g);
    query.bindValue(QStringLiteral(":dim_led_b"), row.dim_led_b);
    query.bindValue(QStringLiteral(":bright_led_r"), row.bright_led_r);
    query.bindValue(QStringLiteral(":bright_led_g"), row.bright_led_g);
    query.bindValue(QStringLiteral(":bright_led_b"), row.bright_led_b);
    query.bindValue(QStringLiteral(":color_var_r"), row.color_var_r);
    query.bindValue(QStringLiteral(":color_var_g"), row.color_var_g);
    query.bindValue(QStringLiteral(":color_var_b"), row.color_var_b);
    query.bindValue(QStringLiteral(":series_led_r"), row.series_led_r);
    query.bindValue(QStringLiteral(":series_led_g"), row.series_led_g);
    query.bindValue(QStringLiteral(":series_led_b"), row.series_led_b);
    query.bindValue(QStringLiteral(":mixed_color_r"), row.mixed_color_r);
    query.bindValue(QStringLiteral(":mixed_color_g"), row.mixed_color_g);
    query.bindValue(QStringLiteral(":mixed_color_b"), row.mixed_color_b);
    query.bindValue(QStringLiteral(":black_screen_light_count"), row.black_screen_light_count);
    query.bindValue(QStringLiteral(":defect_total_count"), row.defect_total_count);
    query.bindValue(QStringLiteral(":defect_type"), row.defect_type);
    query.bindValue(QStringLiteral(":flash_result"), row.flash_result);
    query.bindValue(QStringLiteral(":special_detect"), row.special_detect);
    query.bindValue(QStringLiteral(":inspected_at"), row.inspected_at);
}

QString historyInsertSql()
{
    return QStringLiteral(
        "INSERT INTO lighten_inspection_history ("
        "product_model, board_sn, inspection_result, inspect_times, rw_count, inspect_duration_ms, "
        "dead_led_r, dead_led_g, dead_led_b, "
        "dim_led_r, dim_led_g, dim_led_b, "
        "bright_led_r, bright_led_g, bright_led_b, "
        "color_var_r, color_var_g, color_var_b, "
        "series_led_r, series_led_g, series_led_b, "
        "mixed_color_r, mixed_color_g, mixed_color_b, "
        "black_screen_light_count, defect_total_count, defect_type, "
        "flash_result, special_detect, inspected_at"
        ") VALUES ("
        ":product_model, :board_sn, :inspection_result, :inspect_times, :rw_count, :inspect_duration_ms, "
        ":dead_led_r, :dead_led_g, :dead_led_b, "
        ":dim_led_r, :dim_led_g, :dim_led_b, "
        ":bright_led_r, :bright_led_g, :bright_led_b, "
        ":color_var_r, :color_var_g, :color_var_b, "
        ":series_led_r, :series_led_g, :series_led_b, "
        ":mixed_color_r, :mixed_color_g, :mixed_color_b, "
        ":black_screen_light_count, :defect_total_count, :defect_type, "
        ":flash_result, :special_detect, :inspected_at"
        ")");
}

QString historyUpdateSql()
{
    return QStringLiteral(
        "UPDATE lighten_inspection_history SET "
        "product_model = :product_model, board_sn = :board_sn, inspection_result = :inspection_result, "
        "inspect_times = :inspect_times, rw_count = :rw_count, inspect_duration_ms = :inspect_duration_ms, "
        "dead_led_r = :dead_led_r, dead_led_g = :dead_led_g, dead_led_b = :dead_led_b, "
        "dim_led_r = :dim_led_r, dim_led_g = :dim_led_g, dim_led_b = :dim_led_b, "
        "bright_led_r = :bright_led_r, bright_led_g = :bright_led_g, bright_led_b = :bright_led_b, "
        "color_var_r = :color_var_r, color_var_g = :color_var_g, color_var_b = :color_var_b, "
        "series_led_r = :series_led_r, series_led_g = :series_led_g, series_led_b = :series_led_b, "
        "mixed_color_r = :mixed_color_r, mixed_color_g = :mixed_color_g, mixed_color_b = :mixed_color_b, "
        "black_screen_light_count = :black_screen_light_count, defect_total_count = :defect_total_count, "
        "defect_type = :defect_type, flash_result = :flash_result, special_detect = :special_detect, "
        "inspected_at = :inspected_at "
        "WHERE id = :id");
}

qint64 findHistoryRowId(QSqlDatabase db, const QString &productModel, const QString &boardSn)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT id FROM lighten_inspection_history "
        "WHERE product_model = :product_model AND board_sn = :board_sn "
        "ORDER BY id DESC LIMIT 1"));
    query.bindValue(QStringLiteral(":product_model"), productModel);
    query.bindValue(QStringLiteral(":board_sn"), boardSn);
    if (!query.exec() || !query.next())
        return -1;
    return query.value(0).toLongLong();
}

qint64 insertHistoryRow(QSqlDatabase db, const LightenInspectionHistoryRow &row, QString *errorOut)
{
    QSqlQuery query(db);
    query.prepare(historyInsertSql());
    bindHistoryRowValues(query, row);
    if (!query.exec()) {
        if (errorOut)
            *errorOut = query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

/** ORM/SQL 层 IInspectionHistoryRepository 实现 */
class OrmInspectionHistoryRepository final : public IInspectionHistoryRepository
{
public:
    QVector<InspectionHistoryRecord> queryAll(const InspectionFilter &filter) const override
    {
        QVector<InspectionHistoryRecord> out;
        QSqlDatabase db = DbConnection::database();
        if (!db.isValid() || !db.isOpen())
            return out;

        QVariantMap binds;
        const QString where = repo_filter::buildWhereClause(filter, &binds);
        QSqlQuery query(db);
        query.prepare(QStringLiteral("SELECT * FROM lighten_inspection_history") + where
                      + QStringLiteral(" ORDER BY inspected_at DESC"));
        for (auto it = binds.constBegin(); it != binds.constEnd(); ++it)
            query.bindValue(it.key(), it.value());

        if (!query.exec())
            return out;

        while (query.next())
            out.append(orm_mapper::toRecord(orm_mapper::rowFromSqlQuery(query)));
        return out;
    }

    InspectionPage queryPage(const InspectionFilter &filter, int pageIndex, int pageSize) const override
    {
        return repo_filter::makePage(queryAll(filter), pageIndex, pageSize);
    }

    InspectionStats statsFor(const InspectionFilter &filter) const override
    {
        return repo_filter::statsFromRows(queryAll(filter));
    }

    QVector<InspectionHistoryRecord> queryByServiceQuery(
        const LightenInspectionHistoryQuery &query) const override
    {
        return queryAll(repo_filter::filterFromServiceQuery(query));
    }

    qint64 appendRecord(const InspectionHistoryRecord &record, QString *errorOut) override
    {
        const QString validationError = appendValidationError(record);
        if (!validationError.isEmpty()) {
            if (errorOut)
                *errorOut = validationError;
            return -1;
        }

        QSqlDatabase db = DbConnection::database();
        if (!db.isValid() || !db.isOpen()) {
            if (errorOut)
                *errorOut = QStringLiteral("database is not open");
            return -1;
        }

        return insertHistoryRow(db, orm_mapper::rowFromRecord(record), errorOut);
    }

    qint64 upsertRecord(const InspectionHistoryRecord &record, QString *errorOut) override
    {
        const QString validationError = appendValidationError(record);
        if (!validationError.isEmpty()) {
            if (errorOut)
                *errorOut = validationError;
            return -1;
        }

        QSqlDatabase db = DbConnection::database();
        if (!db.isValid() || !db.isOpen()) {
            if (errorOut)
                *errorOut = QStringLiteral("database is not open");
            return -1;
        }

        const LightenInspectionHistoryRow row = orm_mapper::rowFromRecord(record);
        const qint64 existingId =
            findHistoryRowId(db, row.product_model, row.board_sn);
        if (existingId < 0)
            return insertHistoryRow(db, row, errorOut);

        QSqlQuery query(db);
        query.prepare(historyUpdateSql());
        bindHistoryRowValues(query, row);
        query.bindValue(QStringLiteral(":id"), existingId);
        if (!query.exec()) {
            if (errorOut)
                *errorOut = query.lastError().text();
            return -1;
        }
        return existingId;
    }
};

LightenProductionKpiRecord kpiFromQuery(const QSqlQuery &query)
{
    LightenProductionKpiRecord kpi;
    kpi.totalLightenedPanels = query.value(QStringLiteral("total_lightened_panels")).toInt();
    kpi.okCount              = query.value(QStringLiteral("ok_count")).toInt();
    kpi.ngCount              = query.value(QStringLiteral("ng_count")).toInt();
    kpi.rwCount              = query.value(QStringLiteral("rw_count")).toInt();
    kpi.insertionCount       = query.value(QStringLiteral("insertion_count")).toInt();
    kpi.passRate             = query.value(QStringLiteral("pass_rate")).toDouble();
    return kpi;
}

/** ORM/SQL 层 IProductionKpiRepository 实现 */
class OrmProductionKpiRepository final : public IProductionKpiRepository
{
public:
    bool load(LightenProductionKpiRecord *out, QString *errorOut) const override
    {
        if (!out) {
            if (errorOut)
                *errorOut = QStringLiteral("output pointer is null");
            return false;
        }

        QSqlDatabase db = DbConnection::database();
        if (!db.isValid() || !db.isOpen()) {
            if (errorOut)
                *errorOut = QStringLiteral("database is not open");
            return false;
        }

        QSqlQuery query(db);
        if (!query.exec(QStringLiteral(
                "SELECT total_lightened_panels, ok_count, ng_count, rw_count, "
                "insertion_count, pass_rate "
                "FROM lighten_production_kpi LIMIT 1"))) {
            if (errorOut)
                *errorOut = query.lastError().text();
            return false;
        }

        if (query.next())
            *out = kpiFromQuery(query);
        else
            *out = LightenProductionKpiRecord{};
        return true;
    }

    bool upsert(const LightenProductionKpiRecord &record, QString *errorOut) override
    {
        QSqlDatabase db = DbConnection::database();
        if (!db.isValid() || !db.isOpen()) {
            if (errorOut)
                *errorOut = QStringLiteral("database is not open");
            return false;
        }

        QSqlQuery countQuery(db);
        if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM lighten_production_kpi"))) {
            if (errorOut)
                *errorOut = countQuery.lastError().text();
            return false;
        }
        if (!countQuery.next()) {
            if (errorOut)
                *errorOut = QStringLiteral("failed to count production kpi rows");
            return false;
        }

        const bool hasRow = countQuery.value(0).toInt() > 0;
        QSqlQuery query(db);
        if (hasRow) {
            query.prepare(QStringLiteral(
                "UPDATE lighten_production_kpi SET "
                "total_lightened_panels = :total_lightened_panels, "
                "ok_count = :ok_count, ng_count = :ng_count, rw_count = :rw_count, "
                "insertion_count = :insertion_count, pass_rate = :pass_rate"));
        } else {
            query.prepare(QStringLiteral(
                "INSERT INTO lighten_production_kpi ("
                "total_lightened_panels, ok_count, ng_count, rw_count, insertion_count, pass_rate"
                ") VALUES ("
                ":total_lightened_panels, :ok_count, :ng_count, :rw_count, :insertion_count, :pass_rate"
                ")"));
        }

        query.bindValue(QStringLiteral(":total_lightened_panels"), record.totalLightenedPanels);
        query.bindValue(QStringLiteral(":ok_count"), record.okCount);
        query.bindValue(QStringLiteral(":ng_count"), record.ngCount);
        query.bindValue(QStringLiteral(":rw_count"), record.rwCount);
        query.bindValue(QStringLiteral(":insertion_count"), record.insertionCount);
        query.bindValue(QStringLiteral(":pass_rate"), record.passRate);

        if (!query.exec()) {
            if (errorOut)
                *errorOut = query.lastError().text();
            return false;
        }
        return true;
    }
};

} // namespace

OrmDataStore::OrmDataStore(StorageConfig config, StorageBackend backend)
    : m_config(std::move(config))
    , m_backend(backend)
    , m_history(std::make_unique<OrmInspectionHistoryRepository>())
    , m_kpi(std::make_unique<OrmProductionKpiRepository>())
{
}

bool OrmDataStore::open(QString *errorString)
{
    if (m_open)
        return true;

    if (!connectDatabase(errorString))
        return false;

    if (m_config.createSchemaOnOpen && !createSchema(errorString))
        return false;

    m_open = true;
    return true;
}

void OrmDataStore::close()
{
    if (!m_open)
        return;
    DbConnection::close();
    m_open = false;
}

} // namespace lighten
