/**
 * @file defect_info_codec.cpp
 * @brief DefectInfoList JSON 编解码实现
 */

#include "defect_info_codec.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace aoi::inspection {
namespace {

QJsonObject rectToJson(const aoi::Rect &r)
{
    QJsonObject o;
    o.insert(QStringLiteral("x"), r.x);
    o.insert(QStringLiteral("y"), r.y);
    o.insert(QStringLiteral("w"), r.w);
    o.insert(QStringLiteral("h"), r.h);
    return o;
}

aoi::Rect rectFromJson(const QJsonObject &o)
{
    aoi::Rect r;
    r.x = o.value(QStringLiteral("x")).toInt();
    r.y = o.value(QStringLiteral("y")).toInt();
    r.w = o.value(QStringLiteral("w")).toInt();
    r.h = o.value(QStringLiteral("h")).toInt();
    return r;
}

QJsonObject itemToJson(const DefectInfoItem &item)
{
    QJsonObject o;
    o.insert(QStringLiteral("chipType"), item.chipType);
    o.insert(QStringLiteral("defectType"), static_cast<qint64>(item.defectType));
    o.insert(QStringLiteral("rejudgeDefectType"), static_cast<qint64>(item.rejudgeDefectType));
    o.insert(QStringLiteral("isRejudge"), item.isRejudge);
    o.insert(QStringLiteral("colIndexInPartition"), item.colIndexInPartition);
    o.insert(QStringLiteral("rowIndexInPartition"), item.rowIndexInPartition);
    o.insert(QStringLiteral("colIndexInPanel"), item.colIndexInPanel);
    o.insert(QStringLiteral("rowIndexInPanel"), item.rowIndexInPanel);
    o.insert(QStringLiteral("dieColIndexInPanel"), item.dieColIndexInPanel);
    o.insert(QStringLiteral("dieRowIndexInPanel"), item.dieRowIndexInPanel);
    o.insert(QStringLiteral("roi"), rectToJson(item.roi));
    return o;
}

bool itemFromJson(const QJsonObject &o, DefectInfoItem *item)
{
    if (!item)
        return false;
    DefectInfoItem out;
    out.chipType            = o.value(QStringLiteral("chipType")).toInt();
    out.defectType          = static_cast<InspectionDefectTypeMask>(o.value(QStringLiteral("defectType")).toVariant().toULongLong());
    out.rejudgeDefectType   = static_cast<InspectionDefectTypeMask>(o.value(QStringLiteral("rejudgeDefectType")).toVariant().toULongLong());
    out.isRejudge           = o.value(QStringLiteral("isRejudge")).toBool();
    out.colIndexInPartition = o.value(QStringLiteral("colIndexInPartition")).toInt();
    out.rowIndexInPartition = o.value(QStringLiteral("rowIndexInPartition")).toInt();
    out.colIndexInPanel     = o.value(QStringLiteral("colIndexInPanel")).toInt();
    out.rowIndexInPanel     = o.value(QStringLiteral("rowIndexInPanel")).toInt();
    out.dieColIndexInPanel  = o.value(QStringLiteral("dieColIndexInPanel")).toInt();
    out.dieRowIndexInPanel  = o.value(QStringLiteral("dieRowIndexInPanel")).toInt();
    out.roi                 = rectFromJson(o.value(QStringLiteral("roi")).toObject());
    *item                   = out;
    return true;
}

QJsonObject partitionToJson(const DefectInfoPartition &part)
{
    QJsonArray items;
    for (const DefectInfoItem &item : part.defectItems)
        items.append(itemToJson(item));

    QJsonObject po;
    po.insert(QStringLiteral("partitionIndex"), part.partitionIndex);
    po.insert(QStringLiteral("partitionGridRow"), part.partitionGridRow);
    po.insert(QStringLiteral("partitionGridCol"), part.partitionGridCol);
    po.insert(QStringLiteral("partitionImageOriginName"), part.partitionImageOriginName);
    po.insert(QStringLiteral("partitionImageDetectName"), part.partitionImageDetectName);
    po.insert(QStringLiteral("defectItems"), items);
    return po;
}

bool partitionFromJson(const QJsonObject &po, DefectInfoPartition *part)
{
    if (!part)
        return false;
    DefectInfoPartition out;
    out.partitionIndex           = po.value(QStringLiteral("partitionIndex")).toInt();
    out.partitionGridRow         = po.value(QStringLiteral("partitionGridRow")).toInt();
    out.partitionGridCol         = po.value(QStringLiteral("partitionGridCol")).toInt();
    out.partitionImageOriginName = po.value(QStringLiteral("partitionImageOriginName")).toString();
    out.partitionImageDetectName = po.value(QStringLiteral("partitionImageDetectName")).toString();

    const QJsonArray items = po.value(QStringLiteral("defectItems")).toArray();
    for (const QJsonValue &v : items) {
        DefectInfoItem item;
        if (!itemFromJson(v.toObject(), &item))
            return false;
        out.defectItems.append(item);
    }
    *part = out;
    return true;
}

} // namespace

