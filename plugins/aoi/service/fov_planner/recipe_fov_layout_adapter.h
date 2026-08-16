#pragma once

/**
 * @file recipe_fov_layout_adapter.h
 * @brief 配方 + Marker 锚点 → FovLayoutConfig
 */

#include "data/dataModel/recipe_config_model.h"
#include "fov_layout_types.h"

namespace aoi::fov_planner {

/** aoi::Point → Vec2d 便捷转换 */
inline Vec2d toVec2d(const aoi::Point &p)
{
    return {p.x, p.y};
}

/**
 * @brief Marker 物理锚点输入
 *
 * 检测工步来自 Session markerLocatedPhysical；
 * 标定/预览页可省略（全 false 则回退 marks[].capturePos）。
 */
struct MarkerAnchorInput
{
    aoi::Point bottomLeft;       ///< 槽位 0 物理坐标
    aoi::Point topRight;         ///< 槽位 1 物理坐标
    bool       bottomLeftValid = false;
    bool       topRightValid   = false;
};

/** 从配方与可选 Marker 锚点构建 FovLayoutConfig */
FovLayoutConfig buildFovLayoutConfig(const aoi::RecipeDocument &recipe,
                                     const MarkerAnchorInput &markers = {});

/** 从 InspectionSession 四槽定位数组提取 BL/TR 锚点 */
MarkerAnchorInput markerAnchorFromSession(const aoi::Point locatedPhysical[4],
                                          const bool locatedValid[4]);

/** 单 FOV 物理尺寸 (mm)：imageSize(px) × captureAccuracy(μm/px) / 1000 */
struct FovSizeMm
{
    double widthMm  = 0.0;
    double heightMm = 0.0;
};

/** 配方采集精度 (μm/px) → mm/px */
double captureMmPerPixel(const aoi::RecipeDocument &recipe);

FovSizeMm computeFovSizeMm(const aoi::RecipeDocument &recipe);

} // namespace aoi::fov_planner
