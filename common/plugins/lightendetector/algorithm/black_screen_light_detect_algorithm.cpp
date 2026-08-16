/**
 * @file black_screen_light_detect_algorithm.cpp
 * @brief 黑屏亮灯检测算法实现。
 *
 * 通过 ShotMeter 对黑场背景图像执行 Shot + Meter，复用白场轮廓，
 * 使用 MeterInspect 检测异常亮灯点位并写入 black_screen_lights。
 */

#include "black_screen_light_detect_algorithm.h"

#include <common/plugins/lightendetector/service/algo_sdk_guard.h>
#include <common/plugins/lightendetector/service/shot_meter.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/3rdparty/cisdk/measurement/include/algo_led_meter.h>
#include <common/utility/errors/include/error.h>

#include <algorithm>
#include <vector>

namespace common {
namespace algorithm {

std::string BlackScreenLightDetectAlgorithm::Name() const
{
    return "BlackScreenLightDetect";
}

std::string BlackScreenLightDetectAlgorithm::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr BlackScreenLightDetectAlgorithm::Execute(AlgorithmExecuteData& data)
{
    const auto* p = data.inputs.Get<BlackScreenLightDetectParams>("params");
    if (!p || !p->bg_image || p->bg_image->Empty()) {
        return common::errors::New(1, "BlackScreenLightDetect: missing bg_image");
    }
    if (p->led_w <= 0 || p->led_h <= 0) {
        return common::errors::New(2, "BlackScreenLightDetect: led_w/led_h must be positive");
    }
    if (p->white_green_contour_file_name.empty()) {
        return common::errors::New(3, "BlackScreenLightDetect: white_green_contour_file_name required");
    }

    // 1. 对黑场背景图像执行 Shot + Meter（复用 white_green.contour）。
    ShotMeter shot_meter;
    MeterResult meter{};
    if (auto e = shot_meter.RunBlackScreenLight(*p, &meter)) return e;

    MeterInspectGuard inspector;
    if (auto e = FailAlgo("MeterInspectCreateHandle", MeterInspectCreateHandle(inspector.out(), 0x4440))) return e;

    // 2. 设置黑屏亮灯检测阈值并执行 MeterInspectBlackScreenLights。
    MeterInspectThresh thresh{};
    thresh.thr_black_lights = p->detect_threshold.thr_black_light;
    thresh.weight_val = 1.0f;
    thresh.temp_size = 11;
    if (auto e = FailAlgo("MeterInspectSetThresh", MeterInspectSetThresh(inspector.get(), &thresh))) return e;

    const int max_pts = std::max(1, p->led_w * p->led_h);
    std::vector<AlgoPoint> defects(static_cast<size_t>(max_pts));
    int out_n = max_pts;
    if (auto e = FailAlgo("MeterInspectBlackScreenLights",
                          MeterInspectBlackScreenLights(inspector.get(), meter.meter.get(), defects.data(), &out_n))) return e;
    defects.resize(static_cast<size_t>(std::max(0, out_n)));

    // 3. 可选：导出缺陷点位标注结果图。
    if (!p->black_screen_light_png_file_name.empty()) {
        if (auto e = FailAlgo("LedMeterExportChromaImage",
                              LedMeterExportChromaImage(meter.meter.get(), p->black_screen_light_png_file_name.c_str(), 1, 11, defects.data(), out_n))) return e;
    }

    // 4. 转换并输出缺陷点列表。
    std::vector<BlackScreenLightDefectPoint> out;
    out.reserve(defects.size());
    for (const auto& pt : defects) {
        out.push_back({pt.x, pt.y});
    }
    data.outputs.Set("black_screen_lights", std::move(out));
    return nullptr;
}

REGISTER_ALGORITHM("BlackScreenLightDetect", BlackScreenLightDetectAlgorithm)

} // namespace algorithm
} // namespace common
