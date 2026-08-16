/**
 * @file recipe_step_debug_common.cpp
 * @brief 产品配方单步调试共用实现。
 */

#include "recipe_step_debug_common.h"

#include "lighten_param_persist.h"
#include "messagecenter/message_center.h"

#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/device/displayControl/software_screen/software_screen_base.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include <cstring>
#include <thread>

namespace lighten {

using common::algorithm::PureColorDetectParams;
using common::algorithm::PureDetectColor;
using common::algorithm::PureColorDefectPoint;
using common::algorithm::ColorVariationDefectPoint;

/** 单步调试落盘路径统一使用反斜杠（CISDK / Windows fopen 本地路径约定）。 */
std::string normalizeStepDebugPathSeparators(std::string path)
{
    for (char& c : path) {
        if (c == '/') {
            c = '\\';
        }
    }
    return path;
}

std::string joinStepDebugOutputPath(const std::string& dir, const std::string& fileName)
{
    if (dir.empty()) {
        return normalizeStepDebugPathSeparators(fileName);
    }
    if (fileName.empty()) {
        return normalizeStepDebugPathSeparators(dir);
    }
    std::string path = dir;
    if (path.back() != '\\' && path.back() != '/') {
        path += '\\';
    }
    path += fileName;
    return normalizeStepDebugPathSeparators(path);
}

QString algoThresholdNameAt(int index)
{
    static const QStringList kNames = {
        QStringLiteral("死灯阈值"), QStringLiteral("亮灯阈值"), QStringLiteral("暗灯阈值"),
        QStringLiteral("串灯阈值"), QStringLiteral("串色R"),   QStringLiteral("串色G"),
        QStringLiteral("串色B"),   QStringLiteral("混色初筛"), QStringLiteral("混色筛选"),
    };
    return kNames.value(index);
}

RgbTriple algoRowValuesAt(const AlgoThresholds& algo, int index)
{
    const QString name = algoThresholdNameAt(index);
    for (const AlgoThresholdRow& row : algo.rows) {
        if (row.name == name)
            return row.values;
    }
    return {};
}

float rgbChannelValue(const RgbTriple& rgb, size_t channelIdx)
{
    switch (channelIdx) {
    case 0:
        return static_cast<float>(rgb.r);
    case 1:
        return static_cast<float>(rgb.g);
    case 2:
        return static_cast<float>(rgb.b);
    default:
        return 0.f;
    }
}

size_t colorChannelIndex(ColorChannel ch)
{
    switch (ch) {
    case ColorChannel::Green:
        return 1;
    case ColorChannel::Blue:
        return 2;
    default:
        return 0;
    }
}

PureDetectColor toPureDetectColor(ColorChannel ch)
{
    return static_cast<PureDetectColor>(colorChannelIndex(ch));
}

int cameraIndexFromName(const QString& cameraName)
{
    const std::vector<std::string> keys = DeviceConfigReader::loadCameraKeys();
    const std::string name = cameraName.toStdString();
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == name) {
            return static_cast<int>(i);
        }
    }
    return 0;
}

int pureChannelCaptureExposure(const RecipeStepDebugImageContext& ctx, size_t channelIdx)
{
    const int cameraIndex = cameraIndexFromName(ctx.cameraName);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < ctx.recipe.cameras.size())
                                    ? ctx.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};
    return static_cast<int>(rgbChannelValue(cam.pureExposureMs, channelIdx));
}

int seriesChannelCaptureExposure(const RecipeStepDebugImageContext& ctx, size_t channelIdx)
{
    const int cameraIndex = cameraIndexFromName(ctx.cameraName);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < ctx.recipe.cameras.size())
                                    ? ctx.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};
    return static_cast<int>(rgbChannelValue(cam.seriesExposureMs, channelIdx));
}

void applyRoiToParams(PureColorDetectParams& pure, const RecipeDebugToolbar& dbg)
{
    if (!dbg.roiEnabled || dbg.rois.size() < 8) {
        pure.use_auto_roi = true;
        return;
    }

    pure.use_auto_roi = false;
    pure.left_top     = {static_cast<int>(dbg.rois[0]), static_cast<int>(dbg.rois[1])};
    pure.right_top    = {static_cast<int>(dbg.rois[2]), static_cast<int>(dbg.rois[3])};
    pure.right_bottom = {static_cast<int>(dbg.rois[4]), static_cast<int>(dbg.rois[5])};
    pure.left_bottom  = {static_cast<int>(dbg.rois[6]), static_cast<int>(dbg.rois[7])};
}

