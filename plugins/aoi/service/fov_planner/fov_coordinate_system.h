#pragma once

/**
 * @file fov_coordinate_system.h
 * @brief 灯板逻辑/物理坐标系：双 Marker 仿射 + 组/芯片中心查询
 */

#include "affine_transform_2d.h"
#include "fov_layout_types.h"

#include <optional>
#include <string>

namespace aoi::fov_planner {

/** 组网格 + 组内芯片索引（chipInGroup: 0/1/2 对应组内三颗芯片槽位） */
struct FovChipIndex
{
    int groupRow    = 0; ///< 组行 [0, groupRowCount)
    int groupCol    = 0; ///< 组列 [0, groupColCount)
    int chipInGroup = 0; ///< 组内芯片 [0,2]
};

/**
 * @brief FOV 规划坐标系
 *
 * build() 成功后提供逻辑↔物理变换及组/芯片中心查询。
 */
class FovCoordinateSystem
{
public:
    /** 由 FovLayoutConfig 建立双 Mark 相似仿射 */
    bool build(const FovLayoutConfig &config);

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

    const aoi::geometry::AffineTransform2D &affine() const { return affine_; }
    const FovLayoutConfig &config() const { return config_; }

    /** 逻辑灯板矩形四角（由 BL/TR Mark 推算） */
    aoi::geometry::PanelLogicalCorners logicalCorners() const;

    Vec2d logicalToPhysical(const Vec2d &logical) const;
    Vec2d physicalToLogical(const Vec2d &physical) const;

    Vec2d groupCenterLogical(int groupRow, int groupCol) const;
    Vec2d groupCenterPhysical(int groupRow, int groupCol) const;

    /** 索引越界返回 nullopt */
    std::optional<Vec2d> chipLogicalCenter(int groupRow, int groupCol, int chipInGroup) const;
    std::optional<Vec2d> chipPhysicalCenter(int groupRow, int groupCol, int chipInGroup) const;

    /** 芯片组区域外接矩形（逻辑/物理） */
    Rect2d panelLogicalBounds() const;
    Rect2d panelPhysicalBounds() const;

    /** M1–M2 逻辑矩形四角变换后的物理 XY 外接盒（配方预览轴点动钳位） */
    struct MarkPhysicalAabb
    {
        double minX  = 0.0;
        double maxX  = 0.0;
        double minY  = 0.0;
        double maxY  = 0.0;
        bool   valid = false;
    };
    MarkPhysicalAabb markPhysicalAabb() const;

private:
    /** 组内 chipInGroup 相对组中心的偏移（水平/竖直排列由 chipArrangement 决定） */
    Vec2d chipOffsetInGroup(int chipInGroup) const;

    FovLayoutConfig                  config_;
    aoi::geometry::AffineTransform2D affine_;
    bool                             valid_ = false;
    std::string                      lastError_;
};

} // namespace aoi::fov_planner
