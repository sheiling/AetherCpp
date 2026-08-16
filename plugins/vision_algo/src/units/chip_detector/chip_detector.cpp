#include "chip_detector.h"

#include "algo_ci_data_convert.h"
#include "algo_ci_error_check.h"
#include "image_utils.h"

#include <vision_algo/units/chip_detector/chip_detector_data.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/include/macros.h>
#include <common/utility/defer/include/Defer.h>

#include <Logger.h>

#include <chip_detector.h>
#include <ci_basic_error.h>
#include <model_processor.h>

#include <QVector>

namespace vision_algo {

common::errors::ErrorPtr ChipDetector::runDetect(const ChipDetectorInput::Fields &in,
                                                 ChipDetectorOutput::Fields &out)
{
    const auto cisdkRgbType   = cisdk_convert::toCisdkRgbType(in.chipDetectRgbType);
    const auto cisdkAlignType = cisdk_convert::toCisdkRgbAlignType(in.chipDetectRgbAlignType);
    const auto cisdkChipSize  = cisdk_convert::toCisdkSize2f(in.chipSize);
    const auto cisdkThresh    = cisdk_convert::toCisdkChipThreshold(in.chipThresh);
    const auto cisdkRoi       = cisdk_convert::toCisdkSize(in.chipROI);

    AlgoHandle chip_handle = ChipCreateHandle();
    DEFER([&] {
        ChipDestroyHandle(chip_handle);
        chip_handle = nullptr;
    });

    CHECK_RTN_INFO_LOG(checkAlgoError("ChipSetTypeInfo",
                                      ChipSetTypeInfo(chip_handle, cisdkRgbType, cisdkAlignType,
                                                      &cisdkChipSize)));
    CHECK_RTN_INFO_LOG(
        checkAlgoError("ChipSetThresh", ChipSetThresh(chip_handle, &cisdkThresh)));

    if (in.chipROI.width > 0 && in.chipROI.height > 0) {
        CHECK_RTN_INFO_LOG(
            checkAlgoError("ChipSetROI", ChipSetROI(chip_handle, &cisdkRoi)));
    }

    {
        AlgoHandle modle_h = ModelCreateHandle();
        DEFER([&] {
            ModelDestroyHandle(modle_h);
            modle_h = nullptr;
        });

        CHECK_RTN_INFO_LOG(
            checkAlgoError("ModelLoad", ModelLoad(modle_h, in.modelFile.c_str())));

        CHECK_RTN_INFO_LOG(checkAlgoError(
            "ChipSetDoubleTemplate",
            ChipSetDoubleTemplate(chip_handle, modle_h, in.chipGroupModeName.c_str(),
                                  in.chipModeName.c_str())));
    }

    const auto rgbImage = ensureRgbProcessImage(in.vision.processImage, in.vision.imageCapture);
    if (!rgbImage)
        return algoInvalidImage("vision_algo.error.invalid_image_not_debayered");

    QVector<char> bgrBuffer = packBgrFromRgbImageInfo(rgbImage);
    if (bgrBuffer.isEmpty())
        return algoInvalidImage("vision_algo.error.invalid_image_bgr_pack_failed");

    ::AlgoMat imgMat = AlgoMatCreateShadow(rgbImage->imageMat->rows, rgbImage->imageMat->cols,
                                           ALGO_8UC3, bgrBuffer.data());
    imgMat.len       = static_cast<unsigned int>(bgrBuffer.size());

    AlgoPoint2f out_chip_center{};
    AlgoPoint2f out_solder_center{};
    float       out_chip_angle = 0.f;
    CHECK_RTN_INFO_LOG(checkAlgoError(
        "ChipDetectTarget",
        ChipDetectTarget(chip_handle, &imgMat, &out_chip_center, &out_solder_center,
                         &out_chip_angle)));

    out.outChipAngle   = out_chip_angle;
    out.outChipCenterX = out_chip_center.x;
    out.outChipCenterY = out_chip_center.y;
    out.meta.imageIndex = in.vision.imageIndex;
    out.setOk(true);

    if (!in.outImageFile.empty()) {
        CHECK_RTN_INFO_LOG(checkAlgoError(
            "ChipDetectExportResultPng",
            ChipDetectExportResultPng(chip_handle, in.outImageFile.c_str())));
    }

    return nullptr;
}

std::string ChipDetector::Name() const
{
    return kUnitChipDetector;
}

common::errors::ErrorPtr ChipDetector::Execute(common::algorithm::AlgorithmExecuteData &data)
{
    auto *op = data.inputs.Get<ChipDetectorOp>(ChipDetectorInput::kOp);
    if (!op || *op != ChipDetectorOp::Detect)
        return algoInvalidArgument("vision_algo.error.unit_type_not_available");

    auto *fields = data.inputs.Get<ChipDetectorInput::Fields>(ChipDetectorInput::kFields);
    if (!fields)
        return algoInvalidArgument("vision_algo.error.chip_detector_context_null");

    ChipDetectorOutput outPkt;
    ChipDetectorOutput::Fields result;
    if (const auto err = runDetect(*fields, result))
        return err;
    outPkt.setFields(std::move(result));
    data.outputs = outPkt;
    return nullptr;
}

REGISTER_ALGORITHM(kUnitChipDetector, ChipDetector);

} // namespace vision_algo
