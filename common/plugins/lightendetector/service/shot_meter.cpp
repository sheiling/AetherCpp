/**
 * @file shot_meter.cpp
 * @brief Shot + Meter 组合流程统一入口实现。
 *
 * 针对各检测场景封装 Shot 预处理、背景减除与 Meter 亮度提取，
 * 屏蔽底层 ShotAlgorithm / MeterAlgorithm 的调用细节。
 */

#include "shot_meter.h"

#include "algo_sdk_guard.h"
#include "shot_algorithm.h"

#include <common/3rdparty/cisdk/measurement/include/algo_shot.h>
#include <common/3rdparty/cisdk/measurement/include/meta_info_enum.h>

namespace common {
namespace algorithm {
namespace {

// 将业务层纯色枚举映射为 CISDK AlgoColor。
inline AlgoColor ToAlgoColor(PureDetectColor c)
{
    switch (c) {
    case PureDetectColor::Red: return COLOR_RED;
    case PureDetectColor::Green: return COLOR_GREEN;
    case PureDetectColor::Blue: return COLOR_BLUE;
    case PureDetectColor::Black: return COLOR_BLACK;
    case PureDetectColor::White: return COLOR_WHITE;
    }
    return COLOR_WHITE;
}

// 从 ShotResult 中按目标颜色选取对应通道的 Shot 句柄。
inline AlgoHandle ChannelHandle(const ShotResult& shots, PureDetectColor c)
{
    switch (c) {
    case PureDetectColor::Red: return shots.red.get();
    case PureDetectColor::Green: return shots.green.get();
    case PureDetectColor::Blue: return shots.blue.get();
    default: return nullptr;
    }
}

// 根据白屏检测参数填充 ShotRequest。
void FillShotRequest(const WhiteContourGenParams& p,
                     std::shared_ptr<ImageBuffer> image,
                     AlgoColor color,
                     bool need_original,
                     ShotRequest* out)
{
    out->image = std::move(image);
    out->isColorCamera = p.is_color_camera;
    out->fixingDefectiveThreshold = p.fixing_defective_thr;
    out->needOriginal = need_original;
    out->color = color;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = p.cam_exposure;
    if (p.virtual_flag) {
        out->panel.layout = LED_LAYOUT_TRIANGLE_ROW;
    }
}

// 根据混色检测参数填充 ShotRequest。
void FillShotRequest(const MixColorVariationDetectParams& p,
                     std::shared_ptr<ImageBuffer> image,
                     AlgoColor color,
                     bool need_original,
                     ShotRequest* out)
{
    out->image = std::move(image);
    out->isColorCamera = p.is_color_camera;
    out->fixingDefectiveThreshold = p.fixing_defective_thr;
    out->needOriginal = need_original;
    out->color = color;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = p.cam_exposure;
    if (p.virtual_flag) {
        out->panel.layout = LED_LAYOUT_TRIANGLE_ROW;
    }
}

// 根据纯色检测参数填充 ShotRequest。
void FillShotRequest(const PureColorDetectParams& p,
                     int exposure_ms,
                     std::shared_ptr<ImageBuffer> image,
                     AlgoColor color,
                     ShotRequest* out)
{
    out->image = std::move(image);
    out->isColorCamera = p.is_color_camera;
    out->fixingDefectiveThreshold = p.fixing_defective_thr;
    out->color = color;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = exposure_ms;
    if (p.virtual_flag) {
        out->panel.layout = LED_LAYOUT_TRIANGLE_ROW;
    }
}

// 根据串灯检测参数填充 ShotRequest。
void FillShotRequest(const StringLightDetectParams& p,
                     int exposure_ms,
                     std::shared_ptr<ImageBuffer> image,
                     AlgoColor color,
                     ShotRequest* out)
{
    out->image = std::move(image);
    out->isColorCamera = p.is_color_camera;
    out->fixingDefectiveThreshold = p.fixing_defective_thr;
    out->color = color;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = exposure_ms;
}

// 根据黑屏亮灯检测参数填充黑场 ShotRequest。
void FillShotRequest(const BlackScreenLightDetectParams& p,
                     std::shared_ptr<ImageBuffer> bg_image,
                     ShotRequest* out)
{
    out->image = std::move(bg_image);
    out->isColorCamera = p.is_color_camera;
    out->fixingDefectiveThreshold = p.fixing_defective_thr;
    out->color = COLOR_BLACK;
    out->part.screen_wh = {p.led_w, p.led_h};
    out->part.xywh = {0, 0, p.led_w, p.led_h};
    out->panel.wh = {p.led_w, p.led_h};
    out->pattern.lighted_led_wh = {p.led_w, p.led_h};
    out->capture.exposure = p.cam_exposure;
}

// 由 white_*.contour 路径推导轮廓预览图 white_*_contour.png。
std::string ContourPicturePath(const std::string& contour_file)
{
    constexpr const char* kSuffix = ".contour";
    if (contour_file.size() > 8 &&
        contour_file.compare(contour_file.size() - 8, 8, kSuffix) == 0) {
        return contour_file.substr(0, contour_file.size() - 8) + "_contour.png";
    }
    return {};
}

// 根据白屏检测参数构建 MeterRequest（强制点定位并落盘轮廓）。
MeterRequest BuildMeterRequest(const WhiteContourGenParams& p,
                               AlgoHandle shot,
                               const std::string& contour_file,
                               const std::string& contour_picture)
{
    MeterRequest req{};
    req.shotHandle = shot;
    req.imageSize = p.image_size;
    req.corner4[0] = p.left_top;
    req.corner4[1] = p.right_top;
    req.corner4[2] = p.right_bottom;
    req.corner4[3] = p.left_bottom;
    req.useAutoROI = p.use_auto_roi;
    req.isSide = false;
    req.angle = p.angle;
    req.contourReuse = false;
    req.saveContour = true;
    req.contourFileName = contour_file;
    req.saveContourPicture = !contour_picture.empty();
    req.contourPicturePath = contour_picture;
    return req;
}

// 根据混色检测参数构建 MeterRequest（强制复用白场轮廓，不保存）。
MeterRequest BuildMeterRequest(const MixColorVariationDetectParams& /*p*/,
                               AlgoHandle shot,
                               const std::string& contour_file)
{
    MeterRequest req{};
    req.shotHandle = shot;
    req.contourReuse = true;
    req.saveContour = false;
    req.contourFileName = contour_file;
    return req;
}

// 根据纯色检测参数构建 MeterRequest。
MeterRequest BuildMeterRequest(const PureColorDetectParams& p, AlgoHandle shot)
{
    MeterRequest req{};
    req.shotHandle = shot;
    req.imageSize = p.image_size;
    req.corner4[0] = p.left_top;
    req.corner4[1] = p.right_top;
    req.corner4[2] = p.right_bottom;
    req.corner4[3] = p.left_bottom;
    req.useAutoROI = p.use_auto_roi;
    req.isSide = p.is_side;
    req.angle = p.angle;
    req.contourReuse = false;
    req.saveContour = !p.pure_contour_file_name.empty();
    req.contourFileName = p.pure_contour_file_name;
    return req;
}

// 根据串灯检测参数构建 MeterRequest。
// 纯色检测开启时复用 contour；未开启时用 ROI 做点定位，不保存 contour。
MeterRequest BuildMeterRequest(const StringLightDetectParams& p, AlgoHandle shot)
{
    MeterRequest req{};
    req.shotHandle = shot;
    if (!p.pure_contour_file_name.empty()) {
        req.contourReuse = true;
        req.saveContour = false;
        req.contourFileName = p.pure_contour_file_name;
    } else {
        req.imageSize = p.image_size;
        req.corner4[0] = p.left_top;
        req.corner4[1] = p.right_top;
        req.corner4[2] = p.right_bottom;
        req.corner4[3] = p.left_bottom;
        req.useAutoROI = p.use_auto_roi;
        req.contourReuse = false;
        req.saveContour = false;
    }
    return req;
}

// 根据黑屏亮灯检测参数构建 MeterRequest（强制复用白场轮廓，不保存）。
MeterRequest BuildMeterRequest(const BlackScreenLightDetectParams& p, AlgoHandle shot)
{
    MeterRequest req{};
    req.shotHandle = shot;
    req.contourReuse = true;
    req.saveContour = false;
    req.contourFileName = p.white_green_contour_file_name;
    return req;
}

} // namespace

common::errors::ErrorPtr ShotMeter::RunWhite(const WhiteContourGenParams& p,
                                             RgbMeterResult* out) const
{
    if (!out) {
        return common::errors::New(10, "ShotMeter::RunWhite: output is null");
    }

    const WhiteContourGenParams& params = p;
    if (params.white_red_contour_file_name.empty() ||
        params.white_green_contour_file_name.empty() ||
        params.white_blue_contour_file_name.empty()) {
        return common::errors::New(11, "ShotMeter::RunWhite: white_*_contour_file_name required");
    }

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    // 1. 对正常曝光图像执行 Shot，保留原始 Shot 用于后续保存。
    ShotRequest normal_req{};
    FillShotRequest(params,
                    params.image,
                    ToAlgoColor(params.color),
                    true,
                    &normal_req);
    normal_req.saveImage = params.save_original_image_flag;
    normal_req.imageFileName = params.white_image_file_name;

    ShotResult normal_shot{};
    if (auto e = FailAlgo("ShotAlgorithm.run(normal)", shot_algo.run(normal_req, &normal_shot))) return e;

    // 2. 对 R/G/B 三通道分别构建 MeterRequest 并强制点定位生成 white_*.contour。
    MeterRequest r_req = BuildMeterRequest(params, normal_shot.red.get(),
                                           params.white_red_contour_file_name,
                                           ContourPicturePath(params.white_red_contour_file_name));
    MeterRequest g_req = BuildMeterRequest(params, normal_shot.green.get(),
                                           params.white_green_contour_file_name,
                                           ContourPicturePath(params.white_green_contour_file_name));
    MeterRequest b_req = BuildMeterRequest(params, normal_shot.blue.get(),
                                           params.white_blue_contour_file_name,
                                           ContourPicturePath(params.white_blue_contour_file_name));

    if (auto e = FailAlgo("MeterAlgorithm.run(r)", meter_algo.run(r_req, &out->red))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(g)", meter_algo.run(g_req, &out->green))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(b)", meter_algo.run(b_req, &out->blue))) return e;
    return nullptr;
}

common::errors::ErrorPtr ShotMeter::RunMixColorVariation(const MixColorVariationDetectParams& p,
                                                         RgbMeterResult* out) const
{
    if (!out) {
        return common::errors::New(10, "ShotMeter::RunMixColorVariation: output is null");
    }

    const MixColorVariationDetectParams& params = p;

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    // 1. 对前景图像执行 Shot（曝光复用纯色检测采图参数）。
    ShotRequest fg{};
    FillShotRequest(params, params.image, ToAlgoColor(params.color), false, &fg);

    ShotResult foreground{};
    AlgoError err = shot_algo.run(fg, &foreground);
    if (auto e = FailAlgo("ShotAlgorithm.run(foreground)", err)) return e;

    // 2. 对 R/G/B 三通道分别执行 Meter，强制复用白场预生成的 white_*.contour。
    MeterRequest r_req = BuildMeterRequest(params, foreground.red.get(), params.white_red_contour_file_name);
    MeterRequest g_req = BuildMeterRequest(params, foreground.green.get(), params.white_green_contour_file_name);
    MeterRequest b_req = BuildMeterRequest(params, foreground.blue.get(), params.white_blue_contour_file_name);

    if (auto e = FailAlgo("MeterAlgorithm.run(r)", meter_algo.run(r_req, &out->red))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(g)", meter_algo.run(g_req, &out->green))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(b)", meter_algo.run(b_req, &out->blue))) return e;
    return nullptr;
}

