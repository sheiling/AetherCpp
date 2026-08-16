#pragma once

/**
 * @file black_screen_light_detect_algorithm.h
 * @brief 黑屏亮灯检测算法声明。
 *
 * 在黑场背景下检测异常亮灯点位。
 * 输入：BlackScreenLightDetectParams；输出：black_screen_lights。
 * 实现见 black_screen_light_detect_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_base.h>
#include <common/include/algorithm_data/lightendetector_data.h>

namespace common {
namespace algorithm {

// 黑屏亮灯检测算法：在黑场背景下检测异常亮灯点位。
// 输入：BlackScreenLightDetectParams（bg_image 等）
// 输出：black_screen_lights（异常亮灯缺陷点列表）
class BlackScreenLightDetectAlgorithm final : public AlgorithmBase {
public:
    std::string Name() const override;
    std::string Version() const override;
    common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) override;
};

} // namespace algorithm
} // namespace common