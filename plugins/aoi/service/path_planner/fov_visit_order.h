#pragma once

/**
 * @file fov_visit_order.h
 * @brief 按 S/Z/逐行/逐列模式对 FOV 网格单元排序
 */

#include "capture_path_types.h"
#include "service/fov_planner/fov_grid_planner.h"

#include <vector>

namespace aoi::path_planner {

/**
 * @brief 根据访问模式返回有序的 FOV 单元指针列表
 *
 * 列表内元素指向 `gridPlan.cells` 中对象，调用方需保证 gridPlan 生命周期。
 */
std::vector<const fov_planner::FovGridCell *>
buildFovVisitSequence(const fov_planner::FovGridPlan &gridPlan, FovVisitPattern pattern);

} // namespace aoi::path_planner
