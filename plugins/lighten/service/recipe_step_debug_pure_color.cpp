/**
 * @file recipe_step_debug_pure_color.cpp
 * @brief 产品配方单步调试「纯色检测」实现。
 */

#include "recipe_step_debug_pure_color.h"

#include "recipe_step_debug_common.h"

#include <common/base/algorithm/algorithm_factory.h>

namespace lighten {
namespace {

using common::algorithm::PureColorDetectParams;

PureColorDetectParams buildPureColorParams(const RecipeStepDebugPureColorRequest& request)
{
    const int cameraIndex = cameraIndexFromName(request.cameraName);
    const size_t colorIdx = colorChannelIndex(request.colorChannel);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < request.recipe.cameras.size())
                                    ? request.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};

    PureColorDetectParams pure{};
    pure.color                = toPureDetectColor(request.colorChannel);
    pure.led_w                = request.recipe.basic.width;
    pure.led_h                = request.recipe.basic.height;
    pure.cam_exposure         = pureChannelCaptureExposure(request, colorIdx);
    pure.disp_brightness      = static_cast<int>(rgbChannelValue(cam.pureGrayLevel, colorIdx));
    pure.disp_delay_ms        = request.softDisplay.dispDelayMs;
    pure.is_side              = cameraIndex != 0;
    pure.angle                = cam.rotationDeg;
    pure.fixing_defective_thr = static_cast<float>(request.recipe.algo.badPixelThresh);
    pure.is_color_camera      = true;
    pure.image_size           = {request.xResolution, request.yResolution};
    fillPureColorAndColorVariationThresholds(pure, colorIdx, request.recipe.algo);
    applyRoiToParams(pure, request.debugToolbar);

    pure.dead_check   = true;
    pure.dark_check   = true;
    pure.bright_check = true;
    pure.color_check  = true;

    const std::string outputDir = sampleCameraOutputDirLocal8(request.cameraPath);
    applyStepDebugOutputPaths(pure, outputDir, true, true);
    return pure;
}

} // namespace

RecipeStepDebugPureColorResult runRecipeStepDebugPureColor(
    const RecipeStepDebugPureColorRequest& request)
{
    RecipeStepDebugPureColorResult out{};
    out.outputDir = sampleCameraOutputDirPath(request.cameraPath);

    std::shared_ptr<common::algorithm::ImageBuffer> workingImage;
    if (request.rawImagePath.isEmpty()) {
        if (auto err = softDisplayForStepDebug(request.softDisplay)) {
            out.err = err;
            return out;
        }
        if (auto err = grabRgbBufferForStepDebug(
                request,
                pureChannelCaptureExposure(request, colorChannelIndex(request.colorChannel)),
                &workingImage)) {
            out.err = err;
            return out;
        }
    } else {
        workingImage = loadRawFileToImageBuffer(request.rawImagePath, request);
        if (!workingImage || workingImage->Empty()) {
            out.err = common::errors::New(-10, "Failed to load RAW image for pure color detect");
            return out;
        }
    }

    if (!workingImage || workingImage->Empty()) {
        out.err = common::errors::New(-11, "Pure color detect: image is empty");
        return out;
    }

    PureColorDetectParams params = buildPureColorParams(request);
    params.image                 = workingImage;

    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", std::move(params));

    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("PureColorDetect");
    if (!algo) {
        out.err = common::errors::New(-12, "Algorithm PureColorDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }

    const int colorIdx = static_cast<int>(toPureDetectColor(request.colorChannel));
    notifyStepDebugPureColorResults(
        colorIdx,
        data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("deads"),
        data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("darks"),
        data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("brights"),
        data.outputs.Get<std::vector<common::algorithm::ColorVariationDefectPoint>>("color_variations"));
    return out;
}

} // namespace lighten
