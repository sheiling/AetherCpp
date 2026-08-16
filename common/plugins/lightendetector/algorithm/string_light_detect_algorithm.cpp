/**
 * @file string_light_detect_algorithm.cpp
 * @brief 串灯检测算法实现。
 *
 * 对左右两幅图像分别执行 Shot + Meter，通过 MeterInspect 沿斜向比对
 * map_live / map_dead 检测串灯缺陷并写入 string_lights。
 */

#include "string_light_detect_algorithm.h"

#include <common/plugins/lightendetector/service/algo_sdk_guard.h>
#include <common/plugins/lightendetector/service/shot_meter.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/3rdparty/cisdk/measurement/include/algo_led_meter.h>
#include <common/3rdparty/cisdk/measurement/include/algo_mat.h>
#include <common/utility/errors/include/error.h>

#include <algorithm>
#include <fstream>
#include <vector>

namespace common {
namespace algorithm {
namespace {

// 按检测颜色通道取 map_live 阈值。
float GetMapLiveThreshold(PureDetectColor c, const StringLightDetectThreshold& t)
{
    switch (c) {
    case PureDetectColor::Red: return t.thr_string_light_live_r;
    case PureDetectColor::Green: return t.thr_string_light_live_g;
    case PureDetectColor::Blue: return t.thr_string_light_live_b;
    }
    return 0.f;
}

// 按检测颜色通道取 map_dead 阈值。
float GetMapDeadThreshold(PureDetectColor c, const StringLightDetectThreshold& t)
{
    switch (c) {
    case PureDetectColor::Red: return t.thr_string_light_dead_r;
    case PureDetectColor::Green: return t.thr_string_light_dead_g;
    case PureDetectColor::Blue: return t.thr_string_light_dead_b;
    }
    return 0.f;
}

// 将左右比对数据导出为 CSV。
AlgoError ExportStringLightCsv(const StringLightDetectParams& p, AlgoMat* mat)
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

