#pragma once

/**
 * @file string_light_detect_algorithm.h
 * @brief 串灯检测算法声明。
 *
 * 对左右两张图像执行 Shot + Meter，沿斜向比对检测 map_live / map_dead 串灯缺陷。
 * 输入：StringLightDetectParams；输出：string_lights。
 * 实现见 string_light_detect_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_base.h>
#include <common/include/algorithm_data/lightendetector_data.h>

namespace common {
namespace algorithm {

// 串灯检测算法：对左右两张图像执行 Shot + Meter，沿斜向比对检测 map_live / map_dead 串灯缺陷。
// 输入：StringLightDetectParams（left_image / right_image 等）
// 输出：string_lights（串灯缺陷点列表）
class StringLightDetectAlgorithm final : public AlgorithmBase {
public:
    std::string Name() const override;
    std::string Version() const override;
    common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) override;
};

} // namespace algorithm
} // namespace common
