/**
 * @file pure_color_detect_algorithm.cpp
 * @brief 纯色检测算法实现。
 *
 * 对指定颜色通道执行 Shot + Meter，通过 MeterInspect 检测死灯、暗灯、亮灯及串色缺陷，
 * 结果分别写入 deads / darks / brights / color_variations。
 */

#include "pure_color_detect_algorithm.h"

#include <common/plugins/lightendetector/service/algo_sdk_guard.h>
#include <common/plugins/lightendetector/service/meter_algorithm.h>
#include <common/plugins/lightendetector/service/shot_algorithm.h>
#include <common/plugins/lightendetector/service/shot_meter.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/3rdparty/cisdk/measurement/include/algo_led_meter.h>
#include <common/3rdparty/cisdk/measurement/include/algo_mat.h>
#include <common/3rdparty/cisdk/measurement/include/algo_shot.h>
#include <common/3rdparty/cisdk/measurement/include/meta_info_enum.h>

#include <common/utility/errors/include/error.h>

#include <algorithm>
#include <fstream>

namespace common {
namespace algorithm {
namespace {

inline AlgoColor ToAlgoColor(PureDetectColor c)
{
    switch (c) {
    case PureDetectColor::Red: return COLOR_RED;
    case PureDetectColor::Green: return COLOR_GREEN;
    case PureDetectColor::Blue: return COLOR_BLUE;
    case PureDetectColor::White: return COLOR_WHITE;
    }
    return COLOR_WHITE;
}

void FillColorVariationShotRequest(const PureColorDetectParams& p,
                                   int exposure_ms,
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
    out->capture.exposure = exposure_ms;
    if (p.virtual_flag) {
        out->panel.layout = LED_LAYOUT_TRIANGLE_ROW;
    }
}

MeterRequest BuildColorVariationMeterRequest(const PureColorDetectParams& p,
                                             AlgoHandle shot,
                                             const std::string& contour_file)
{
    MeterRequest req{};
    req.shotHandle = shot;
    req.contourReuse = !contour_file.empty();
    req.saveContour = false;
    req.contourFileName = contour_file;
    return req;
}

// 将业务层纯色检测阈值转换为 CISDK MeterInspectThresh。
MeterInspectThresh BuildMeterInspectThresh(const PureColorDetectThreshold& t)
{
    MeterInspectThresh m{};
    m.thr_dead_R = t.thr_dead_R;
    m.thr_dead_G = t.thr_dead_G;
    m.thr_dead_B = t.thr_dead_B;
    m.thr_dimMin_R = t.thr_dim_min_R;
    m.thr_dimMax_R = t.thr_dim_max_R;
    m.thr_dimMin_G = t.thr_dim_min_G;
    m.thr_dimMax_G = t.thr_dim_max_G;
    m.thr_dimMin_B = t.thr_dim_min_B;
    m.thr_dimMax_B = t.thr_dim_max_B;
    m.thr_bright_R = t.thr_bright_R;
    m.thr_bright_G = t.thr_bright_G;
    m.thr_bright_B = t.thr_bright_B;
    m.weight_val = 1.0f;
    m.temp_size = 11;
    return m;
}

MeterInspectThresh BuildColorVariationInspectThresh(const PureColorDetectParams& p)
{
    MeterInspectThresh t = BuildMeterInspectThresh(p.detect_threshold);
    t.thr_Red_R_max = p.color_variation_detect_threshold.thr_color_variation_red_r;
    t.thr_Red_G_min = p.color_variation_detect_threshold.thr_color_variation_red_g;
    t.thr_Red_B_min = p.color_variation_detect_threshold.thr_color_variation_red_b;
    t.thr_Green_R_min = p.color_variation_detect_threshold.thr_color_variation_green_r;
    t.thr_Green_G_max = p.color_variation_detect_threshold.thr_color_variation_green_g;
    t.thr_Green_B_min = p.color_variation_detect_threshold.thr_color_variation_green_b;
    t.thr_Blue_R_min = p.color_variation_detect_threshold.thr_color_variation_blue_r;
    t.thr_Blue_G_min = p.color_variation_detect_threshold.thr_color_variation_blue_g;
    t.thr_Blue_B_max = p.color_variation_detect_threshold.thr_color_variation_blue_b;
    return t;
}

// 按检测颜色取对应通道的死灯阈值。
float DeadThreshold(PureDetectColor c, const PureColorDetectThreshold& t)
{
    switch (c) {
    case PureDetectColor::Red:
        return t.thr_dead_R;
    case PureDetectColor::Green:
        return t.thr_dead_G;
    case PureDetectColor::Blue:
        return t.thr_dead_B;
    }
    return 0.f;
}

// 按检测颜色取对应通道的暗灯阈值（dim max）。
float DarkThreshold(PureDetectColor c, const PureColorDetectThreshold& t)
{
    switch (c) {
    case PureDetectColor::Red:
        return t.thr_dim_max_R;
    case PureDetectColor::Green:
        return t.thr_dim_max_G;
    case PureDetectColor::Blue:
        return t.thr_dim_max_B;
    }
    return 0.f;
}

// 按检测颜色取对应通道的亮灯阈值。
float BrightThreshold(PureDetectColor c, const PureColorDetectThreshold& t)
{
    switch (c) {
    case PureDetectColor::Red:
        return t.thr_bright_R;
    case PureDetectColor::Green:
        return t.thr_bright_G;
    case PureDetectColor::Blue:
        return t.thr_bright_B;
    }
    return 0.f;
}

// 从 MeterInspect 内部数据填充死灯缺陷点。
void FillDeadPoints(PureDetectColor color,
                    const PureColorDetectThreshold& thr,
                    AlgoMat* qty,
                    const std::vector<AlgoPoint>& pts,
                    std::vector<PureColorDefectPoint>* out)
{
    if (!qty || pts.empty()) {
        return;
    }
    out->reserve(out->size() + pts.size());
    const float dthr = DeadThreshold(color, thr);
    for (const AlgoPoint& pt : pts) {
        PureColorDefectPoint p{};
        p.x = pt.x;
        p.y = pt.y;
        p.tempdata = AlgoMatGet(qty, pt.y, pt.x, 0);
        p.origindata = AlgoMatGet(qty, pt.y, pt.x, 1);
        p.comparedata = AlgoMatGet(qty, pt.y, pt.x, 2);
        p.dead_thread = dthr;
        out->push_back(p);
    }
}

// 从 MeterInspect 内部数据填充暗灯缺陷点。
void FillDarkPoints(PureDetectColor color,
                    const PureColorDetectThreshold& thr,
                    AlgoMat* qty,
                    const std::vector<AlgoPoint>& pts,
                    std::vector<PureColorDefectPoint>* out)
{
    if (!qty || pts.empty()) {
        return;
    }
    out->reserve(out->size() + pts.size());
    const float dthr = DarkThreshold(color, thr);
    for (const AlgoPoint& pt : pts) {
        PureColorDefectPoint p{};
        p.x = pt.x;
        p.y = pt.y;
        p.tempdata = AlgoMatGet(qty, pt.y, pt.x, 0);
        p.origindata = AlgoMatGet(qty, pt.y, pt.x, 1);
        p.comparedata = AlgoMatGet(qty, pt.y, pt.x, 2);
        p.dark_thread = dthr;
        out->push_back(p);
    }
}

// 从 MeterInspect 内部数据填充亮灯缺陷点。
void FillBrightPoints(PureDetectColor color,
                      const PureColorDetectThreshold& thr,
                      AlgoMat* qty,
                      const std::vector<AlgoPoint>& pts,
                      std::vector<PureColorDefectPoint>* out)
{
    if (!qty || pts.empty()) {
        return;
    }
    out->reserve(out->size() + pts.size());
    const float bthr = BrightThreshold(color, thr);
    for (const AlgoPoint& pt : pts) {
        PureColorDefectPoint p{};
        p.x = pt.x;
        p.y = pt.y;
        p.tempdata = AlgoMatGet(qty, pt.y, pt.x, 0);
        p.origindata = AlgoMatGet(qty, pt.y, pt.x, 1);
        p.comparedata = AlgoMatGet(qty, pt.y, pt.x, 2);
        p.bright_thread = bthr;
        out->push_back(p);
    }
}

void FillColorVariationPoints(const PureColorDetectParams& p,
                              AlgoMat* mat,
                              const std::vector<AlgoPoint>& pts,
                              std::vector<ColorVariationDefectPoint>* out)
{
    if (!mat || pts.empty()) {
        return;
    }
    out->reserve(out->size() + pts.size());
    for (const AlgoPoint& ap : pts) {
        ColorVariationDefectPoint point{};
        point.x = ap.x;
        point.y = ap.y;
        if (p.color == PureDetectColor::Red) {
            point.r_compare_data = AlgoMatGet(mat, ap.y, ap.x, 0);
            point.g_compare_data = AlgoMatGet(mat, ap.y, ap.x, 1);
            point.b_compare_data = AlgoMatGet(mat, ap.y, ap.x, 2);
            point.color_r_threshold = p.color_variation_detect_threshold.thr_color_variation_red_r;
            point.color_g_threshold = p.color_variation_detect_threshold.thr_color_variation_red_g;
            point.color_b_threshold = p.color_variation_detect_threshold.thr_color_variation_red_b;
        } else if (p.color == PureDetectColor::Green) {
            point.r_compare_data = AlgoMatGet(mat, ap.y, ap.x, 2);
            point.g_compare_data = AlgoMatGet(mat, ap.y, ap.x, 0);
            point.b_compare_data = AlgoMatGet(mat, ap.y, ap.x, 1);
            point.color_r_threshold = p.color_variation_detect_threshold.thr_color_variation_green_r;
            point.color_g_threshold = p.color_variation_detect_threshold.thr_color_variation_green_g;
            point.color_b_threshold = p.color_variation_detect_threshold.thr_color_variation_green_b;
        } else {
            point.r_compare_data = AlgoMatGet(mat, ap.y, ap.x, 2);
            point.g_compare_data = AlgoMatGet(mat, ap.y, ap.x, 1);
            point.b_compare_data = AlgoMatGet(mat, ap.y, ap.x, 0);
            point.color_r_threshold = p.color_variation_detect_threshold.thr_color_variation_blue_r;
            point.color_g_threshold = p.color_variation_detect_threshold.thr_color_variation_blue_g;
            point.color_b_threshold = p.color_variation_detect_threshold.thr_color_variation_blue_b;
        }
        out->push_back(point);
    }
}

// 导出亮度对比内部数据（tempdata / origindata / comparedata）为 CSV 文件。
AlgoError WriteLightQuantityCsvPlane(AlgoMat* qty, int plane_idx, const std::string& path)
{
    if (!qty || path.empty()) {
        return ALGO_PASS;
    }
    std::ofstream f(path.c_str(), std::ios::out | std::ios::trunc);
    if (!f) {
        return ALGO_INPUT_INVALID(ALGOMOD_ALGOMAT);
    }
    f << "";
    for (int j = 0; j < qty->cols; ++j) {
        f << ',' << (j + 1);
    }
    f << '\n';
    for (int i = 0; i < qty->rows; ++i) {
        f << (i + 1);
        for (int j = 0; j < qty->cols; ++j) {
            f << ',' << AlgoMatGet(qty, i, j, plane_idx);
        }
        f << '\n';
    }
    return ALGO_PASS;
}

bool HasPureCsvExport(const PureColorDetectParams& p)
{
    return !p.csv_pure_tempdata_file.empty()
        || !p.csv_pure_origindata_file.empty()
        || !p.csv_pure_comparedata_file.empty();
}

bool HasColorVariationCsvExport(const PureColorDetectParams& p)
{
    return !p.csv_color_red_comparedata_file.empty()
        || !p.csv_color_green_comparedata_file.empty()
        || !p.csv_color_blue_comparedata_file.empty();
}

AlgoError ExportLightQuantityCsv(const PureColorDetectParams& p, AlgoMat* qty)
{
    if (!qty) {
        return ALGO_PASS;
    }
    AlgoError e = WriteLightQuantityCsvPlane(qty, 0, p.csv_pure_tempdata_file);
    if (e != ALGO_PASS) {
        return e;
    }
    e = WriteLightQuantityCsvPlane(qty, 1, p.csv_pure_origindata_file);
    if (e != ALGO_PASS) {
        return e;
    }
    return WriteLightQuantityCsvPlane(qty, 2, p.csv_pure_comparedata_file);
}

AlgoError ExportColorVariationCsv(const PureColorDetectParams& p, AlgoMat* mat)
{
    if (!mat) {
        return ALGO_PASS;
    }
    auto write_plane = [&](const std::string& path, int idx) -> AlgoError {
        if (path.empty()) {
            return ALGO_PASS;
        }
        std::ofstream f(path.c_str(), std::ios::out | std::ios::trunc);
        if (!f) {
            return ALGO_INPUT_INVALID(ALGOMOD_ALGOMAT);
        }
        f << "";
        for (int j = 0; j < mat->cols; ++j) {
            f << "," << (j + 1);
        }
        f << "\n";
        for (int i = 0; i < mat->rows; ++i) {
            f << (i + 1);
            for (int j = 0; j < mat->cols; ++j) {
                f << "," << AlgoMatGet(mat, i, j, idx);
            }
            f << "\n";
        }
        return ALGO_PASS;
    };

    int r = 0;
    int g = 1;
    int b = 2;
    if (p.color == PureDetectColor::Green) {
        r = 2; g = 0; b = 1;
    } else if (p.color == PureDetectColor::Blue) {
        r = 2; g = 1; b = 0;
    }
    AlgoError e = write_plane(p.csv_color_red_comparedata_file, r);
    if (e != ALGO_PASS) return e;
    e = write_plane(p.csv_color_green_comparedata_file, g);
    if (e != ALGO_PASS) return e;
    return write_plane(p.csv_color_blue_comparedata_file, b);
}

common::errors::ErrorPtr RunRgbMeterForColorVariation(const PureColorDetectParams& p, RgbMeterResult* out)
{
    if (!out) {
        return common::errors::New(10, "PureColorDetect: RunRgbMeterForColorVariation output is null");
    }

    ShotAlgorithm shot_algo;
    MeterAlgorithm meter_algo;

    ShotRequest fg{};
    FillColorVariationShotRequest(p, p.cam_exposure, p.image, ToAlgoColor(p.color), false, &fg);
    fg.saveImage = p.save_original_image_flag;
    fg.imageFileName = p.normal_image_file;

    ShotResult foreground{};
    if (auto e = FailAlgo("ShotAlgorithm.run(color_variation foreground)", shot_algo.run(fg, &foreground))) return e;

    const bool reuse_pure_contour = !p.pure_contour_file_name.empty();

    auto build_channel_meter = [&](AlgoHandle shot) {
        MeterRequest req = BuildColorVariationMeterRequest(
            p,
            shot,
            reuse_pure_contour ? p.pure_contour_file_name : std::string{});
        req.contourReuse = reuse_pure_contour;
        req.saveContour = false;
        return req;
    };

    MeterRequest r_req = build_channel_meter(foreground.red.get());
    MeterRequest g_req = build_channel_meter(foreground.green.get());
    MeterRequest b_req = build_channel_meter(foreground.blue.get());

    if (auto e = FailAlgo("MeterAlgorithm.run(color_variation r)", meter_algo.run(r_req, &out->red))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(color_variation g)", meter_algo.run(g_req, &out->green))) return e;
    if (auto e = FailAlgo("MeterAlgorithm.run(color_variation b)", meter_algo.run(b_req, &out->blue))) return e;
    return nullptr;
}

common::errors::ErrorPtr RunColorVariationDetect(
    const PureColorDetectParams& p,
    AlgoHandle red_meter,
    AlgoHandle green_meter,
    AlgoHandle blue_meter,
    std::vector<ColorVariationDefectPoint>* color_variations)
{
    if (!color_variations) {
        return common::errors::New(10, "PureColorDetect: output color_variations is null");
    }

    MeterInspectGuard color_inspector;
    if (auto e = FailAlgo("MeterInspectCreateHandle(color_variation)", MeterInspectCreateHandle(color_inspector.out(), 0x4440))) return e;
    const MeterInspectThresh inspect_thresh = BuildColorVariationInspectThresh(p);
    if (auto e = FailAlgo("MeterInspectSetThresh(color_variation)", MeterInspectSetThresh(color_inspector.get(), &inspect_thresh))) return e;
    if (auto e = FailAlgo("MeterInspectColorVariation",
                          MeterInspectColorVariation(color_inspector.get(), red_meter, green_meter, blue_meter))) return e;

    const int max_pts = std::max(1, p.led_w * p.led_h);
    std::vector<AlgoPoint> list(static_cast<size_t>(max_pts));
    int n = max_pts;
    if (auto e = FailAlgo("MeterInspectGetResultList(color_variation)",
                          MeterInspectGetResultList(color_inspector.get(), LED_DEFECT_FLAG_WRONG_COLOR, list.data(), &n, LED_DEFECT_FLAG_UNINIT))) return e;
    n = std::max(0, n);
    list.resize(static_cast<size_t>(n));

    AlgoMatGuard color_inner;
    color_inner.reset(AlgoMatNew());
    if (auto e = FailAlgo("MeterInspectGetInnerData(color_variation)",
                          MeterInspectGetInnerData(color_inspector.get(), color_inner.get(), 1))) return e;

    color_variations->clear();
    color_variations->reserve(list.size());
    FillColorVariationPoints(p, color_inner.get(), list, color_variations);

    if (!p.color_variation_png_file_name.empty()) {
        AlgoHandle export_meter = (p.color == PureDetectColor::Red) ? red_meter
                               : (p.color == PureDetectColor::Green) ? green_meter
                               : blue_meter;
        if (auto e = FailAlgo("LedMeterExportChromaImage(color_variation)",
                              LedMeterExportChromaImage(export_meter, p.color_variation_png_file_name.c_str(), 1, 11, list.data(), n))) return e;
    }
    if (HasColorVariationCsvExport(p)) {
        if (auto e = FailAlgo("ExportColorVariationCsv", ExportColorVariationCsv(p, color_inner.get()))) return e;
    }
    return nullptr;
}

// 对 Meter 结果执行纯色缺陷检测（死灯 / 暗灯 / 亮灯），并填充输出列表。
common::errors::ErrorPtr RunPureColorInspect(const PureColorDetectParams& p,
                                             AlgoHandle meter_handle,
                                             std::vector<PureColorDefectPoint>* deads,
                                             std::vector<PureColorDefectPoint>* darks,
                                             std::vector<PureColorDefectPoint>* brights)
{
    MeterInspectGuard inspector;
    AlgoError err = MeterInspectCreateHandle(inspector.out(), 0x4440);
    if (auto e = FailAlgo("MeterInspectCreateHandle", err)) {
        return e;
    }

    MeterInspectThresh thresh = BuildMeterInspectThresh(p.detect_threshold);
    err = MeterInspectSetThresh(inspector.get(), &thresh);
    if (auto fe = FailAlgo("MeterInspectSetThresh", err)) {
        return fe;
    }

    // 亮灯检测须优先执行，以便获取亮灯内部数据。
    AlgoMatGuard inner_bright;
    if (p.bright_check) {
        err = MeterInspectBrightLed(inspector.get(), meter_handle);
        if (auto fe = FailAlgo("MeterInspectBrightLed", err)) {
            return fe;
        }
        inner_bright.reset(AlgoMatNew());
        err = MeterInspectGetInnerData(inspector.get(), inner_bright.get(), 1);
        if (auto fe = FailAlgo("MeterInspectGetInnerData(bright)", err)) {
            return fe;
        }
    }

    if (p.dead_check) {
        err = MeterInspectDeadLed(inspector.get(), meter_handle);
        if (auto fe = FailAlgo("MeterInspectDeadLed", err)) {
            return fe;
        }
    }

    if (p.dark_check) {
        err = MeterInspectDarkLed(inspector.get(), meter_handle);
        if (auto fe = FailAlgo("MeterInspectDarkLed", err)) {
            return fe;
        }
    }

    AlgoMatGuard inner_dead_dark;
    if (p.dead_check || p.dark_check || HasPureCsvExport(p)) {
        inner_dead_dark.reset(AlgoMatNew());
        err = MeterInspectGetInnerData(inspector.get(), inner_dead_dark.get(), 1);
        if (auto fe = FailAlgo("MeterInspectGetInnerData(dead/dark/export)", err)) {
            return fe;
        }
    }

    const int max_pts = std::max(1, p.led_w * p.led_h);

    // 收集死灯缺陷点，可选导出结果图。
    if (p.dead_check) {
        std::vector<AlgoPoint> buf(static_cast<size_t>(max_pts));
        int n = max_pts;
        err = MeterInspectGetResultList(
            inspector.get(), LED_DEFECT_FLAG_DEAD, buf.data(), &n, LED_DEFECT_FLAG_UNINIT);
        if (auto fe = FailAlgo("MeterInspectGetResultList(dead)", err)) {
            return fe;
        }
        buf.resize(static_cast<size_t>(std::max(0, n)));

        if (!p.dead_png_file.empty()) {
            err = LedMeterExportChromaImage(meter_handle, p.dead_png_file.c_str(), 1, 11, buf.data(), n);
            if (auto fe = FailAlgo("LedMeterExportChromaImage(dead)", err)) {
                return fe;
            }
        }
        FillDeadPoints(p.color, p.detect_threshold, inner_dead_dark.get(), buf, deads);
    }

    // 收集暗灯缺陷点，可选导出结果图。
    if (p.dark_check) {
        std::vector<AlgoPoint> buf(static_cast<size_t>(max_pts));
        int n = max_pts;
        err = MeterInspectGetResultList(
            inspector.get(), LED_DEFECT_FLAG_DIM, buf.data(), &n, LED_DEFECT_FLAG_UNINIT);
        if (auto fe = FailAlgo("MeterInspectGetResultList(dim)", err)) {
            return fe;
        }
        buf.resize(static_cast<size_t>(std::max(0, n)));

        if (!p.dark_png_file.empty()) {
            err = LedMeterExportChromaImage(meter_handle, p.dark_png_file.c_str(), 1, 11, buf.data(), n);
            if (auto fe = FailAlgo("LedMeterExportChromaImage(dark)", err)) {
                return fe;
            }
        }
        FillDarkPoints(p.color, p.detect_threshold, inner_dead_dark.get(), buf, darks);
    }

    // 收集亮灯缺陷点，可选导出结果图。
    if (p.bright_check) {
        std::vector<AlgoPoint> buf(static_cast<size_t>(max_pts));
        int n = max_pts;
        err = MeterInspectGetResultList(
            inspector.get(), LED_DEFECT_FLAG_BRIGHT, buf.data(), &n, LED_DEFECT_FLAG_UNINIT);
        if (auto fe = FailAlgo("MeterInspectGetResultList(bright)", err)) {
            return fe;
        }
        buf.resize(static_cast<size_t>(std::max(0, n)));

        if (!p.bright_png_file.empty()) {
            err = LedMeterExportChromaImage(meter_handle, p.bright_png_file.c_str(), 1, 11, buf.data(), n);
            if (auto fe = FailAlgo("LedMeterExportChromaImage(bright)", err)) {
                return fe;
            }
        }
        FillBrightPoints(p.color, p.detect_threshold, inner_bright.get(), buf, brights);
    }

    // 可选：导出亮度对比 CSV。
    if (HasPureCsvExport(p)) {
        err = ExportLightQuantityCsv(p, inner_dead_dark.get());
        if (auto fe = FailAlgo("ExportLightQuantityCsv", err)) {
            return fe;
        }
    }

    return nullptr;
}

} // namespace

std::string PureColorDetectAlgorithm::Name() const
{
    return "PureColorDetect";
}

std::string PureColorDetectAlgorithm::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr PureColorDetectAlgorithm::Execute(AlgorithmExecuteData& data)
{
    const PureColorDetectParams* params = data.inputs.Get<PureColorDetectParams>("params");
    if (!params || !params->image || params->image->Empty()) {
        return common::errors::New(1, "PureColorDetect: missing inputs.params.image");
    }
    if (params->led_w <= 0 || params->led_h <= 0) {
        return common::errors::New(2, "PureColorDetect: led_w/led_h must be positive");
    }

    const bool run_pure_inspect = params->dead_check || params->dark_check || params->bright_check;
    std::vector<PureColorDefectPoint> deads;
    std::vector<PureColorDefectPoint> darks;
    std::vector<PureColorDefectPoint> brights;

    // 1. 纯色 Shot + Meter：有轮廓路径时点定位并落盘；否则仅执行缺陷检测所需的 Meter。
    const PureColorDetectParams& meter_params = *params;
    const bool need_pure_meter = run_pure_inspect || !meter_params.pure_contour_file_name.empty();
    MeterResult meter{};
    if (need_pure_meter) {
        ShotMeter shot_meter;
        if (auto e = shot_meter.RunPureColor(meter_params, &meter)) {
            return e;
        }

        if (run_pure_inspect) {
            if (auto inspect_err = RunPureColorInspect(*params, meter.meter.get(), &deads, &darks, &brights)) {
                return inspect_err;
            }
        }
    }

    // 2. 串色检测：复用纯色 contour，对 R/G/B 三通道执行 Meter 并检测错色缺陷。
    std::vector<ColorVariationDefectPoint> color_variations;
    if (params->color_check) {
        RgbMeterResult rgb_meter{};
        if (auto e = RunRgbMeterForColorVariation(*params, &rgb_meter)) {
            return e;
        }
        if (auto e = RunColorVariationDetect(
                *params,
                rgb_meter.red.meter.get(),
                rgb_meter.green.meter.get(),
                rgb_meter.blue.meter.get(),
                &color_variations)) {
            return e;
        }
    }

    data.outputs.Set("deads", std::move(deads));
    data.outputs.Set("darks", std::move(darks));
    data.outputs.Set("brights", std::move(brights));
    data.outputs.Set("color_variations", std::move(color_variations));
    return nullptr;
}

REGISTER_ALGORITHM("PureColorDetect", PureColorDetectAlgorithm)

} // namespace algorithm
} // namespace common
