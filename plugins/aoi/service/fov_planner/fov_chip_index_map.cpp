/**
 * @file fov_chip_index_map.cpp
 * @brief FovChipIndexMap：芯片 ↔ FOV 网格 ↔ 图像像素映射
 */

#include "fov_chip_index_map.h"

#include <cmath>
#include <limits>

namespace aoi::fov_planner {

namespace {

constexpr size_t kInvalidFovCellIndex = static_cast<size_t>(-1);

/** gridRow/col 编码为哈希键（假定 col < 10000） */
int gridKey(int row, int col)
{
    return row * 10000 + col;
}

} // namespace

const FovGridCell *FovChipIndexMap::cellAt(size_t index) const
{
    return index < fovCells_.size() ? &fovCells_[index] : nullptr;
}

bool FovChipIndexMap::build(const FovCoordinateSystem &coords,
                            const FovGridPlan &gridPlan,
                            const double mmPerPixel,
                            const int imageWidthPx,
                            const int imageHeightPx)
{
    bindings_.clear();
    fovCells_.clear();
    fovByIndex_.clear();
    fovByGrid_.clear();
    valid_      = false;
    lastError_.clear();

    if (!coords.isValid() || gridPlan.cells.empty()) {
        lastError_ = "invalid coords or fov grid plan";
        return false;
    }
    if (mmPerPixel <= 0.0 || imageWidthPx <= 0 || imageHeightPx <= 0) {
        lastError_ = "invalid image scale";
        return false;
    }

    mmPerPixel_    = mmPerPixel;
    imageWidthPx_  = imageWidthPx;
    imageHeightPx_ = imageHeightPx;

    fovCells_ = gridPlan.cells;

    int maxIndex = 0;
    for (size_t i = 0; i < fovCells_.size(); ++i) {
        const FovGridCell &cell = fovCells_[i];
        fovByGrid_[gridKey(cell.gridRow, cell.gridCol)] = i;
        maxIndex = std::max(maxIndex, cell.index);
    }
    fovByIndex_.assign(static_cast<size_t>(maxIndex + 1), kInvalidFovCellIndex);
    for (size_t i = 0; i < fovCells_.size(); ++i) {
        const FovGridCell &cell = fovCells_[i];
        if (cell.index >= 0 && cell.index < static_cast<int>(fovByIndex_.size()))
            fovByIndex_[static_cast<size_t>(cell.index)] = i;
    }

    const int groupRows = coords.config().groupRowCount;
    const int groupCols = coords.config().groupColCount;
    if (groupRows <= 0 || groupCols <= 0) {
        lastError_ = "invalid group grid size";
        return false;
    }

    bindings_.reserve(static_cast<size_t>(groupRows) * static_cast<size_t>(groupCols) * 3U);

    for (int gr = 0; gr < groupRows; ++gr) {
        for (int gc = 0; gc < groupCols; ++gc) {
            for (int ig = 0; ig < 3; ++ig) {
                const auto logical  = coords.chipLogicalCenter(gr, gc, ig);
                const auto physical = coords.chipPhysicalCenter(gr, gc, ig);
                if (!logical || !physical)
                    continue;

                const FovGridCell *fov = findFovContaining(*physical);
                if (!fov)
                    continue;

                FovChipBinding binding;
                binding.imageIndex     = fov->index;
                binding.groupRow       = gr;
                binding.groupCol       = gc;
                binding.chipInGroup    = ig;
                binding.fovGridRow     = fov->gridRow;
                binding.fovGridCol     = fov->gridCol;
                binding.logicalCenter  = *logical;
                binding.physicalCenter = *physical;

                const double dxMm = physical->x - fov->centerPhysical.x;
                const double dyMm = physical->y - fov->centerPhysical.y;
                binding.imageCenterX = imageWidthPx * 0.5 + dxMm / mmPerPixel_;
                binding.imageCenterY = imageHeightPx * 0.5 - dyMm / mmPerPixel_;

                bindings_.push_back(binding);
            }
        }
    }

    valid_ = !bindings_.empty();
    if (!valid_)
        lastError_ = "no chip bindings generated";
    return valid_;
}

std::optional<FovChipBinding> FovChipIndexMap::findChip(int groupRow, int groupCol,
                                                          int chipInGroup) const
{
    for (const FovChipBinding &b : bindings_) {
        if (b.groupRow == groupRow && b.groupCol == groupCol && b.chipInGroup == chipInGroup)
            return b;
    }
    return std::nullopt;
}

std::optional<const FovGridCell *> FovChipIndexMap::fovByImageIndex(int imageIndex) const
{
    if (imageIndex < 0 || imageIndex >= static_cast<int>(fovByIndex_.size()))
        return std::nullopt;
    const FovGridCell *cell = cellAt(fovByIndex_[static_cast<size_t>(imageIndex)]);
    if (!cell)
        return std::nullopt;
    return cell;
}

std::optional<FovChipBinding> FovChipIndexMap::findChipByImage(int imageIndex, double px,
                                                               double py) const
{
    if (!fovByImageIndex(imageIndex))
        return std::nullopt;

    double bestDist = std::numeric_limits<double>::max();
    std::optional<FovChipBinding> best;
    for (const FovChipBinding &b : bindings_) {
        if (b.imageIndex != imageIndex)
            continue;
        const double dx = b.imageCenterX - px;
        const double dy = b.imageCenterY - py;
        const double d2 = dx * dx + dy * dy;
        if (d2 < bestDist) {
            bestDist = d2;
            best     = b;
        }
    }
    return best;
}

std::optional<int> FovChipIndexMap::imageIndexForFovGrid(int gridRow, int gridCol) const
{
    const auto it = fovByGrid_.find(gridKey(gridRow, gridCol));
    if (it == fovByGrid_.end())
        return std::nullopt;
    const FovGridCell *cell = cellAt(it->second);
    if (!cell)
        return std::nullopt;
    return cell->index;
}

const FovGridCell *FovChipIndexMap::findFovContaining(const Vec2d &physical) const
{
    const FovGridCell *best     = nullptr;
    double             bestDist = std::numeric_limits<double>::max();
    for (const FovGridCell &cell : fovCells_) {
        if (cell.boundsPhysical.contains(physical))
            return &cell;
        const double dx = physical.x - cell.centerPhysical.x;
        const double dy = physical.y - cell.centerPhysical.y;
        const double d2 = dx * dx + dy * dy;
        if (d2 < bestDist) {
            bestDist = d2;
            best     = &cell;
        }
    }
    return best;
}

} // namespace aoi::fov_planner
