#pragma once

/**
 * @file fov_chip_index_map.h
 * @brief 芯片组 ↔ FOV 网格 ↔ 图像像素 索引映射
 */

#include "fov_coordinate_system.h"
#include "fov_grid_planner.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace aoi::fov_planner {

/** 单颗芯片与所属 FOV、图像像素中心的绑定 */
struct FovChipBinding
{
    int   imageIndex  = 0; ///< 对应 FovGridCell::index
    int   groupRow    = 0;
    int   groupCol    = 0;
    int   chipInGroup = 0;
    int   fovGridRow  = 0;
    int   fovGridCol  = 0;
    Vec2d logicalCenter;
    Vec2d physicalCenter;
    double imageCenterX = 0.0; ///< FOV 图像内像素 X
    double imageCenterY = 0.0; ///< FOV 图像内像素 Y
};

/**
 * @brief 遍历所有芯片组，建立 FOV 索引与像素位置映射
 *
 * 像素：imageCenterX = W/2 + (chipX - fovCenterX) / mmPerPixel
 *      imageCenterY = H/2 - (chipY - fovCenterY) / mmPerPixel
 *
 * fovCells_ 持有 grid 副本，FovGeometryPlan 拷贝后索引仍有效。
 */
class FovChipIndexMap
{
public:
    bool build(const FovCoordinateSystem &coords,
               const FovGridPlan &gridPlan,
               double mmPerPixel,
               int imageWidthPx,
               int imageHeightPx);

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

    const std::vector<FovChipBinding> &bindings() const { return bindings_; }

    std::optional<FovChipBinding> findChip(int groupRow, int groupCol, int chipInGroup) const;
    std::optional<const FovGridCell *> fovByImageIndex(int imageIndex) const;
    std::optional<FovChipBinding> findChipByImage(int imageIndex, double px, double py) const;
    std::optional<int> imageIndexForFovGrid(int gridRow, int gridCol) const;

private:
    /** 查找包含 physical 的 FOV；无包含时返回距中心最近者 */
    const FovGridCell *findFovContaining(const Vec2d &physical) const;
    const FovGridCell *cellAt(size_t index) const;

    std::vector<FovChipBinding> bindings_;
    /** 拷贝 gridPlan.cells，避免 FovGeometryPlan 赋值后悬空指针 */
    std::vector<FovGridCell> fovCells_;
    /** imageIndex → fovCells_ 下标；无效为 size_t(-1) */
    std::vector<size_t> fovByIndex_;
    /** gridKey(row,col) → fovCells_ 下标 */
    std::unordered_map<int, size_t> fovByGrid_;
    bool                                         valid_ = false;
    std::string                                  lastError_;
    double                                       mmPerPixel_ = 1.0;
    int                                          imageWidthPx_ = 0;
    int                                          imageHeightPx_ = 0;
};

} // namespace aoi::fov_planner
