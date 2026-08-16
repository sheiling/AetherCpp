#pragma once

/**
 * @file chip_detect_bridge.h
 * @brief 配方 + FOV 图像 → VisionAlgo 固晶检测桥接
 */

#include "data/dataModel/recipe_config_model.h"
#include "inspection_types.h"

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <common/core/device/imageCapture/image_capture.h>

#include <QString>

#include <memory>

namespace aoi {

struct ChipDetectOutcome
{
    bool                             ok = false;
    QString                          error;
    aoi::inspection::FovDetectRecord record;
};

bool buildDieBondingDetectParam(const RecipeDocument &recipe,
                                vision_algo::DieBondingChipDetectParam *outParam,
                                QString *errorOut = nullptr);

QString recipeDetectConfigKey(const RecipeDocument &recipe);

bool dieBondingDetectParamsEqual(const vision_algo::DieBondingChipDetectParam &a,
                                 const vision_algo::DieBondingChipDetectParam &b);

void logDieBondingDetectParam(const char *tag,
                              const vision_algo::DieBondingChipDetectParam &param,
                              double umPerPx, const RecipeDocument *recipe = nullptr,
                              int detectImageW = 0, int detectImageH = 0);

ChipDetectOutcome detectFovProcessImage(const RecipeDocument &recipe,
                                        std::shared_ptr<common::device::ImageInfo> processImage,
                                        int fovIndex, bool useDeepLearning = true,
                                        bool dlCropLocate = true);

bool initDieBondingEngine(const vision_algo::DieBondingChipDetectParam &param, std::size_t poolSize,
                          int dlInferenceEngineOverride, QString *errorOut = nullptr);

bool syncDieBondingEngineParam(const vision_algo::DieBondingChipDetectParam &param,
                               QString *errorOut = nullptr);

} // namespace aoi
