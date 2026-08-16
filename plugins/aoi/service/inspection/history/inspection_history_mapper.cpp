/**
 * @file inspection_history_mapper.cpp
 */

#include "inspection_history_mapper.h"

#include "defect_info_codec.h"

#include "languagemanager.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace aoi::inspection {
namespace {

QString csvEscapeField(const QString &field)
{
    if (field.contains(QLatin1Char('"')) || field.contains(QLatin1Char(',')) || field.contains(QLatin1Char('\n'))
        || field.contains(QLatin1Char('\r')))
        return QStringLiteral("\"%1\"").arg(QString(field).replace(QLatin1Char('"'), QStringLiteral("\"\"")));
    return field;
}

QString historyResultCsvText(InspectionHistoryResult result)
{
    switch (result) {
    case InspectionHistoryResult::Ok:
        return QStringLiteral("OK");
    case InspectionHistoryResult::Ng:
        return QStringLiteral("NG");
    case InspectionHistoryResult::Rw:
        return QStringLiteral("RW");
    }
    return QStringLiteral("—");
}

QString formatDetectTimeForCsv(const QString &iso)
{
    const QDateTime dt = QDateTime::fromString(iso, Qt::ISODate);
    if (dt.isValid())
        return dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    return iso;
}

void appendCsvRow(QTextStream &out, const InspectionHistoryRow &row)
{
    QStringList cols;
    cols << csvEscapeField(row.productName) << csvEscapeField(row.sn)
         << csvEscapeField(historyResultCsvText(row.result)) << QString::number(row.inspectTimes)
         << QString::number(row.rwCount);
    for (int i = 0; i < kInspectionDefectTypeCount; ++i)
        cols << QString::number(row.defectCounts[i]);
    cols << csvEscapeField(formatDetectTimeForCsv(row.detectTime));
    out << cols.join(QLatin1Char(',')) << QLatin1Char('\n');
}

QStringList inspectionHistoryCsvHeaders()
{
    QStringList headers;
    headers << TR("aoi.insp.col.model") << TR("aoi.insp.col.sn") << TR("aoi.insp.col.result")
            << TR("aoi.insp.col.inspect_count") << TR("aoi.insp.col.rework_count");
    for (int i = 0; i < kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<InspectionDefectType>(i);
        headers << TR(inspectionDefectTypeDisplayKey(type));
    }
    headers << TR("aoi.insp.col.time");
    return headers;
}

void countMask(InspectionDefectTypeMask mask, std::array<int, kInspectionDefectTypeCount> *counts)
{
    if (!counts)
        return;
    for (int i = 0; i < kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<InspectionDefectType>(i);
        if (inspectionMaskHasDefect(mask, type))
            (*counts)[i]++;
    }
}

InspectionDefectTypeMask effectiveItemMask(const DefectInfoItem &item)
{
    if (item.isRejudge)
        return item.rejudgeDefectType;
    return item.defectType;
}

} // namespace

std::array<int, kInspectionDefectTypeCount> defectCountsFromList(const DefectInfoList &list)
{
    std::array<int, kInspectionDefectTypeCount> counts{};
    for (const DefectInfoPartition &part : list.defectPartitions) {
        for (const DefectInfoItem &item : part.defectItems)
            countMask(effectiveItemMask(item), &counts);
    }
    return counts;
}

InspectionHistoryResult inspectionResultFromList(const DefectInfoList &list)
{
    bool hasDefectItem = false;
    bool hasPending    = false;
    bool hasRejudgeNg  = false;

    for (const DefectInfoPartition &part : list.defectPartitions) {
        for (const DefectInfoItem &item : part.defectItems) {
            if (item.defectType == 0 && !item.isRejudge)
                continue;
            hasDefectItem = true;
            if (!item.isRejudge && item.defectType != 0)
                hasPending = true;
            if (item.isRejudge && item.rejudgeDefectType != 0)
                hasRejudgeNg = true;
        }
    }

    if (!hasDefectItem)
        return InspectionHistoryResult::Ok;
    if (hasPending)
        return InspectionHistoryResult::Rw;
    return hasRejudgeNg ? InspectionHistoryResult::Ng : InspectionHistoryResult::Ok;
}

InspectionHistoryRow rowFromDefectInfoList(const DefectInfoList &list)
{
    InspectionHistoryRow row;
    row.productName            = list.productName;
    row.sn                     = list.SN;
    row.detectTime             = list.detectTime;
    row.result                 = inspectionResultFromList(list);
    row.defectCounts           = defectCountsFromList(list);
    row.defectPartitionsJson   = defectPartitionsToJson(list.defectPartitions);
    row.defectInfoFullFileName = list.defectInfoFullFileName;
    row.validDefectType        = list.validDefectType;
    row.append                 = list.append;
    row.chipGroupRowCount      = list.chipGroupRowCount;
    row.chipGroupColCount      = list.chipGroupColCount;
    row.dieRowCount            = list.dieRowCount;
    row.dieColCount            = list.dieColCount;
    return row;
}

DefectInfoList defectInfoListFromRow(const InspectionHistoryRow &row)
{
    DefectInfoList list;
    list.productName            = row.productName;
    list.SN                     = row.sn;
    list.detectTime             = row.detectTime;
    list.defectInfoFullFileName = row.defectInfoFullFileName;
    list.validDefectType        = row.validDefectType;
    list.append                 = row.append;
    list.chipGroupRowCount      = row.chipGroupRowCount;
    list.chipGroupColCount      = row.chipGroupColCount;
    list.dieRowCount            = row.dieRowCount;
    list.dieColCount            = row.dieColCount;
    defectPartitionsFromJson(row.defectPartitionsJson, &list.defectPartitions, nullptr);
    return list;
}

bool writeInspectionHistoryCsv(const QVector<InspectionHistoryRow> &rows, const QString &filePath,
                               QString *errorOut)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorOut)
            *errorOut = file.errorString();
        return false;
    }

    file.write("\xEF\xBB\xBF", 3);

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    const QStringList headers = inspectionHistoryCsvHeaders();
    out << headers.join(QLatin1Char(',')) << QLatin1Char('\n');
    for (const InspectionHistoryRow &row : rows)
        appendCsvRow(out, row);
    return true;
}

} // namespace aoi::inspection
