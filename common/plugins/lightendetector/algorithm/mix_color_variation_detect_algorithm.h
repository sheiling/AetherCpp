#pragma once

/**
 * @file mix_color_variation_detect_algorithm.h
 * @brief 混色检测算法声明。
 *
 * 基于白场参考图与纯色待测图执行 Shot + Meter，联合 RGB 三通道计量检测混色缺陷。
 * 输入：MixColorVariationDetectParams；输出：mix_colors。
 * 实现见 mix_color_variation_detect_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_base.h>
#include <common/include/algorithm_data/lightendetector_data.h>

namespace common {
namespace algorithm {

// 混色检测算法：基于白场参考图与纯色待测图执行 Shot + Meter，联合 RGB 三通道计量检测混色缺陷。
// 输入：MixColorVariationDetectParams（image / bg_image / white_*_contour_file_name 等）
// 输出：mix_colors（混色缺陷点列表）
class MixColorVariationDetectAlgorithm final : public AlgorithmBase {
public:
    std::string Name() const override;
    std::string Version() const override;
    common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) override;
};

} // namespace algorithm
} // namespace common
