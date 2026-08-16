#pragma once

/**
 * @file pure_color_detect_algorithm.h
 * @brief 纯色检测算法声明。
 *
 * 对指定颜色通道图像执行 Shot + Meter，检测死灯、暗灯、亮灯及串色缺陷。
 * 输入：PureColorDetectParams；输出：deads / darks / brights / color_variations。
 * 实现见 pure_color_detect_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_base.h>
#include <common/include/algorithm_data/lightendetector_data.h>

namespace common {
namespace algorithm {

// 纯色检测算法：对指定颜色通道图像执行 Shot + Meter，检测死灯、暗灯、亮灯及串色缺陷。
// 输入：PureColorDetectParams（image 等）
// 输出：deads / darks / brights（三类缺陷点列表，由 dead_check / dark_check / bright_check 控制）
//       color_variations（串色缺陷点列表，由 color_check 控制）
class PureColorDetectAlgorithm final : public AlgorithmBase {
public:
    std::string Name() const override;
    std::string Version() const override;

    common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) override;
};

} // namespace algorithm
} // namespace common