void applyRoiToStringLightParams(common::algorithm::StringLightDetectParams& params,
                                 const RecipeDebugToolbar& dbg)
{
    if (!dbg.roiEnabled || dbg.rois.size() < 8) {
        params.use_auto_roi = true;
        return;
    }

    params.use_auto_roi = false;
    params.left_top     = {static_cast<int>(dbg.rois[0]), static_cast<int>(dbg.rois[1])};
    params.right_top    = {static_cast<int>(dbg.rois[2]), static_cast<int>(dbg.rois[3])};
    params.right_bottom = {static_cast<int>(dbg.rois[4]), static_cast<int>(dbg.rois[5])};
    params.left_bottom  = {static_cast<int>(dbg.rois[6]), static_cast<int>(dbg.rois[7])};
}

void fillPureColorAndColorVariationThresholds(PureColorDetectParams& pure,
                                              size_t channelIdx,
                                              const AlgoThresholds& algo)
{
    const RgbTriple deadVals   = algoRowValuesAt(algo, 0);
    const RgbTriple brightVals = algoRowValuesAt(algo, 1);
    const RgbTriple dimVals    = algoRowValuesAt(algo, 2);
    const RgbTriple cvVals     = algoRowValuesAt(algo, 4 + static_cast<int>(channelIdx));

    auto& thr = pure.detect_threshold;
    auto& cvt = pure.color_variation_detect_threshold;

    if (channelIdx == 0) {
        thr.thr_dead_R = rgbChannelValue(deadVals, 0);
        thr.thr_dim_max_R = rgbChannelValue(dimVals, 0);
        thr.thr_bright_R = rgbChannelValue(brightVals, 0);
    } else if (channelIdx == 1) {
        thr.thr_dead_G = rgbChannelValue(deadVals, 1);
        thr.thr_dim_max_G = rgbChannelValue(dimVals, 1);
        thr.thr_bright_G = rgbChannelValue(brightVals, 1);
    } else {
        thr.thr_dead_B = rgbChannelValue(deadVals, 2);
        thr.thr_dim_max_B = rgbChannelValue(dimVals, 2);
        thr.thr_bright_B = rgbChannelValue(brightVals, 2);
    }

    cvt.thr_color_variation_red_r   = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_red_g   = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_red_b   = static_cast<float>(cvVals.b);
    cvt.thr_color_variation_green_r = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_green_g = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_green_b = static_cast<float>(cvVals.b);
    cvt.thr_color_variation_blue_r  = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_blue_g  = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_blue_b  = static_cast<float>(cvVals.b);
}

void fillStringLightThresholds(common::algorithm::StringLightDetectParams& stringParams,
                               size_t channelIdx,
                               const AlgoThresholds& algo)
{
    const RgbTriple vals = algoRowValuesAt(algo, 3);
    const float thr      = rgbChannelValue(vals, channelIdx);
    auto& st             = stringParams.detect_threshold;
    if (channelIdx == 0) {
        st.thr_string_light_live_r = thr;
        st.thr_string_light_dead_r = thr;
    } else if (channelIdx == 1) {
        st.thr_string_light_live_g = thr;
        st.thr_string_light_dead_g = thr;
    } else {
        st.thr_string_light_live_b = thr;
        st.thr_string_light_dead_b = thr;
    }
}

void notifyStepDebugPureColorResults(int colorIdx,
                                     const std::vector<PureColorDefectPoint>* deads,
                                     const std::vector<PureColorDefectPoint>* darks,
                                     const std::vector<PureColorDefectPoint>* brights,
                                     const std::vector<ColorVariationDefectPoint>* color_variations)
{
    if (colorIdx < 0 || colorIdx > 2) {
        return;
    }
    const auto notify = [&](DetectItemKind kind, size_t count) {
        MessageCenter::Center::Notify(
            EVENT_DEBUG_NORMAL_DEFECT_COUNT,
            DEBUG_NORMAL_DEFECT_COUNT_NOTIFY_ARGS(kind, colorIdx, static_cast<int>(count)));
    };
    if (deads) {
        notify(DetectItemKind::DeadLed, deads->size());
    }
    if (darks) {
        notify(DetectItemKind::DimLed, darks->size());
    }
    if (brights) {
        notify(DetectItemKind::BrightLed, brights->size());
    }
    if (color_variations) {
        notify(DetectItemKind::ColorVariation, color_variations->size());
    }
}

