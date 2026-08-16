/**
 * @file inspection_results_model.cpp
 * @brief 检测历史数据访问（经 DataStore 查询 Mock / SQLite / MySQL）
 */

#include "inspection_results_model.h"

#include "repository/datastore.h"
#include "repository/interface/iinspectionhistoryrepository.h"

#include <QFile>
#include <QTextStream>
#include <QTime>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

/** @brief 检测历史筛选模型与 InspectionResultsRepository 门面实现 */
namespace lighten {

namespace {

/** CSV 字段转义：含逗号、引号或换行时用双引号包裹 */
QString csvField(const QString &value)
{
    if (!value.contains(QLatin1Char('"')) && !value.contains(QLatin1Char(','))
        && !value.contains(QLatin1Char('\n')) && !value.contains(QLatin1Char('\r')))
        return value;

    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

/** 与检测结果页历史宽表 27 列对齐的 CSV 表头（QStringLiteral 避免 const char* + QLatin1String 中文乱码） */
QString inspectionHistoryCsvHeader()
{
    return QStringLiteral(
        "产品型号,灯板编号,检测结果,检测次数,返工次数,黑屏亮灯,"
        "死灯R,死灯G,死灯B,暗灯R,暗灯G,暗灯B,亮灯R,亮灯G,亮灯B,"
        "串色R,串色G,串色B,串灯R,串灯G,串灯B,混色R,混色G,混色B,"
        "Flash结果,特殊检测项,检测时间\n");
}

QString recordToCsvLine(const InspectionHistoryRecord &row)
{
    QStringList fields;
    fields << csvField(row.model.trimmed())
           << csvField(row.serialNumber.trimmed())
           << csvField(inspectionResultText(row.result))
           << QString::number(row.inspectTimes)
           << QString::number(row.rwCount)
           << QString::number(row.blackScreenLightCount);
    for (int i = 0; i < kDefectRgbCount; ++i)
        fields << QString::number(row.defectCounts[i]);
    fields << csvField(flashResultText(row.flashResult))
           << csvField(row.specialDetect.trimmed())
           << csvField(row.inspectedAt.trimmed());
    return fields.join(QLatin1Char(',')) + QLatin1Char('\n');
}

} // namespace

bool InspectionFilter::operator==(const InspectionFilter &o) const
{
    return productModel == o.productModel && boardSerial == o.boardSerial
           && resultFilter == o.resultFilter && defectType == o.defectType
           && flashResultFilter == o.flashResultFilter && timeFrom == o.timeFrom
           && timeTo == o.timeTo;
}

InspectionFilter InspectionFilter::defaultToday()
{
    InspectionFilter f;
    const QDateTime now = QDateTime::currentDateTime();
    f.timeFrom          = QDateTime(now.date(), QTime(0, 0));
    f.timeTo            = now;
    f.productModel      = QStringLiteral("全部");
    f.resultFilter         = QStringLiteral("全部");
    f.defectType           = QStringLiteral("全部");
    f.flashResultFilter    = QStringLiteral("全部");
    return f;
}

/** 返回当前 DataStore 历史仓储；未 initialize 时为 nullptr */
static IInspectionHistoryRepository *historyRepo()
{
    if (IDataStore *store = DataStore::instance())
        return &store->inspectionHistory();
    return nullptr;
}

InspectionPage InspectionResultsRepository::query(const InspectionFilter &filter, int pageIndex,
                                                  int pageSize)
{
    if (auto *repo = historyRepo())
        return repo->queryPage(filter, pageIndex, pageSize);
    return {};
}

InspectionStats InspectionResultsRepository::statsFor(const InspectionFilter &filter)
{
    if (auto *repo = historyRepo())
        return repo->statsFor(filter);
    return {};
}

bool InspectionResultsRepository::exportToCsv(const InspectionFilter &filter,
                                            const QString &filePath)
{
    auto *repo = historyRepo();
    if (!repo)
        return false;

    const auto rows = repo->queryAll(filter);
    QFile      f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream ts(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Utf8);
#else
    ts.setCodec("UTF-8");
#endif
    ts << inspectionHistoryCsvHeader();
    for (const auto &row : rows)
        ts << recordToCsvLine(row);
    return true;
}

} // namespace lighten
