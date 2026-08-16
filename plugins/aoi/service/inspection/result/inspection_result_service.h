#pragma once

/**
 * @file inspection_result_service.h
 * @brief 板级缺陷汇总、阈值判定、UI 快照上报
 */

#include "inspection_session.h"

#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

namespace aoi::inspection {

PanelInspectionResult aggregatePanelResult(const InspectionSession &session);

/**
 * @brief AdHoc 单张图虚拟调测 → DefectInfoList（与产线 buildDefectInfoList 字段语义一致）
 *
 * @param fov               单次 detectFov 结果
 * @param recipe            当前配方
 * @param defects           系统缺陷配置（有效类型掩码）
 * @param originImagePath   原图路径（复判裁剪用）
 * @param detectImagePath   检测结果图路径（可与原图相同）
 * @param serialNumber      板级 SN；空则自动生成 VI- 前缀
 */
DefectInfoList buildAdHocDefectInfoList(const FovDetectRecord &fov, const RecipeDocument &recipe,
                                        const aoi::DefectSettings &defects,
                                        const QString &originImagePath,
                                        const QString &detectImagePath,
                                        const QString &serialNumber = {});

/** 从 DefectInfoList 构建 CurrentInspectionSnapshot（与产线 aggregatePanelResult 快照语义一致） */
aoi::CurrentInspectionSnapshot buildSnapshotFromDefectInfoList(const DefectInfoList &list,
                                                               const aoi::DefectSettings &defects);

scheduler::StepResult runAggregateResultsService(const std::shared_ptr<InspectionSession> &session,
                                                 scheduler::FlowContext &ctx);

} // namespace aoi::inspection
