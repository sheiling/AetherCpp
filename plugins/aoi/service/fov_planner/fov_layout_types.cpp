/**
 * @file fov_layout_types.cpp
 * @brief FovLayoutConfig 相关几何计算
 */

#include "fov_layout_types.h"

#include <algorithm>
#include <limits>

namespace aoi::fov_planner {

Vec2d computeGroupCenterLogical(const FovLayoutConfig &config, int groupRow, int groupCol)
{
    // (0,0) 组中心 = BL Mark - offsetGroupMarkBl
    const Vec2d group00 = config.logicalMarkerBottomLeft - config.offsetGroupMarkBl;
    return {group00.x + groupCol * config.groupPitchXmm,
            group00.y + groupRow * config.groupPitchYmm};
}

Vec2d computeLogicalMarkerTopRight(const FovLayoutConfig &config)
{
    const int lastRow = std::max(config.groupRowCount - 1, 0);
    const int lastCol = std::max(config.groupColCount - 1, 0);
    const Vec2d groupLast = computeGroupCenterLogical(config, lastRow, lastCol);
    return groupLast + config.offsetGroupMarkTr;
}

Rect2d computeChipRegionLogicalBounds(const FovLayoutConfig &config)
{
    if (config.groupRowCount <= 0 || config.groupColCount <= 0)
        return {};

    const int lastRow = config.groupRowCount - 1;
    const int lastCol = config.groupColCount - 1;
    const double halfW = config.groupWidthMm * 0.5;
    const double halfH = config.groupHeightMm * 0.5;

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    // 四角组中心 ± 半组宽/高 取并集
    const int corners[4][2] = {{0, 0}, {0, lastCol}, {lastRow, 0}, {lastRow, lastCol}};
    for (const auto &corner : corners) {
        const Vec2d center = computeGroupCenterLogical(config, corner[0], corner[1]);
        minX               = std::min(minX, center.x - halfW);
        maxX               = std::max(maxX, center.x + halfW);
        minY               = std::min(minY, center.y - halfH);
        maxY               = std::max(maxY, center.y + halfH);
    }

    return {minX, minY, maxX - minX, maxY - minY};
}

Rect2d computeMarkLogicalBounds(const FovLayoutConfig &config)
{
    const double left   = std::min(config.logicalMarkerBottomLeft.x, config.logicalMarkerTopRight.x);
    const double right  = std::max(config.logicalMarkerBottomLeft.x, config.logicalMarkerTopRight.x);
    const double bottom = std::min(config.logicalMarkerBottomLeft.y, config.logicalMarkerTopRight.y);
    const double top    = std::max(config.logicalMarkerBottomLeft.y, config.logicalMarkerTopRight.y);
    return {left, bottom, right - left, top - bottom};
}

Rect2d computeMarkNavigateLogicalBounds(const FovLayoutConfig &config)
{
    const Rect2d marks = computeMarkLogicalBounds(config);
    const double padX  = std::max(config.groupPitchXmm * 3.0, 0.0);
    const double padY  = std::max(config.groupPitchYmm * 3.0, 0.0);
    return {marks.left() - padX, marks.bottom() - padY, marks.width + padX * 2.0,
            marks.height + padY * 2.0};
}

namespace {

Rect2d expandToInclude(const Rect2d &rect, const Vec2d &point, double marginMm)
{
    if (rect.width <= 0.0 || rect.height <= 0.0) {
        return {point.x - marginMm, point.y - marginMm, marginMm * 2.0, marginMm * 2.0};
    }

    const double left   = std::min(rect.left(), point.x - marginMm);
    const double bottom = std::min(rect.bottom(), point.y - marginMm);
    const double right  = std::max(rect.right(), point.x + marginMm);
    const double top    = std::max(rect.top(), point.y + marginMm);
    return {left, bottom, right - left, top - bottom};
}

} // namespace

Rect2d computeNavigatorViewBounds(const FovLayoutConfig &config)
{
    Rect2d bounds = computeChipRegionLogicalBounds(config);
    bounds        = expandToInclude(bounds, config.logicalMarkerBottomLeft, 6.0);
    bounds        = expandToInclude(bounds, config.logicalMarkerTopRight, 6.0);
    return bounds;
}

} // namespace aoi::fov_planner