void notifyStepDebugStringLightResults(
    int colorIdx,
    const std::vector<common::algorithm::StringLightDefectPoint>* string_lights)
{
    if (colorIdx < 0 || colorIdx > 2 || !string_lights) {
        return;
    }
    size_t mapDeadCount = 0;
    size_t mapLiveCount = 0;
    for (const auto& pt : *string_lights) {
        if (pt.is_map_live)
            ++mapLiveCount;
        else
            ++mapDeadCount;
    }
    MessageCenter::Center::Notify(
        EVENT_DEBUG_NORMAL_DEFECT_COUNT,
        DEBUG_NORMAL_DEFECT_COUNT_NOTIFY_ARGS(DetectItemKind::ColorMapDead,
                                              colorIdx,
                                              static_cast<int>(mapDeadCount)));
    MessageCenter::Center::Notify(
        EVENT_DEBUG_NORMAL_DEFECT_COUNT,
        DEBUG_NORMAL_DEFECT_COUNT_NOTIFY_ARGS(DetectItemKind::ColorMapLive,
                                              colorIdx,
                                              static_cast<int>(mapLiveCount)));
}

QString sampleCameraOutputDirPath(const QString& cameraPath)
{
    const QString base = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("sample"));
    const QString dir  = QDir::toNativeSeparators(QDir(base).filePath(cameraPath));
    QDir().mkpath(dir);
    return dir;
}

std::string sampleCameraOutputDirLocal8(const QString& cameraPath)
{
    return normalizeStepDebugPathSeparators(
        sampleCameraOutputDirPath(cameraPath).toLocal8Bit().toStdString());
}

std::shared_ptr<common::algorithm::ImageBuffer> loadRawFileToImageBuffer(
    const QString& filePath,
    const RecipeStepDebugImageContext& ctx)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    const QByteArray rawData = file.readAll();
    if (rawData.isEmpty()) {
        return nullptr;
    }

    const int width    = ctx.xResolution;
    const int height   = ctx.yResolution;
    const int elemSize = ctx.elemSize;
    if (width <= 0 || height <= 0 || elemSize <= 0) {
        return nullptr;
    }

    const int channels = 1;
    const int minRowBytes = width * channels * elemSize;
    if (minRowBytes <= 0 || rawData.size() < minRowBytes * height) {
        return nullptr;
    }

    auto buf = common::algorithm::ImageBuffer::Create(width, height, channels, elemSize);
    if (!buf || !buf->data) {
        return nullptr;
    }

    const auto* src = reinterpret_cast<const unsigned char*>(rawData.constData());
    std::memcpy(buf->data.get(), src, buf->ByteSize());
    return buf;
}

namespace {

common::errors::ErrorPtr grabRgbBuffer(
    const std::shared_ptr<common::device::DeviceManager>& dm,
    const std::string& camera_name,
    int exposure_us,
    int elemSize,
    std::shared_ptr<common::algorithm::ImageBuffer>* out_image)
{
    if (!dm) {
        return common::errors::New(-1, "DeviceManager is null");
    }

    auto cam = dm->CameraHandler(camera_name);
    if (!cam) {
        return common::errors::New(-3, "Camera not found: " + camera_name);
    }
    if (!cam->IsConnected()) {
        return common::errors::New(-4, "Camera disconnected: " + camera_name);
    }

    if (exposure_us > 0) {
        if (auto ee = cam->SetExposureTime(exposure_us)) {
            return ee;
        }
    }

    int h = 0;
    int w = 0;
    unsigned char* sdk_buf = nullptr;
    int len = 0;
    if (auto ge = cam->SyncGrab(h, w, sdk_buf, len)) {
        return ge;
    }
    if (!sdk_buf || len <= 0 || h <= 0 || w <= 0) {
        return common::errors::New(-5, "SyncGrab returned empty buffer");
    }

    const int channels = 1;
    auto img = common::algorithm::ImageBuffer::Create(w, h, channels, elemSize);
    const size_t cap = img->ByteSize();
    const size_t n   = static_cast<size_t>(len);
    const size_t copy_n = (cap < n) ? cap : n;
    std::memcpy(img->data.get(), sdk_buf, copy_n);

    *out_image = std::move(img);
    return nullptr;
}

common::device::SoftScreenColor toSoftScreenColor(ColorChannel ch)
{
    switch (ch) {
    case ColorChannel::Green:
        return common::device::COLOR_GREEN;
    case ColorChannel::Blue:
        return common::device::COLOR_BLUE;
    case ColorChannel::Black:
        return common::device::COLOR_BLACK;
    case ColorChannel::White:
        return common::device::COLOR_WHITE;
    default:
        return common::device::COLOR_RED;
    }
}

} // namespace