QString defectPartitionsToJson(const QVector<DefectInfoPartition> &partitions)
{
    QJsonArray arr;
    for (const DefectInfoPartition &part : partitions)
        arr.append(partitionToJson(part));
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

bool defectPartitionsFromJson(const QString &json, QVector<DefectInfoPartition> *out, QString *errorOut)
{
    if (!out) {
        if (errorOut)
            *errorOut = QStringLiteral("defectPartitionsFromJson: null out");
        return false;
    }
    out->clear();
    if (json.trimmed().isEmpty())
        return true;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (!doc.isArray()) {
        if (errorOut)
            *errorOut = err.errorString();
        return false;
    }

    for (const QJsonValue &v : doc.array()) {
        DefectInfoPartition part;
        if (!partitionFromJson(v.toObject(), &part)) {
            if (errorOut)
                *errorOut = QStringLiteral("invalid partition json");
            return false;
        }
        out->append(part);
    }
    return true;
}

QString defectInfoListToJson(const DefectInfoList &list)
{
    QJsonObject root;
    root.insert(QStringLiteral("defectInfoFullFileName"), list.defectInfoFullFileName);
    root.insert(QStringLiteral("detectTime"), list.detectTime);
    root.insert(QStringLiteral("validDefectType"), static_cast<qint64>(list.validDefectType));
    root.insert(QStringLiteral("append"), list.append);
    root.insert(QStringLiteral("productName"), list.productName);
    root.insert(QStringLiteral("SN"), list.SN);
    root.insert(QStringLiteral("chipGroupRowCount"), list.chipGroupRowCount);
    root.insert(QStringLiteral("chipGroupColCount"), list.chipGroupColCount);
    root.insert(QStringLiteral("dieRowCount"), list.dieRowCount);
    root.insert(QStringLiteral("dieColCount"), list.dieColCount);
    root.insert(QStringLiteral("defectPartitions"), QJsonDocument::fromJson(defectPartitionsToJson(list.defectPartitions).toUtf8()).array());
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

bool defectInfoListFromJson(const QString &json, DefectInfoList *out, QString *errorOut)
{
    if (!out) {
        if (errorOut)
            *errorOut = QStringLiteral("defectInfoListFromJson: null out");
        return false;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (!doc.isObject()) {
        if (errorOut)
            *errorOut = err.errorString();
        return false;
    }

    const QJsonObject root = doc.object();
    DefectInfoList list;
    list.defectInfoFullFileName = root.value(QStringLiteral("defectInfoFullFileName")).toString();
    list.detectTime             = root.value(QStringLiteral("detectTime")).toString();
    list.validDefectType        = static_cast<InspectionDefectTypeMask>(root.value(QStringLiteral("validDefectType")).toVariant().toULongLong());
    list.append                 = root.value(QStringLiteral("append")).toBool();
    list.productName            = root.value(QStringLiteral("productName")).toString();
    list.SN                     = root.value(QStringLiteral("SN")).toString();
    list.chipGroupRowCount      = root.value(QStringLiteral("chipGroupRowCount")).toInt();
    list.chipGroupColCount      = root.value(QStringLiteral("chipGroupColCount")).toInt();
    list.dieRowCount            = root.value(QStringLiteral("dieRowCount")).toInt();
    list.dieColCount            = root.value(QStringLiteral("dieColCount")).toInt();

    const QJsonArray partitions = root.value(QStringLiteral("defectPartitions")).toArray();
    for (const QJsonValue &v : partitions) {
        DefectInfoPartition part;
        if (!partitionFromJson(v.toObject(), &part)) {
            if (errorOut)
                *errorOut = QStringLiteral("invalid defectPartitions in list json");
            return false;
        }
        list.defectPartitions.append(part);
    }

    *out = list;
    return true;
}

} // namespace aoi::inspection
