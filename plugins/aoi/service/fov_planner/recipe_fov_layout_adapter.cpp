/**
 * @file recipe_fov_layout_adapter.cpp
 * @brief 配方 + Marker 锚点 → FovLayoutConfig
 */

#include "recipe_fov_layout_adapter.h"

#include "param_define.h"

namespace aoi::fov_planner {

namespace {

/** 读取配方 marks[slot].capturePos（标定示教坐标） */
aoi::Point markerCapturePos(const aoi::RecipeDocument &doc, int slot)
{
    const aoi::MarkPointConfig &mark = aoi::markAt(doc, slot);
    return {mark.capturePos.x, mark.capturePos.y};
}

/**
 * @brief 解析物理 Mark 坐标：优先 Session 定位结果，否则回退 capturePos
 *
 * slot 0 → BL，slot 1 → TR；topRight 参数保留扩展用。
 */
Vec2d resolveMarkerPhysical(const MarkerAnchorInput &markers,
                            const aoi::RecipeDocument &doc,
                            int slot,
                            bool topRight)
{
    Q_UNUSED(topRight);
    if (slot == 0 && markers.bottomLeftValid)
        return toVec2d(markers.bottomLeft);
    if (slot == 1 && markers.topRightValid)
        return toVec2d(markers.topRight);
    const aoi::Point p = markerCapturePos(doc, slot);
    return {p.x, p.y};
}

} // namespace

double captureMmPerPixel(const aoi::RecipeDocument &recipe)
{
    const double umPerPixel =
        recipe.camera.captureAccuracy > 0.0 ? recipe.camera.captureAccuracy : 1.0;
    return umPerPixel / 1000.0;
}

FovSizeMm computeFovSizeMm(const aoi::RecipeDocument &recipe)
{
    const double mmPerPixel = captureMmPerPixel(recipe);
    const int imageWidth  = recipe.camera.imageWidth > 0 ? recipe.camera.imageWidth : 1920;
    const int imageHeight = recipe.camera.imageHeight > 0 ? recipe.camera.imageHeight : 1080;
    return {static_cast<double>(imageWidth) * mmPerPixel,
            static_cast<double>(imageHeight) * mmPerPixel};
}

MarkerAnchorInput markerAnchorFromSession(const aoi::Point locatedPhysical[4],
                                          const bool locatedValid[4])
{
    MarkerAnchorInput input;
    if (locatedValid[0]) {
        input.bottomLeft      = locatedPhysical[0];
        input.bottomLeftValid = true;
    }
    if (locatedValid[1]) {
        input.topRight      = locatedPhysical[1];
        input.topRightValid = true;
    }
    return input;
}

FovLayoutConfig buildFovLayoutConfig(const aoi::RecipeDocument &recipe,
                                     const MarkerAnchorInput &markers)
{
    const aoi::RecipeBasicInfo &basic = recipe.basic;

    FovLayoutConfig cfg;
    cfg.logicalMarkerBottomLeft = {0.0, 0.0};

    cfg.groupRowCount      = basic.width;
    cfg.groupColCount      = basic.height;
    cfg.chipWidthMm        = basic.chipWidthMm;
    cfg.chipHeightMm       = basic.chipHeightMm;
    cfg.groupWidthMm       = basic.groupWidthMm;
    cfg.groupHeightMm      = basic.groupHeightMm;
    cfg.chipColorOrder     = static_cast<FovChipColorOrder>(basic.chipColorOrder);
    cfg.chipArrangement    = static_cast<FovChipArrangement>(basic.chipArrangement);
    cfg.groupPitchXmm      = basic.groupPitchXmm;
    cfg.groupPitchYmm      = basic.groupPitchYmm;
    cfg.chipPitchInGroupMm = basic.chipPitchInGroupMm;
    cfg.offsetGroupMarkBl  = toVec2d(basic.offsetGroupMarkBl);
    cfg.offsetGroupMarkTr  = toVec2d(basic.offsetGroupMarkTr);
    cfg.logicalMarkerTopRight = computeLogicalMarkerTopRight(cfg);

    cfg.physicalMarkerBottomLeft = resolveMarkerPhysical(markers, recipe, 0, false);
    cfg.physicalMarkerTopRight   = resolveMarkerPhysical(markers, recipe, 1, true);

    const FovSizeMm fovSize = computeFovSizeMm(recipe);
    cfg.fovWidthMm          = fovSize.widthMm;
    cfg.fovHeightMm         = fovSize.heightMm;

    return cfg;
}

} // namespace aoi::fov_planner