common::errors::ErrorPtr grabRgbBufferForStepDebug(
    const RecipeStepDebugImageContext& ctx,
    int exposureMs,
    std::shared_ptr<common::algorithm::ImageBuffer>* out_image)
{
    if (!out_image) {
        return common::errors::New(-1, "grabRgbBufferForStepDebug: out_image is null");
    }

    const auto dm = tryGetDeviceManager();
    if (!dm) {
        return common::errors::New(-2, "DeviceManager unavailable");
    }

    return grabRgbBuffer(
        dm,
        ctx.cameraName.toStdString(),
        exposureMs,
        ctx.elemSize,
        out_image);
}

common::errors::ErrorPtr softDisplayForStepDebug(const RecipeStepDebugSoftDisplayParams& params)
{
    const auto dm = tryGetDeviceManager();
    if (!dm) {
        return common::errors::New(-1, "DeviceManager unavailable");
    }

    const std::string softScreenKey = params.softScreenKey.toStdString();
    if (softScreenKey.empty()) {
        return common::errors::New(-2, "SoftScreen key is empty");
    }

    auto screen = dm->SoftScreenHandler(softScreenKey);
    if (!screen) {
        return common::errors::New(-3, "SoftScreen handler missing: " + softScreenKey);
    }

    common::device::SoftScreenParam screenParam{};
    screenParam.color      = toSoftScreenColor(params.colorChannel);
    screenParam.mode       = params.displayMode;
    screenParam.brightness = params.brightness;
    screenParam.interval   = 1;
    screenParam.width      = params.panelWidth;
    screenParam.height     = params.panelHeight;

    screen->Display(screenParam);
    if (params.dispDelayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(params.dispDelayMs));
    }
    return nullptr;
}

void applyStepDebugOutputPaths(PureColorDetectParams& p,
                               const std::string& cameraDir,
                               bool pureColorOutputs,
                               bool colorVariationOutputs)
{
    p.adjustFileName();
    p.save_original_image_flag = true;

    const std::string dir = normalizeStepDebugPathSeparators(cameraDir);
    const auto join = [&](const std::string& fileName) {
        return joinStepDebugOutputPath(dir, fileName);
    };

    p.normal_image_file      = join(p.normal_image_file);
    p.pure_contour_file_name = join(p.pure_contour_file_name);

    if (pureColorOutputs) {
        p.dead_png_file             = join(p.dead_png_file);
        p.dark_png_file             = join(p.dark_png_file);
        p.bright_png_file           = join(p.bright_png_file);
        p.csv_pure_tempdata_file    = join(p.csv_pure_tempdata_file);
        p.csv_pure_origindata_file  = join(p.csv_pure_origindata_file);
        p.csv_pure_comparedata_file = join(p.csv_pure_comparedata_file);
    }

    if (colorVariationOutputs) {
        p.color_variation_png_file_name    = join(p.color_variation_png_file_name);
        p.csv_color_red_comparedata_file   = join(p.csv_color_red_comparedata_file);
        p.csv_color_green_comparedata_file = join(p.csv_color_green_comparedata_file);
        p.csv_color_blue_comparedata_file  = join(p.csv_color_blue_comparedata_file);
    }
}

void applyStepDebugStringLightOutputPaths(common::algorithm::StringLightDetectParams& p,
                                          const std::string& cameraDir)
{
    p.adjustFileName();
    p.save_original_image_flag = true;
    p.save_result_image_flag   = true;

    const std::string dir = normalizeStepDebugPathSeparators(cameraDir);
    const auto join = [&](const std::string& fileName) {
        return joinStepDebugOutputPath(dir, fileName);
    };

    p.normal_left_image_file_name     = join(p.normal_left_image_file_name);
    p.normal_right_image_file_name    = join(p.normal_right_image_file_name);
    p.string_live_file_name           = join(p.string_live_file_name);
    p.string_dead_file_name           = join(p.string_dead_file_name);
    p.csv_left_comparedata_file_name  = join(p.csv_left_comparedata_file_name);
    p.csv_right_comparedata_file_name = join(p.csv_right_comparedata_file_name);
}

} // namespace lighten
