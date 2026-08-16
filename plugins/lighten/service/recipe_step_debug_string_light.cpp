/**
 * @file recipe_step_debug_string_light.cpp
 * @brief 产品配方单步调试「串灯检测」实现。
 */

#include "recipe_step_debug_string_light.h"

#include <common/base/algorithm/algorithm_factory.h>

#include <QDir>

namespace lighten {
namespace {

using common::algorithm::StringLightDetectParams;

StringLightDetectParams buildStringLightParams(const RecipeStepDebugStringLightRequest& request)
{
    const int cameraIndex = cameraIndexFromName(request.cameraName);
    const size_t colorIdx = colorChannelIndex(request.colorChannel);
    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < request.recipe.cameras.size())
                                    ? request.recipe.cameras[cameraIndex]
                                    : PerCameraParams{};

    StringLightDetectParams params{};
    params.color                = toPureDetectColor(request.colorChannel);
    params.led_w                = request.recipe.basic.width;
    params.led_h                = request.recipe.basic.height;
    params.cam_exposure         = seriesChannelCaptureExposure(request, colorIdx);
    params.disp_brightness      = static_cast<int>(rgbChannelValue(cam.seriesGrayLevel, colorIdx));
    params.disp_delay_ms        = request.softDisplay.dispDelayMs;
    params.is_color_camera      = true;
    params.fixing_defective_thr = static_cast<float>(request.recipe.algo.badPixelThresh);
    params.image_size           = {request.xResolution, request.yResolution};
    fillStringLightThresholds(params, colorIdx, request.recipe.algo);
    applyRoiToStringLightParams(params, request.debugToolbar);

    const std::string outputDir = sampleCameraOutputDirLocal8(request.cameraPath);
    applyStepDebugStringLightOutputPaths(params, outputDir);
    params.pure_contour_file_name =
        joinStepDebugOutputPath(outputDir,
                                std::string(common::algorithm::PureDetectColorToString(params.color))
                                    + ".contour");
    return params;
}

} // namespace

QString stepDebugPureContourFilePath(const QString& cameraPath, ColorChannel colorChannel)
{
    const QString prefix = QString::fromLatin1(
        common::algorithm::PureDetectColorToString(toPureDetectColor(colorChannel)));
    return QDir(sampleCameraOutputDirPath(cameraPath)).filePath(prefix + QStringLiteral(".contour"));
}

RecipeStepDebugStringLightResult runRecipeStepDebugStringLight(
    const RecipeStepDebugStringLightRequest& request)
{
    RecipeStepDebugStringLightResult out{};
    out.outputDir = sampleCameraOutputDirPath(request.cameraPath);

    const size_t colorIdx = colorChannelIndex(request.colorChannel);
    const int exposure    = seriesChannelCaptureExposure(request, colorIdx);

    std::shared_ptr<common::algorithm::ImageBuffer> leftImage;
    std::shared_ptr<common::algorithm::ImageBuffer> rightImage;

    const bool captureFromCamera =
        request.leftObliqueRawPath.isEmpty() && request.rightObliqueRawPath.isEmpty();

    if (captureFromCamera) {
        RecipeStepDebugSoftDisplayParams leftDisplay = request.softDisplay;
        leftDisplay.displayMode                      = common::device::ODD_MODE;
        if (auto err = softDisplayForStepDebug(leftDisplay)) {
            out.err = err;
            return out;
        }
        if (auto err = grabRgbBufferForStepDebug(request, exposure, &leftImage)) {
            out.err = err;
            return out;
        }

        RecipeStepDebugSoftDisplayParams rightDisplay = request.softDisplay;
        rightDisplay.displayMode                      = common::device::EVEN_MODE;
        if (auto err = softDisplayForStepDebug(rightDisplay)) {
            out.err = err;
            return out;
        }
        if (auto err = grabRgbBufferForStepDebug(request, exposure, &rightImage)) {
            out.err = err;
            return out;
        }
    } else {
        leftImage  = loadRawFileToImageBuffer(request.leftObliqueRawPath, request);
        rightImage = loadRawFileToImageBuffer(request.rightObliqueRawPath, request);
        if (!leftImage || leftImage->Empty() || !rightImage || rightImage->Empty()) {
            out.err = common::errors::New(-10, "Failed to load RAW image for string light detect");
            return out;
        }
    }

    StringLightDetectParams params = buildStringLightParams(request);
    params.left_image              = std::move(leftImage);
    params.right_image             = std::move(rightImage);

    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", std::move(params));

    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("StringLightDetect");
    if (!algo) {
        out.err = common::errors::New(-12, "Algorithm StringLightDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }

    notifyStepDebugStringLightResults(
        static_cast<int>(toPureDetectColor(request.colorChannel)),
        data.outputs.Get<std::vector<common::algorithm::StringLightDefectPoint>>("string_lights"));
    return out;
}

} // namespace lighten
