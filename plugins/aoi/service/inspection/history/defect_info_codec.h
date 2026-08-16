#pragma once

/**
 * @file defect_info_codec.h
 * @brief DefectInfoList JSON 编解码（文件落盘与检测历史库共用）
 */

#include "inspection_types.h"

#include <QString>

namespace aoi::inspection {

/** `defectPartitions` → JSON 数组文本（与 defect_info.json 内分区结构一致） */
QString defectPartitionsToJson(const QVector<DefectInfoPartition> &partitions);
bool    defectPartitionsFromJson(const QString &json, QVector<DefectInfoPartition> *out,
                                 QString *errorOut = nullptr);

/** 完整 `DefectInfoList` ↔ JSON 对象（含分区数组） */
QString defectInfoListToJson(const DefectInfoList &list);
bool    defectInfoListFromJson(const QString &json, DefectInfoList *out, QString *errorOut = nullptr);

} // namespace aoi::inspection