    AlgoError e = write_plane(p.csv_left_comparedata_file_name, 0);
    if (e != ALGO_PASS) return e;
    return write_plane(p.csv_right_comparedata_file_name, 1);
}

// 基于左右 Meter 执行串灯检测，输出 map_live / map_dead 缺陷点。
common::errors::ErrorPtr RunStringLightInspect(
    const StringLightDetectParams& p,
    const PairMeterResult& meter_pair,
    std::vector<StringLightDefectPoint>* out)
{
    if (!out) {
        return common::errors::New(10, "StringLightDetect: output string_lights is null");
    }

    MeterInspectGuard inspector;
    if (auto e = FailAlgo("MeterInspectCreateHandle", MeterInspectCreateHandle(inspector.out(), 0x4440))) return e;

    // 1. 设置串灯检测阈值。
    MeterInspectThresh thresh{};
    thresh.thr_mapLive_R = p.detect_threshold.thr_string_light_live_r;
    thresh.thr_mapLive_G = p.detect_threshold.thr_string_light_live_g;
    thresh.thr_mapLive_B = p.detect_threshold.thr_string_light_live_b;
    thresh.thr_mapDead_R = p.detect_threshold.thr_string_light_dead_r;
    thresh.thr_mapDead_G = p.detect_threshold.thr_string_light_dead_g;
    thresh.thr_mapDead_B = p.detect_threshold.thr_string_light_dead_b;
    thresh.weight_val = 1.0f;
    thresh.temp_size = 11;

    if (auto e = FailAlgo("MeterInspectSetThresh", MeterInspectSetThresh(inspector.get(), &thresh))) return e;
    // 2. 沿左斜线方向比对左右 Meter，检测串灯。
    if (auto e = FailAlgo("MeterInspectLedMap", MeterInspectLedMap(inspector.get(), meter_pair.first.meter.get(), meter_pair.second.meter.get(), LEDMAP_LEFT_SLASH_LINE, 2))) return e;

    // 3. 获取 map_live / map_dead 缺陷点列表。
    const int max_pts = std::max(1, p.led_w * p.led_h);
    std::vector<AlgoPoint> live_pts(static_cast<size_t>(max_pts));
    std::vector<AlgoPoint> dead_pts(static_cast<size_t>(max_pts));
    int live_n = max_pts;
    int dead_n = max_pts;

    if (auto e = FailAlgo("MeterInspectGetResultList(map_live)",
                          MeterInspectGetResultList(inspector.get(), LED_DEFECT_FLAG_MAP_LIVE, live_pts.data(), &live_n, LED_DEFECT_FLAG_UNINIT))) return e;
    if (auto e = FailAlgo("MeterInspectGetResultList(map_dead)",
                          MeterInspectGetResultList(inspector.get(), LED_DEFECT_FLAG_MAP_DEAD, dead_pts.data(), &dead_n, LED_DEFECT_FLAG_UNINIT))) return e;
    live_pts.resize(static_cast<size_t>(std::max(0, live_n)));
    dead_pts.resize(static_cast<size_t>(std::max(0, dead_n)));

    // 4. 可选：导出缺陷点位标注结果图。
    if (!p.string_live_file_name.empty()) {
        if (auto e = FailAlgo("LedMeterExportChromaImage(map_live)",
                              LedMeterExportChromaImage(meter_pair.first.meter.get(), p.string_live_file_name.c_str(), 1, 11, live_pts.data(), live_n))) return e;
    }
    if (!p.string_dead_file_name.empty()) {
        if (auto e = FailAlgo("LedMeterExportChromaImage(map_dead)",
                              LedMeterExportChromaImage(meter_pair.first.meter.get(), p.string_dead_file_name.c_str(), 1, 11, dead_pts.data(), dead_n))) return e;
    }

    // 5. 获取内部比对数据，可选导出 CSV。
    AlgoMatGuard light_qty;
    light_qty.reset(AlgoMatNew());
    if (auto e = FailAlgo("MeterInspectGetInnerData", MeterInspectGetInnerData(inspector.get(), light_qty.get(), 1))) return e;
    if (!p.csv_left_comparedata_file_name.empty() || !p.csv_right_comparedata_file_name.empty()) {
        if (auto e = FailAlgo("ExportStringLightCsv", ExportStringLightCsv(p, light_qty.get()))) return e;
    }

    // 6. 组装缺陷点输出（含左右比对值与阈值）。
    out->clear();
    out->reserve(live_pts.size() + dead_pts.size());
    const float live_thr = GetMapLiveThreshold(p.color, p.detect_threshold);
    const float dead_thr = GetMapDeadThreshold(p.color, p.detect_threshold);

    for (const auto& pt : live_pts) {
        out->push_back({
            pt.x,
            pt.y,
            static_cast<float>(AlgoMatGet(light_qty.get(), pt.y, pt.x, 0)),
            static_cast<float>(AlgoMatGet(light_qty.get(), pt.y, pt.x, 1)),
            live_thr,
            dead_thr,
            true});
    }
    for (const auto& pt : dead_pts) {
        out->push_back({
            pt.x,
            pt.y,
            static_cast<float>(AlgoMatGet(light_qty.get(), pt.y, pt.x, 0)),
            static_cast<float>(AlgoMatGet(light_qty.get(), pt.y, pt.x, 1)),
            live_thr,
            dead_thr,
            false});
    }
    return nullptr;
}

} // namespace

std::string StringLightDetectAlgorithm::Name() const
{
    return "StringLightDetect";
}

std::string StringLightDetectAlgorithm::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr StringLightDetectAlgorithm::Execute(AlgorithmExecuteData& data)
{
    const auto* p = data.inputs.Get<StringLightDetectParams>("params");
    if (!p || !p->left_image || p->left_image->Empty() || !p->right_image || p->right_image->Empty()) {
        return common::errors::New(1, "StringLightDetect: missing left/right image");
    }
    if (p->led_w <= 0 || p->led_h <= 0) {
        return common::errors::New(2, "StringLightDetect: led_w/led_h must be positive");
    }

    // 1. 对左右图像执行 Shot + Meter（复用纯色 contour 或 ROI 点定位）。
    ShotMeter shot_meter;
    PairMeterResult meter_pair{};
    if (auto e = shot_meter.RunStringLight(*p, &meter_pair)) return e;

    // 2. 执行串灯检测并输出缺陷点列表。
    std::vector<StringLightDefectPoint> out;
    auto inspect_err = RunStringLightInspect(*p, meter_pair, &out);
    if (inspect_err) {
        return inspect_err;
    }

    data.outputs.Set("string_lights", std::move(out));
    return nullptr;
}

REGISTER_ALGORITHM("StringLightDetect", StringLightDetectAlgorithm)

} // namespace algorithm
} // namespace common
