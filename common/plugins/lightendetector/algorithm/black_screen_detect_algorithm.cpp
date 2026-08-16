/**
 * @file black_screen_detect_algorithm.cpp
 * @brief 黑屏检测算法实现。
 *
 * 对白场与黑场图像分别执行 Shot，通过 MeterInspect 对比判定屏幕是否处于黑屏状态。
 */

#include "black_screen_detect_algorithm.h"

#include <common/plugins/lightendetector/service/algo_sdk_guard.h>
#include <common/plugins/lightendetector/service/shot_algorithm.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/utility/errors/include/error.h>

namespace common {
namespace algorithm {
namespace {

// 根据黑屏检测参数填充 ShotRequest。
void FillShotRequest(const BlackScreenDetectParams& p,
                     int exposure_ms,
                     std::shared_ptr<ImageBuffer> img,
                     AlgoColor color,
                     const std::string& img_file,
                     ShotRequest* out)
{
    out->image = std::move(img);
    out->isColorCamera = true;
    out->color = color;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = exposure_ms;
    out->saveImage = p.save_original_image_flag && !img_file.empty();
    out->imageFileName = img_file;
}

} // namespace

std::string BlackScreenDetectAlgorithm::Name() const
{
    return "BlackScreenDetect";
}

std::string BlackScreenDetectAlgorithm::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr BlackScreenDetectAlgorithm::Execute(AlgorithmExecuteData& data)
{
    const auto* p = data.inputs.Get<BlackScreenDetectParams>("params");
    if (!p || !p->normal_image || p->normal_image->Empty() || !p->black_image || p->black_image->Empty()) {
        return common::errors::New(1, "BlackScreenDetect: missing normal_image/black_image");
    }
    if (p->led_w <= 0 || p->led_h <= 0) {
        return common::errors::New(2, "BlackScreenDetect: led_w/led_h must be positive");
    }

    ShotAlgorithm shot_algo;
    MeterInspectGuard inspector;
    if (auto e = FailAlgo("MeterInspectCreateHandle", MeterInspectCreateHandle(inspector.out(), 0x4440))) return e;

    // 1. 对黑场与白场图像分别执行 Shot 预处理（曝光复用采图参数）。
    ShotRequest black_req{};
    FillShotRequest(*p,
                    p->black_cam_exposure,
                    p->black_image,
                    COLOR_BLACK,
                    p->black_image_file_name,
                    &black_req);

    ShotRequest normal_req{};
    FillShotRequest(*p,
                    p->white_cam_exposure,
                    p->normal_image,
                    COLOR_WHITE,
                    std::string{},
                    &normal_req);

    ShotResult black_shot{}, normal_shot{};
    if (auto e = FailAlgo("ShotAlgorithm.run(black)", shot_algo.run(black_req, &black_shot))) return e;
    if (auto e = FailAlgo("ShotAlgorithm.run(normal)", shot_algo.run(normal_req, &normal_shot))) return e;

    // 2. 设置黑屏检测阈值并执行 MeterInspectBlackScreen（使用绿色通道 Shot）。
    MeterInspectThresh thresh{};
    thresh.thr_black = p->detect_threshold.thr_black_screen;
    thresh.weight_val = 1.0f;
    thresh.temp_size = 11;
    if (auto e = FailAlgo("MeterInspectSetThresh", MeterInspectSetThresh(inspector.get(), &thresh))) return e;

    int result = 0;
    if (auto e = FailAlgo("MeterInspectBlackScreen",
                          MeterInspectBlackScreen(inspector.get(), normal_shot.green.get(), black_shot.green.get(), &result))) return e;

    data.outputs.Set("is_black_screen", result);
    return nullptr;
}

REGISTER_ALGORITHM("BlackScreenDetect", BlackScreenDetectAlgorithm)

} // namespace algorithm
} // namespace common