common::errors::ErrorPtr ShotMeter::RunPureColor(const PureColorDetectParams& p, MeterResult* out) const
{
    if (!out) {
        return common::errors::New(10, "ShotMeter::RunPureColor: output is null");
    }

    const PureColorDetectParams& params = p;

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    // 1. 对前景图像执行 Shot。
    ShotRequest fg{};
    FillShotRequest(params, params.cam_exposure, params.image, ToAlgoColor(params.color), &fg);
    fg.saveImage = params.save_original_image_flag;
    fg.imageFileName = params.normal_image_file;

    ShotResult foreground{};
    AlgoError err = shot_algo.run(fg, &foreground);
    if (auto e = FailAlgo("ShotAlgorithm.run(foreground)", err)) return e;

    // 2. 对目标颜色通道执行 Meter 测量（有轮廓路径时强制点定位并落盘）。
    MeterRequest req = BuildMeterRequest(params, ChannelHandle(foreground, params.color));
    err = meter_algo.run(req, out);
    if (auto e = FailAlgo("MeterAlgorithm.run", err)) return e;
    return nullptr;
}

common::errors::ErrorPtr ShotMeter::RunStringLight(const StringLightDetectParams& p, PairMeterResult* out) const
{
    if (!out) {
        return common::errors::New(10, "ShotMeter::RunStringLight: output is null");
    }

    const StringLightDetectParams& params = p;

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    // 1. 对左右两幅图像分别执行 Shot。
    ShotRequest left_req{};
    FillShotRequest(params, params.cam_exposure, params.left_image, ToAlgoColor(params.color), &left_req);
    left_req.saveImage = params.save_original_image_flag;
    left_req.imageFileName = params.normal_left_image_file_name;

    ShotRequest right_req{};
    FillShotRequest(params, params.cam_exposure, params.right_image, ToAlgoColor(params.color), &right_req);
    right_req.saveImage = params.save_original_image_flag;
    right_req.imageFileName = params.normal_right_image_file_name;

    ShotResult left_shot{}, right_shot{};
    if (auto e = FailAlgo("ShotAlgorithm.run(left)", shot_algo.run(left_req, &left_shot))) return e;
    if (auto e = FailAlgo("ShotAlgorithm.run(right)", shot_algo.run(right_req, &right_shot))) return e;

    // 2. 对左右图像分别执行 Meter 测量。
    MeterRequest left_meter_req = BuildMeterRequest(params, ChannelHandle(left_shot, params.color));
    MeterRequest right_meter_req = BuildMeterRequest(params, ChannelHandle(right_shot, params.color));
    if (auto e = FailAlgo("MeterAlgorithm.run(left)", meter_algo.run(left_meter_req, &out->first))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(right)", meter_algo.run(right_meter_req, &out->second))) return e;
    return nullptr;
}

common::errors::ErrorPtr ShotMeter::RunBlackScreenLight(const BlackScreenLightDetectParams& p,
                                                         MeterResult* out) const
{
    if (!out) {
        return common::errors::New(10, "ShotMeter::RunBlackScreenLight: output is null");
    }

    const BlackScreenLightDetectParams& params = p;

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    // 1. 对黑场背景图像执行 Shot。
    ShotRequest bg_req{};
    FillShotRequest(params, params.bg_image, &bg_req);

    ShotResult bg_shot{};
    if (auto e = FailAlgo("ShotAlgorithm.run(bg)", shot_algo.run(bg_req, &bg_shot))) return e;

    // 2. 对绿色通道执行 Meter 测量（黑屏亮灯检测使用绿通道）。
    MeterRequest bg_meter_req = BuildMeterRequest(params, bg_shot.green.get());
    if (auto e = FailAlgo("MeterAlgorithm.run(bg)", meter_algo.run(bg_meter_req, out))) return e;
    return nullptr;
}

} // namespace algorithm
} // namespace common
