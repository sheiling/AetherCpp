#include "barcode_scan_unit.h"

#include "algo_ci_error_check.h"

#include <vision_algo/units/scan/barcode_scan_data.h>

#include "image_utils.h"

#include <common/base/algorithm/algorithm_factory.h>

#include <BarcodeFormat.h>
#include <ImageView.h>
#include <ReadBarcode.h>
#include <ReaderOptions.h>

namespace vision_algo {
namespace {

BarcodeScanOutput::Fields mapZxingResult(const ZXing::Barcode &barcode)
{
    BarcodeScanOutput::Fields result;
    if (!barcode.isValid())
        return result;

    const std::string text = barcode.text();
    if (text.empty())
        return result;

    result.setOk(true);
    result.content   = QString::fromStdString(text);
    result.symbology = QString::fromStdString(ZXing::ToString(barcode.format()));
    result.codeType  = barcode.format() == ZXing::BarcodeFormat::QRCode ? BarcodeCodeType::QrCode
                                                                        : BarcodeCodeType::Barcode;

    for (const auto &pt : barcode.position())
        result.cornerPoints.append(QPointF(pt.x, pt.y));

    return result;
}

BarcodeScanOutput::Fields tryZxingDetectRgbMat(const ::AlgoMat &rgbMat)
{
    if (AlgoMatIsEmpty(&rgbMat) || !rgbMat.data)
        return {};

    try {
        ZXing::ReaderOptions opts;
        opts.setTryHarder(true)
            .setTryRotate(true)
            .setTryInvert(true)
            .setTryDownscale(true)
            .setFormats(ZXing::BarcodeFormat::Any);

        const ZXing::ImageView view(rgbMat.data, rgbMat.cols, rgbMat.rows, ZXing::ImageFormat::RGB,
                                    rgbMat.cols * 3);

        for (const auto &barcode : ZXing::ReadBarcodes(view, opts)) {
            BarcodeScanOutput::Fields result = mapZxingResult(barcode);
            if (result.ok())
                return result;
        }
    } catch (const std::exception &) {
        return {};
    }

    return {};
}

BarcodeScanOutput::Fields detectFromProcessImage(
    const std::shared_ptr<common::device::ImageInfo> &processImage)
{
    if (!isValidRgbImageInfo(processImage))
        return {};
    return tryZxingDetectRgbMat(*processImage->imageMat);
}

} // namespace

std::string BarcodeScanUnit::Name() const
{
    return kUnitBarcodeScan;
}

common::errors::ErrorPtr BarcodeScanUnit::Execute(common::algorithm::AlgorithmExecuteData &data)
{
    const VisionImageContext *vision = data.inputs.Get<VisionImageContext>(BarcodeScanInput::kVision);
    if (!vision)
        return algoInvalidArgument("vision_algo.error.barcode_context_null");
    if (!vision->processImage)
        return algoInvalidArgument("vision_algo.error.barcode_no_image");

    BarcodeScanOutput outPkt;
    outPkt.setResult(detectFromProcessImage(vision->processImage));
    data.outputs = outPkt;
    return nullptr;
}

REGISTER_ALGORITHM(kUnitBarcodeScan, BarcodeScanUnit);

} // namespace vision_algo
