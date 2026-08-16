#pragma once

/**
 * @file defect_info_persistence.h
 * @brief DefectInfoList JSON 落盘（复判确认）
 */

#include "inspection_types.h"

#include <QString>

namespace aoi::inspection {

/** 将板级缺陷列表写入 `list.defectInfoFullFileName`；路径为空则失败 */
bool saveDefectInfoList(const DefectInfoList &list, QString *errorOut = nullptr);

} // namespace aoi::inspection
