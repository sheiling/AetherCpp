#pragma once

/**
 * @file shot_meter.h
 * @brief Shot + Meter 组合流程统一入口声明。
 *
 * 针对白屏、黑场、纯色、混色、串灯、黑屏亮灯等检测场景，
 * 封装「Shot 预处理 → 可选背景减除 → Meter 亮度提取」完整流水线。
 * 实现见 shot_meter.cpp。
 */

#include "meter_algorithm.h"

#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/utility/errors/include/error.h>

namespace common {
namespace algorithm {

// RGB 三通道 Meter 测量结果，用于白屏检测与混色检测等需要分通道分析的场景。
struct RgbMeterResult final {
    MeterResult red{};   // 红色通道测量结果
    MeterResult green{}; // 绿色通道测量结果
    MeterResult blue{};  // 蓝色通道测量结果
};

// 双路 Meter 测量结果，用于串灯检测等需要对比左右两幅图像的场景。
struct PairMeterResult final {
    MeterResult first{};  // 第一路（通常为左侧）测量结果
    MeterResult second{}; // 第二路（通常为右侧）测量结果
};

// Shot + Meter 组合流程的统一入口。
// 针对各检测场景封装「Shot 预处理 → 可选背景减除 → Meter 亮度提取」的完整流水线，
// 屏蔽底层 ShotAlgorithm / MeterAlgorithm 的调用细节。
class ShotMeter final {
public:
    // 白屏检测：对正常曝光图像执行 Shot 拆分，分别对 R/G/B 通道做 Meter 测量。
    common::errors::ErrorPtr RunWhite(const WhiteContourGenParams& params,
                                      RgbMeterResult* out) const;

    // 混色检测：对前景图像的 R/G/B 通道分别测量，复用纯色采图曝光与白场轮廓。
    common::errors::ErrorPtr RunMixColorVariation(const MixColorVariationDetectParams& params,
                                                  RgbMeterResult* out) const;

    // 纯色检测：对指定颜色通道执行 Shot 与 Meter，可选背景减除。
    common::errors::ErrorPtr RunPureColor(const PureColorDetectParams& params,
                                          MeterResult* out) const;

    // 串灯检测：对左右两幅图像分别执行 Shot 与 Meter，可选背景减除。
    common::errors::ErrorPtr RunStringLight(const StringLightDetectParams& params,
                                            PairMeterResult* out) const;

    // 黑屏亮灯检测：对黑场背景图像执行 Shot 与 Meter，复用白场轮廓。
    common::errors::ErrorPtr RunBlackScreenLight(const BlackScreenLightDetectParams& params,
                                                 MeterResult* out) const;
};

} // namespace algorithm
} // namespace common
