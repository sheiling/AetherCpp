#pragma once

/**
 * @file fov_geometry_planner.h
 * @brief FOV 几何规划聚合结果与三层协调规划器（不含采集路径；路径见 service/path_planner）
 */

#include "fov_chip_index_map.h"
#include "fov_coordinate_system.h"
#include "fov_grid_planner.h"
#include "fov_layout_types.h"

#include <string>

namespace aoi::fov_planner {

/**
 * @brief 完整 FOV 几何规划输出
 *
 * | 成员 | 说明 |
 * |------|------|
 * | coords | 仿射坐标系 + 芯片物理位置查询 |
 * | grid | FOV 网格划分（cells / rowCount / colCount） |
 * | chipMap | 芯片 → FOV 索引 / 网格行列 / 图像像素 |
 */
struct FovGeometryPlan
{
    FovCoordinateSystem coords;
    FovGridPlan         grid;
    FovChipIndexMap     chipMap;

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

private:
    friend class FovGeometryPlanner;
    bool        valid_ = false;
    std::string lastError_;
};

/**
 * @brief FOV 几何规划器
 */
class FovGeometryPlanner
{
public:
    /**
     * @param config         布局配置（配方 + Marker 锚点）
     * @param mmPerPixel     相机采集精度 (mm/px)
     * @param imageWidthPx   图像宽度 (px)
     * @param imageHeightPx  图像高度 (px)
     */
    FovGeometryPlan plan(const FovLayoutConfig &config,
                         double mmPerPixel,
                         int imageWidthPx,
                         int imageHeightPx) const;
};

} // namespace aoi::fov_planner
