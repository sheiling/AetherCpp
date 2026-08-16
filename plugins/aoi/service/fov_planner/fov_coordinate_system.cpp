/**
 * @file fov_coordinate_system.cpp
 * @brief FovCoordinateSystem 实现
 */

#include "fov_coordinate_system.h"

#include "affine_transform_2d.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aoi::fov_planner {

bool FovCoordinateSystem::build(const FovLayoutConfig &config)
{
    config_ = config;
    valid_  = false;
    lastError_.clear();

    if (config.groupRowCount <= 0 || config.groupColCount <= 0) {
        lastError_ = "invalid group grid size";
        return false;
    }
    if (config.fovWidthMm <= 0.0 || config.fovHeightMm <= 0.0) {
        lastError_ = "invalid fov size";
        return false;
    }

    affine_ = aoi::geometry::AffineTransform2D::fromTwoPointSimilarity(
        config.logicalMarkerBottomLeft,
        config.logicalMarkerTopRight,
        config.physicalMarkerBottomLeft,
        config.physicalMarkerTopRight);
    if (!affine_.isValid()) {
        lastError_ = affine_.lastError();
        return false;
    }

    valid_ = true;
    return true;
}

aoi::geometry::PanelLogicalCorners FovCoordinateSystem::logicalCorners() const
{
    return aoi::geometry::PanelLogicalCorners::fromBottomLeftAndTopRight(
        config_.logicalMarkerBottomLeft, config_.logicalMarkerTopRight);
}

Vec2d FovCoordinateSystem::logicalToPhysical(const Vec2d &logical) const
{
    return affine_.transform(logical);
}

Vec2d FovCoordinateSystem::physicalToLogical(const Vec2d &physical) const
{
    return affine_.inverseTransform(physical);
}

Vec2d FovCoordinateSystem::groupCenterLogical(int groupRow, int groupCol) const
{
    const Vec2d group00 = config_.logicalMarkerBottomLeft - config_.offsetGroupMarkBl;
    return {group00.x + groupCol * config_.groupPitchXmm,
            group00.y + groupRow * config_.groupPitchYmm};
}

Vec2d FovCoordinateSystem::groupCenterPhysical(int groupRow, int groupCol) const
{
    return logicalToPhysical(groupCenterLogical(groupRow, groupCol));
}

Vec2d FovCoordinateSystem::chipOffsetInGroup(int chipInGroup) const
{
    const int    slot  = std::clamp(chipInGroup, 0, 2);
    const double pitch = config_.chipPitchInGroupMm;
    static constexpr double kOffsets[3] = {-1.0, 0.0, 1.0};
    if (config_.chipArrangement == FovChipArrangement::Horizontal)
        return {kOffsets[slot] * pitch, 0.0};
    return {0.0, kOffsets[slot] * pitch};
}

std::optional<Vec2d> FovCoordinateSystem::chipLogicalCenter(int groupRow, int groupCol,
                                                            int chipInGroup) const
{
    if (groupRow < 0 || groupCol < 0 || groupRow >= config_.groupRowCount
        || groupCol >= config_.groupColCount || chipInGroup < 0 || chipInGroup > 2)
        return std::nullopt;
    return groupCenterLogical(groupRow, groupCol) + chipOffsetInGroup(chipInGroup);
}

std::optional<Vec2d> FovCoordinateSystem::chipPhysicalCenter(int groupRow, int groupCol,
                                                               int chipInGroup) const
{
    const auto logical = chipLogicalCenter(groupRow, groupCol, chipInGroup);
    if (!logical)
        return std::nullopt;
    return logicalToPhysical(*logical);
}

Rect2d FovCoordinateSystem::panelLogicalBounds() const
{
    return computeChipRegionLogicalBounds(config_);
}

Rect2d FovCoordinateSystem::panelPhysicalBounds() const
{
    // 逻辑矩形四角分别变换后取物理 XY 外接盒（仿射下非轴对齐）
    const Rect2d logical = computeChipRegionLogicalBounds(config_);
    const Vec2d  pBL     = logicalToPhysical({logical.left(), logical.bottom()});
    const Vec2d  pBR     = logicalToPhysical({logical.right(), logical.bottom()});
    const Vec2d  pTL     = logicalToPhysical({logical.left(), logical.top()});
    const Vec2d  pTR     = logicalToPhysical({logical.right(), logical.top()});

    const double minX = std::min({pBL.x, pBR.x, pTL.x, pTR.x});
    const double maxX = std::max({pBL.x, pBR.x, pTL.x, pTR.x});
    const double minY = std::min({pBL.y, pBR.y, pTL.y, pTR.y});
    const double maxY = std::max({pBL.y, pBR.y, pTL.y, pTR.y});
    return {minX, minY, maxX - minX, maxY - minY};
}

FovCoordinateSystem::MarkPhysicalAabb FovCoordinateSystem::markPhysicalAabb() const
{
    MarkPhysicalAabb aabb;
    if (!valid_)
        return aabb;

    const Rect2d mark = computeMarkLogicalBounds(config_);
    const Vec2d  corners[4] = {{mark.left(), mark.bottom()},
                               {mark.right(), mark.bottom()},
                               {mark.left(), mark.top()},
                               {mark.right(), mark.top()}};

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const Vec2d &logical : corners) {
        const Vec2d physical = logicalToPhysical(logical);
        minX                 = std::min(minX, physical.x);
        maxX                 = std::max(maxX, physical.x);
        minY                 = std::min(minY, physical.y);
        maxY                 = std::max(maxY, physical.y);
    }

    aabb.minX  = minX;
    aabb.maxX  = maxX;
    aabb.minY  = minY;
    aabb.maxY  = maxY;
    aabb.valid = minX <= maxX && minY <= maxY;
    return aabb;
}

} // namespace aoi::fov_planner
