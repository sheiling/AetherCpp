#pragma once

/**
 * @file black_screen_detect_algorithm.h
 * @brief 黑屏检测算法声明。
 *
 * 对比白场与黑场图像，判断屏幕是否处于黑屏状态。
 * 输入：BlackScreenDetectParams；输出：is_black_screen。
 * 实现见 black_screen_detect_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_base.h>
#include <common/include/algorithm_data/lightendetector_data.h>

namespace common {
namespace algorithm {

// 黑屏检测算法：对比白场与黑场图像，判断屏幕是否处于黑屏状态。
// 输入：BlackScreenDetectParams（normal_image / black_image、black/white_cam_exposure 等）
// 输出：is_black_screen（非零表示检测到黑屏）
class BlackScreenDetectAlgorithm final : public AlgorithmBase {
public:
    std::string Name() const override;
    std::string Version() const override;
    common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) override;
};

} // namespace algorithm
} // namespace common
