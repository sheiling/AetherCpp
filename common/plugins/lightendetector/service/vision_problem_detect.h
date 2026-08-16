#pragma once

/**
 * @file vision_problem_detect.h
 * @brief 线性不良（列/行连续缺陷）与区块不良（连通域）检测。
 *
 * 算法逻辑移植自华星 lighten_detect 的 VisionProblemDetect；
 * 由 ResultSummarySteps 在汇总阶段按 specialDetectRules.line_defect / block_defect 调用。
 */

#include <common/include/geometry.h>

#include <vector>

namespace common {
namespace algorithm {

/** 线性不良检测参数 */
struct ColumnProblemDetectParam {
    int led_w = 0;                    ///< 灯板列数（像素宽）
    int led_h = 0;                    ///< 灯板行数（像素高）
    int width_detect_flag = 0;        ///< 列方向连续缺陷判定阈值（同列纵向累加）
    int height_detect_flag = 0;       ///< 行方向连续缺陷判定阈值（同行横向累加）
    std::vector<geometry::Point> points;                          ///< 待检缺陷点（R/G/B 汇总）
    std::vector<std::vector<geometry::Point>> column_problems;    ///< 输出：检出的线性不良段
};

/** 区块不良检测参数 */
struct BlockProblemDetectParam {
    int led_w = 0;                    ///< 灯板列数
    int led_h = 0;                    ///< 灯板行数
    int detect_flag = 0;              ///< 连通域灯点数下限
    std::vector<geometry::Point> points;                       ///< 待检缺陷点（R/G/B 汇总）
    std::vector<std::vector<geometry::Point>> block_problems;  ///< 输出：检出的区块不良
};

/** 检测列/行方向连续缺陷，结果写入 param.column_problems */
void ColumnProblemDetect(ColumnProblemDetectParam& param);

/** 检测连通域区块缺陷，结果写入 param.block_problems */
void BlockProblemDetect(BlockProblemDetectParam& param);

} // namespace algorithm
} // namespace common
