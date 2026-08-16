#pragma once

/**
 * @file recipe_step_debug_common.h
 * @brief 产品配方单步调试共用：缺陷数量通知、阈值读取、采图/读 RAW、sample 落盘路径。
 */

#include "recipe_model.h"
#include "lighten_types.h"

#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/core/device/displayControl/software_screen/software_screen_base.h>
#include <common/utility/errors/include/error.h>

#include <QString>

#include <memory>
#include <vector>

namespace lighten {

/**
 * 配方单步调试缺陷数量通知（MessageCenter::Center::Notify）。
 *
 * 订阅方：RecipeConfigView::slot_debug_pure_color_defect_count（产品配方配置 → 结果输出区域）。
 * 参数约定（QVariantList，共 3 项）：
 *   [0] DetectItemKind — 检测项（死灯/暗灯/亮灯/串色等）
 *   [1] int            — RGB 颜色下标：R=0，G=1，B=2（与 PureDetectColor::Red/Green/Blue 一致）
 *   [2] int            — 该检测项在该颜色通道下的缺陷点数量
 */
#define EVENT_DEBUG_NORMAL_DEFECT_COUNT "event-debug-normal-defect-count"
#define DEBUG_NORMAL_DEFECT_COUNT_NOTIFY_ARGS(item_kind, color_idx, count) \
    static_cast<int>(item_kind), static_cast<int>(color_idx), static_cast<int>(count)

/** 单步调试打屏参数（由 UI 从配方页采集后填入）。 */
struct RecipeStepDebugSoftDisplayParams
{
    QString                        softScreenKey;
    ColorChannel                   colorChannel  = ColorChannel::Red;
    common::device::SoftScreenMode displayMode   = common::device::FULL_MODE;
    int                            panelWidth    = 0;
    int                            panelHeight   = 0;
    int                            brightness    = 255;
    int                            dispDelayMs   = 0;
};

/** 单步调试采图/读图上下文（纯色/串色/串灯检测共用字段） */
struct RecipeStepDebugImageContext
{
    RecipeDocument     recipe;
    QString            cameraName;
    QString            cameraPath;
    int                xResolution = 0;
    int                yResolution = 0;
    int                elemSize    = 0;
    QString            rawImagePath;
    RecipeDebugToolbar debugToolbar;
};

QString algoThresholdNameAt(int index);
RgbTriple algoRowValuesAt(const AlgoThresholds& algo, int index);
float rgbChannelValue(const RgbTriple& rgb, size_t channelIdx);

size_t colorChannelIndex(ColorChannel ch);
common::algorithm::PureDetectColor toPureDetectColor(ColorChannel ch);

int cameraIndexFromName(const QString& cameraName);
int pureChannelCaptureExposure(const RecipeStepDebugImageContext& ctx, size_t channelIdx);
int seriesChannelCaptureExposure(const RecipeStepDebugImageContext& ctx, size_t channelIdx);

void applyRoiToParams(common::algorithm::PureColorDetectParams& pure, const RecipeDebugToolbar& dbg);
void applyRoiToStringLightParams(common::algorithm::StringLightDetectParams& params,
                                 const RecipeDebugToolbar& dbg);

/** 从配方算法阈值表填充纯色（死/暗/亮）与串色阈值到 PureColorDetectParams。 */
void fillPureColorAndColorVariationThresholds(common::algorithm::PureColorDetectParams& pure,
                                              size_t channelIdx,
                                              const AlgoThresholds& algo);

void fillStringLightThresholds(common::algorithm::StringLightDetectParams& stringParams,
                               size_t channelIdx,
                               const AlgoThresholds& algo);

/** PureColorDetect 执行完成后，将 deads/darks/brights/color_variations 数量 Notify 至 UI。 */
void notifyStepDebugPureColorResults(int colorIdx,
                                     const std::vector<common::algorithm::PureColorDefectPoint>* deads,
                                     const std::vector<common::algorithm::PureColorDefectPoint>* darks,
                                     const std::vector<common::algorithm::PureColorDefectPoint>* brights,
                                     const std::vector<common::algorithm::ColorVariationDefectPoint>* color_variations);

void applyStepDebugOutputPaths(common::algorithm::PureColorDetectParams& p,
                               const std::string& cameraDir,
                               bool pureColorOutputs,
                               bool colorVariationOutputs);

void applyStepDebugStringLightOutputPaths(common::algorithm::StringLightDetectParams& p,
                                          const std::string& cameraDir);

std::string joinStepDebugOutputPath(const std::string& dir, const std::string& fileName);

QString sampleCameraOutputDirPath(const QString& cameraPath);
std::string sampleCameraOutputDirLocal8(const QString& cameraPath);

std::shared_ptr<common::algorithm::ImageBuffer> loadRawFileToImageBuffer(
    const QString& filePath,
    const RecipeStepDebugImageContext& ctx);

common::errors::ErrorPtr grabRgbBufferForStepDebug(
    const RecipeStepDebugImageContext& ctx,
    int exposureMs,
    std::shared_ptr<common::algorithm::ImageBuffer>* out_image);

/** 单步调试打屏：查找软屏设备、映射参数并调用 Display（参照 lighten_device_ops::SoftDisplay）。 */
common::errors::ErrorPtr softDisplayForStepDebug(const RecipeStepDebugSoftDisplayParams& params);

/** StringLightDetect 执行完成后，将 string_lights 数量 Notify 至 UI。 */
void notifyStepDebugStringLightResults(
    int colorIdx,
    const std::vector<common::algorithm::StringLightDefectPoint>* string_lights);

} // namespace lighten
