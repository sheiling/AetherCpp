#include "vision_problem_detect.h"

#include <queue>
#include <vector>

/**
 * @file vision_problem_detect.cpp
 * @brief ColumnProblemDetect / BlockProblemDetect 实现（移植自华星 VisionProblemDetect）。
 */

namespace common {
namespace algorithm {
namespace {

void FloodFillBlock(
    int start_x,
    int start_y,
    int led_w,
    int led_h,
    std::vector<int>& grid,
    std::vector<geometry::Point>& out)
{
    std::queue<int> q_x;
    std::queue<int> q_y;
    q_x.push(start_x);
    q_y.push(start_y);

    while (!q_x.empty()) {
        const int x = q_x.front();
        const int y = q_y.front();
        out.push_back({x, y});
        grid[static_cast<size_t>(x * led_h + y)] = 0;

        q_x.pop();
        q_y.pop();

        if (x < led_w && grid[static_cast<size_t>((x + 1) * led_h + y)] == 1) {
            grid[static_cast<size_t>((x + 1) * led_h + y)] = 0;
            q_x.push(x + 1);
            q_y.push(y);
        }
        if (x > 0 && grid[static_cast<size_t>((x - 1) * led_h + y)] == 1) {
            grid[static_cast<size_t>((x - 1) * led_h + y)] = 0;
            q_x.push(x - 1);
            q_y.push(y);
        }
        if (y < led_h && grid[static_cast<size_t>(x * led_h + y + 1)] == 1) {
            grid[static_cast<size_t>(x * led_h + y + 1)] = 0;
            q_x.push(x);
            q_y.push(y + 1);
        }
        if (y > 0 && grid[static_cast<size_t>(x * led_h + y - 1)] == 1) {
            grid[static_cast<size_t>(x * led_h + y - 1)] = 0;
            q_x.push(x);
            q_y.push(y - 1);
        }
    }
}

void MarkDefectGrid(
    const std::vector<geometry::Point>& points,
    int led_w,
    int led_h,
    std::vector<int>& grid)
{
    grid.assign(static_cast<size_t>(led_w * led_h), 0);
    for (const auto& pt : points) {
        if (pt.x >= 0 && pt.x < led_w && pt.y >= 0 && pt.y < led_h) {
            grid[static_cast<size_t>(pt.x * led_h + pt.y)] = 1;
        }
    }
}

} // namespace

void ColumnProblemDetect(ColumnProblemDetectParam& param)
{
    param.column_problems.clear();

    const int width  = param.led_w;
    const int height = param.led_h;
    if (width <= 0 || height <= 0 || param.points.empty()) {
        return;
    }

    std::vector<int> grid;
    MarkDefectGrid(param.points, width, height, grid);

    // 列方向（纵向）连续缺陷
    for (int i = 0; i < width * height; i += height) {
        for (int j = i + 1; j < i + height; ++j) {
            if (grid[static_cast<size_t>(j)] == 0) {
                continue;
            }
            grid[static_cast<size_t>(j)] += grid[static_cast<size_t>(j - 1)];
            if (grid[static_cast<size_t>(j)] >= param.width_detect_flag
                && ((j + 1 >= i + height) || (grid[static_cast<size_t>(j + 1)] != 1))) {
                const int x_begin = i / height;
                const int y_begin = (j - grid[static_cast<size_t>(j)] + 1) % height;
                const int y_end   = j % height;

                std::vector<geometry::Point> segment;
                for (int k = y_begin; k <= y_end; ++k) {
                    segment.push_back({x_begin, k});
                }
                param.column_problems.push_back(std::move(segment));
            }
        }
    }

    MarkDefectGrid(param.points, width, height, grid);

    // 行方向（横向）连续缺陷
    for (int i = 0; i < height; ++i) {
        for (int j = i + height; j < width * height; j += height) {
            if (grid[static_cast<size_t>(j)] == 0) {
                continue;
            }
            grid[static_cast<size_t>(j)] += grid[static_cast<size_t>(j - height)];
            if (grid[static_cast<size_t>(j)] >= param.height_detect_flag
                && ((j + height >= width * height) || (grid[static_cast<size_t>(j + height)] != 1))) {
                const int x_begin = (j - grid[static_cast<size_t>(j)] * height + height) / height;
                const int x_end   = j / height;

                std::vector<geometry::Point> segment;
                for (int k = x_begin; k <= x_end; ++k) {
                    segment.push_back({k, i});
                }
                param.column_problems.push_back(std::move(segment));
            }
        }
    }
}

void BlockProblemDetect(BlockProblemDetectParam& param)
{
    param.block_problems.clear();

    const int led_w = param.led_w;
    const int led_h = param.led_h;
    if (led_w <= 0 || led_h <= 0 || param.points.empty()) {
        return;
    }

    std::vector<int> grid;
    MarkDefectGrid(param.points, led_w, led_h, grid);

    for (int i = 0; i < led_w; ++i) {
        for (int j = 0; j < led_h; ++j) {
            if (grid[static_cast<size_t>(i * led_h + j)] != 1) {
                continue;
            }
            std::vector<geometry::Point> block;
            block.reserve(static_cast<size_t>(led_w * led_h));
            FloodFillBlock(i, j, led_w, led_h, grid, block);
            if (static_cast<int>(block.size()) >= param.detect_flag) {
                param.block_problems.push_back(std::move(block));
            }
        }
    }
}

} // namespace algorithm
} // namespace common
