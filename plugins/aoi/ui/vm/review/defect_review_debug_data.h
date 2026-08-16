#pragma once

/**
 * @file defect_review_debug_data.h
 * @brief 缺陷复判联调用 `DefectInfoList` 构造（bridge 调试入口唯一数据源）
 *
 * 由 `HomeViewModel::triggerDirectReviewFlow()` 经 `ScanFlowProgressBridge` 投递；
 * 不在正式 `buildDefectInfoList` 或复判 VM 内嵌 mock 分支。
 * 设计见 `plugins/aoi/docs/AOI 检测软件 设计方案.md` §3.1.4。
 */

#include "inspection_types.h"

namespace aoi::review {

/**
 * @brief 构造贴近正式板级结构的调试 `DefectInfoList`
 * @param verticalArrangement true=Vertical（组内沿行展开），false=Horizontal（组内沿列展开）
 */
inspection::DefectInfoList buildReviewDebugDefectInfoList(bool verticalArrangement = true);

} // namespace aoi::review
