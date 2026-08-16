#include "mark_point_locator.h"

#include <vision_algo/units/marker/mark_point_locate_data.h>
#include <vision_algo/units/model/model_processor_data.h>
#include <vision_algo/core/service_access.h>

#include <common/base/imgproc/image_type_map.h>
#include <common/core/device/imageCapture/image_capture.h>

#include <algo_mat.h>
#include <QFile>

namespace aoi {
namespace {

bool isRgbProcessImage(const std::shared_ptr<common::device::ImageInfo> &info)
{
    return info && info->imageMat && !AlgoMatIsEmpty(info->imageMat.get())
           && info->imageFormat == common::imgproc::ImageFormat::RGB24;
}

bool buildMarkLocateInput(const std::shared_ptr<common::device::ImageInfo> &processImage,
                          const MarkPointConfig &config,
                          vision_algo::MarkPointLocateInput::Fields &in, QString &errorKey)
{
    if (!isRgbProcessImage(processImage)) {
        errorKey = QStringLiteral("aoi.recipe.mark.no_preview");
        return false;
    }

    const vision_algo::MarkerDetectInfo &mk = config.markerInfo;
    const QString modelFile = QString::fromStdString(mk.modelFileName).trimmed();
    const QString modelName = QString::fromStdString(mk.modelName).trimmed();
    if (modelFile.isEmpty() || modelName.isEmpty()) {
        errorKey = QStringLiteral("aoi.recipe.mark.template_missing");
        return false;
    }

    const std::string modelPath = vision_algo::resolveModelFilePath(modelFile.toStdString());
    if (!QFile::exists(QString::fromStdString(modelPath))) {
        errorKey = QStringLiteral("aoi.recipe.mark.model_not_found");
        return false;
    }

    in.vision.processImage = processImage;
    in.markerInfo          = mk;
    in.markerInfo.modelFileName = modelPath;
    in.vision.saveImageFlag     = false;
    return true;
}

} // namespace

bool MarkPointLocator::sdkReady()
{
    auto *svc = vision_algo::service();
    if (!svc || !svc->isReady())
        return false;
    vision_algo::ModelProcessorInput modelIn;
    modelIn.setOp(vision_algo::ModelProcessorOp::SdkReady);
    common::algorithm::AlgorithmExecuteData modelData;
    modelData.inputs = modelIn;
    if (svc->execute(vision_algo::kUnitModelProcessor, modelData))
        return false;
    if (auto *modelReady = modelData.outputs.Get<bool>(vision_algo::ModelProcessorOutput::kSdkReady);
        !modelReady || !*modelReady)
        return false;

    vision_algo::MarkPointLocateInput markerIn;
    markerIn.setOp(vision_algo::MarkPointLocateOp::SdkReady);
    common::algorithm::AlgorithmExecuteData markerData;
    markerData.inputs = markerIn;
    if (svc->execute(vision_algo::kUnitMarkPointLocate, markerData))
        return false;
    if (auto *markerReady = markerData.outputs.Get<bool>(vision_algo::MarkPointLocateOutput::kSdkReady))
        return *markerReady;
    return false;
}

QString MarkPointLocator::sdkIssue()
{
    return vision_algo::markModelSdkIssue();
}

MarkLocateResult MarkPointLocator::locate(
    const std::shared_ptr<common::device::ImageInfo> &processImage, const MarkPointConfig &config)
{
    MarkLocateResult result;

    if (!sdkReady()) {
        result.message = vision_algo::markModelSdkIssue();
        return result;
    }

    vision_algo::MarkPointLocateInput::Fields fields;
    QString                              buildError;
    if (!buildMarkLocateInput(processImage, config, fields, buildError)) {
        result.errorKey = buildError;
        return result;
    }

    auto *svc = vision_algo::service();
    if (!svc || !svc->isUnitAvailable(vision_algo::kUnitMarkPointLocate)) {
        result.errorKey = QStringLiteral("aoi.recipe.mark.locate_failed");
        return result;
    }

    vision_algo::MarkPointLocateInput inPkt;
    inPkt.setOp(vision_algo::MarkPointLocateOp::Locate);
    inPkt.setFields(std::move(fields));

    common::algorithm::AlgorithmExecuteData data;
    data.inputs = static_cast<const common::algorithm::AlgorithmPacket &>(inPkt);
    if (const auto err = svc->execute(vision_algo::kUnitMarkPointLocate, data); err) {
        result.errorKey = QStringLiteral("aoi.recipe.mark.locate_failed");
        return result;
    }

    auto *outBody = data.outputs.Get<vision_algo::MarkPointLocateOutput::Fields>(
        vision_algo::MarkPointLocateOutput::kFields);
    if (!outBody) {
        result.errorKey = QStringLiteral("aoi.recipe.mark.locate_failed");
        return result;
    }

    result.ok          = outBody->ok();
    result.pixelX      = outBody->pixelX;
    result.pixelY      = outBody->pixelY;
    result.resultImage = common::device::ImageCapture::cloneImageInfo(processImage);
    return result;
}

} // namespace aoi
