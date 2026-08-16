#pragma once

/**
 * @file fov_plan_service.h
 * @brief FOV 规划对外入口：配方 / Session → FovGeometryPlan
 */

#include "data/dataModel/recipe_config_model.h"
#include "fov_geometry_planner.h"
#include "recipe_fov_layout_adapter.h"

#include <QString>

namespace aoi::inspection {
struct InspectionSession;
} // namespace aoi::inspection

namespace aoi::fov_planner {

/** FovPlanService::generate 返回值 */
struct FovPlanGenerateResult
{
    bool            ok = false; ///< 规划是否成功
    QString         message;    ///< 失败原因或 formatSummary 文案
    FovGeometryPlan plan;       ///< 几何规划结果（ok 时有效）
};

/**
 * @brief FOV 规划对外服务
 *
 * | 方法 | 场景 |
 * |------|------|
 * | generate(session) | 检测工步：Session markerLocatedPhysical 作仿射锚点 |
 * | generate(recipe) | 配方预览：marks.capturePos 作锚点 |
 */
class FovPlanService
{
public:
    static FovPlanGenerateResult generate(const aoi::RecipeDocument &recipe,
                                          const MarkerAnchorInput &markers = {});

    static FovPlanGenerateResult generate(const aoi::inspection::InspectionSession &session);

    /** 生成 UI 可展示的 FOV 行列/张数摘要 */
    static QString formatSummary(const FovGeometryPlan &plan);
};

} // namespace aoi::fov_planner
