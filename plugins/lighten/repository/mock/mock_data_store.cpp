/**
 * @file mock_data_store.cpp
 * @brief MockDataStore 内存仓储实现（无演示数据）
 */

#include "repository/mock/mock_data_store.h"

#include "repository/filter_utils.h"

#include <interface/ilighten_production_kpi_service.h>

#include <QDateTime>

/** @brief Mock 内存仓储实现 */
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

InspectionHistoryRecord prepareStoredRecord(InspectionHistoryRecord record)
{
    if (record.inspectedAt.trimmed().isEmpty())
        record.inspectedAt =
            QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    return record;
}

int findHistoryRowIndex(const QVector<InspectionHistoryRecord> &rows, const QString &model,
                        const QString &boardSn)
{
    for (int i = 0; i < rows.size(); ++i) {
        if (rows[i].model == model && rows[i].serialNumber == boardSn)
            return i;
    }
    return -1;
}

/** Mock 层 IInspectionHistoryRepository 实现（基于内存 QVector 筛选分页） */
class MockInspectionHistoryRepository final : public IInspectionHistoryRepository
{
public:
    explicit MockInspectionHistoryRepository(QVector<InspectionHistoryRecord> *rows)
        : m_rows(rows)
    {
    }

    QVector<InspectionHistoryRecord> queryAll(const InspectionFilter &filter) const override
    {
        QVector<InspectionHistoryRecord> out;
        for (const auto &row : *m_rows) {
            if (repo_filter::matchesRecord(row, filter))
                out.append(row);
        }
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

        m_rows->prepend(prepareStoredRecord(record));
        return static_cast<qint64>(m_rows->size());
    }

    qint64 upsertRecord(const InspectionHistoryRecord &record, QString *errorOut) override
    {
        const QString validationError = appendValidationError(record);
        if (!validationError.isEmpty()) {
            if (errorOut)
                *errorOut = validationError;
            return -1;
        }

        const QString model = record.model.trimmed();
        const QString boardSn = record.serialNumber.trimmed();
        const int index = findHistoryRowIndex(*m_rows, model, boardSn);
        const InspectionHistoryRecord stored = prepareStoredRecord(record);
        if (index < 0) {
            m_rows->prepend(stored);
            return static_cast<qint64>(m_rows->size());
        }

        (*m_rows)[index] = stored;
        return static_cast<qint64>(index + 1);
    }

private:
    QVector<InspectionHistoryRecord> *m_rows;
};

/** Mock 层 IProductionKpiRepository 实现 */
class MockProductionKpiRepository final : public IProductionKpiRepository
{
public:
    explicit MockProductionKpiRepository(LightenProductionKpiRecord *row)
        : m_row(row)
    {
    }

    bool load(LightenProductionKpiRecord *out, QString *errorOut) const override
    {
        if (!out) {
            if (errorOut)
                *errorOut = QStringLiteral("output pointer is null");
            return false;
        }
        *out = *m_row;
        return true;
    }

    bool upsert(const LightenProductionKpiRecord &record, QString *errorOut) override
    {
        Q_UNUSED(errorOut);
        *m_row = record;
        return true;
    }

private:
    LightenProductionKpiRecord *m_row;
};

} // namespace

MockDataStore::MockDataStore(StorageConfig config)
    : m_config(std::move(config))
    , m_history(std::make_unique<MockInspectionHistoryRepository>(&m_rows))
    , m_kpi(std::make_unique<MockProductionKpiRepository>(&m_kpiRow))
{
}

bool MockDataStore::open(QString *)
{
    m_open = true;
    return true;
}

void MockDataStore::close()
{
    m_open = false;
}

} // namespace lighten
