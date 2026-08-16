/**
 * @file mix_color_variation_detect_algorithm.cpp
 * @brief 混色检测算法实现。
 *
 * 基于白场参考轮廓与纯色待测图，对 RGB 三通道分别执行 Shot + Meter，
 * 通过 MeterInspect 联合计量检测混色缺陷并写入 mix_colors。
 */

#include "mix_color_variation_detect_algorithm.h"

#include <common/plugins/lightendetector/service/algo_sdk_guard.h>
#include <common/plugins/lightendetector/service/shot_meter.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/3rdparty/cisdk/measurement/include/algo_led_meter.h>
#include <common/3rdparty/cisdk/measurement/include/meta_info_enum.h>
#include <common/utility/errors/include/error.h>

#include <algorithm>

namespace common {
namespace algorithm {
namespace {

// 将业务层混色检测阈值转换为 CISDK MeterInspectMixColorThresh。
MeterInspectMixColorThresh BuildMixColorThresh(const MixColorVariationDetectParams& p)
{
    MeterInspectMixColorThresh t{};
    t.thr_mixColor_preSelect_R = p.mix_color_detect_threshold.thr_mix_color_pre_red;
    t.thr_mixColor_deltaE_R = p.mix_color_detect_threshold.thr_mix_color_red;
    t.thr_mixColor_preSelect_G = p.mix_color_detect_threshold.thr_mix_color_pre_green;
    t.thr_mixColor_deltaE_G = p.mix_color_detect_threshold.thr_mix_color_green;
    t.thr_mixColor_preSelect_B = p.mix_color_detect_threshold.thr_mix_color_pre_blue;
    t.thr_mixColor_deltaE_B = p.mix_color_detect_threshold.thr_mix_color_blue;
    t.thr_mixColor_preSelect_W = 5.f;
    t.thr_mixColor_deltaE_W = 21.f;
    t.thr_preSelect_median_filter_kernel_size = 5;
    t.thr_preSelect_mean_filter_kernel_size = 13;
    t.thr_deltaE_block_size = p.mix_color_detect_threshold.mix_color_block_size;
    return t;
}

// 将业务层纯色检测阈值转换为 CISDK MeterInspectThresh（混色前置死灯 / 暗灯检测用）。
MeterInspectThresh BuildMixInspectThresh(const MixColorVariationDetectParams& p)
{
    MeterInspectThresh t{};
    t.thr_dead_R = p.pure_detect_threshold.thr_dead_R;
    t.thr_dead_G = p.pure_detect_threshold.thr_dead_G;
    t.thr_dead_B = p.pure_detect_threshold.thr_dead_B;
    t.thr_dimMin_R = p.pure_detect_threshold.thr_dim_min_R;
    t.thr_dimMax_R = p.pure_detect_threshold.thr_dim_max_R;
    t.thr_dimMin_G = p.pure_detect_threshold.thr_dim_min_G;
    t.thr_dimMax_G = p.pure_detect_threshold.thr_dim_max_G;
    t.thr_dimMin_B = p.pure_detect_threshold.thr_dim_min_B;
    t.thr_dimMax_B = p.pure_detect_threshold.thr_dim_max_B;
    t.thr_bright_R = p.pure_detect_threshold.thr_bright_R;
    t.thr_bright_G = p.pure_detect_threshold.thr_bright_G;
    t.thr_bright_B = p.pure_detect_threshold.thr_bright_B;
    t.weight_val = 1.0f;
    t.temp_size = 11;
    return t;
}

// 联合 RGB 三通道 Meter 执行混色缺陷检测，并填充输出列表。
common::errors::ErrorPtr RunMixColorDetect(
    const MixColorVariationDetectParams& p,
    AlgoHandle red_meter,
    AlgoHandle green_meter,
    AlgoHandle blue_meter,
    std::vector<MixColorDefectPoint>* mix_colors)
{
    if (!mix_colors) {
        return common::errors::New(10, "MixColorVariationDetect: output mix_colors is null");
    }

    MeterInspectGuard mix_inspector;
    if (auto e = FailAlgo("MeterInspectCreateHandle(mix)", MeterInspectCreateHandle(mix_inspector.out(), 0x4440))) return e;
    const MeterInspectThresh inspect_thresh = BuildMixInspectThresh(p);
    if (auto e = FailAlgo("MeterInspectSetThresh(mix)", MeterInspectSetThresh(mix_inspector.get(), &inspect_thresh))) return e;
    const MeterInspectMixColorThresh mix_thresh = BuildMixColorThresh(p);
    if (auto e = FailAlgo("MeterInspectSetMixColorThresh", MeterInspectSetMixColorThresh(mix_inspector.get(), &mix_thresh))) return e;

    // 按待测颜色选取主通道 Meter，用于死灯 / 暗灯前置检测及结果图导出。
    AlgoHandle main_meter = (p.color == PureDetectColor::Red) ? red_meter
                        : (p.color == PureDetectColor::Green) ? green_meter
                        : blue_meter;
    if (auto e = FailAlgo("MeterInspectDeadLed(mix)", MeterInspectDeadLed(mix_inspector.get(), main_meter))) return e;
    if (auto e = FailAlgo("MeterInspectDarkLed(mix)", MeterInspectDarkLed(mix_inspector.get(), main_meter))) return e;
    if (auto e = FailAlgo("MeterInspectMixColor",
                          MeterInspectMixColor(mix_inspector.get(), red_meter, green_meter, blue_meter))) return e;

    const int max_pts = std::max(1, p.led_w * p.led_h);
    std::vector<AlgoPoint> list(static_cast<size_t>(max_pts));
    int n = max_pts;
    if (auto e = FailAlgo("MeterInspectGetResultList(mix)",
                          MeterInspectGetResultList(mix_inspector.get(), LED_DEFECT_FLAG_MIX_COLOR, list.data(), &n, LED_DEFECT_FLAG_DIM | LED_DEFECT_FLAG_MIX_COLOR))) return e;
    n = std::max(0, n);
    list.resize(static_cast<size_t>(n));

    // 收集混色缺陷点，可选导出结果图。
    mix_colors->clear();
    mix_colors->reserve(list.size());
    for (const AlgoPoint& pt : list) {
        mix_colors->push_back({pt.x, pt.y});
    }

    if (!p.mix_color_png_file_name.empty()) {
        if (auto e = FailAlgo("LedMeterExportChromaImage(mix)",
                              LedMeterExportChromaImage(main_meter, p.mix_color_png_file_name.c_str(), 1, 11, list.data(), n))) return e;
    }
    return nullptr;
}

} // namespace

std::string MixColorVariationDetectAlgorithm::Name() const
{
    return "MixColorVariationDetect";
}

std::string MixColorVariationDetectAlgorithm::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr MixColorVariationDetectAlgorithm::Execute(AlgorithmExecuteData& data)
{
    const auto* p = data.inputs.Get<MixColorVariationDetectParams>("params");
    if (!p || !p->image || p->image->Empty()) {
        return common::errors::New(1, "MixColorVariationDetect: missing inputs.params.image");
    }
    if (p->led_w <= 0 || p->led_h <= 0) {
        return common::errors::New(2, "MixColorVariationDetect: led_w/led_h must be positive");
    }
    if (p->white_red_contour_file_name.empty() ||
        p->white_green_contour_file_name.empty() ||
        p->white_blue_contour_file_name.empty()) {
        return common::errors::New(4, "MixColorVariationDetect: white_*_contour_file_name required");
    }

    ShotMeter shot_meter;

    // 1. 对待测纯色图执行混色 Shot + Meter，复用步骤预生成的白场 white_*.contour。
    RgbMeterResult rgb_meter{};
    if (auto e = shot_meter.RunMixColorVariation(*p, &rgb_meter)) return e;

    // 2. 联合 RGB 计量执行混色缺陷检测，输出 mix_colors。
    std::vector<MixColorDefectPoint> mix_colors;
    if (auto e = RunMixColorDetect(
            *p,
            rgb_meter.red.meter.get(),
            rgb_meter.green.meter.get(),
            rgb_meter.blue.meter.get(),
            &mix_colors)) {
        return e;
    }

    data.outputs.Set("mix_colors", std::move(mix_colors));
    return nullptr;
}

REGISTER_ALGORITHM("MixColorVariationDetect", MixColorVariationDetectAlgorithm)

} // namespace algorithm
} // namespace common
