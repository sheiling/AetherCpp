#include "mark_point_locate.h"

#include "algo_ci_error_check.h"

#include "vision_algo_i18n.h"

#include "image_utils.h"

#include <vision_algo/units/marker/mark_point_locate_data.h>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/include/macros.h>
#include <common/utility/defer/include/Defer.h>

#include <Logger.h>

#include <ci_basic_error.h>
#include <ci_detection_modname.h>
#include <ci_utils_algomat.h>
#include <marker_detector.h>
#include <model_processor.h>

#include <filesystem>

#include <QVector>

namespace vision_algo {

std::string MarkPointLocate::Name() const
{
    return kUnitMarkPointLocate;
}

common::errors::ErrorPtr MarkPointLocate::Execute(common::algorithm::AlgorithmExecuteData &data)
{
    auto *op = data.inputs.Get<MarkPointLocateOp>(MarkPointLocateInput::kOp);
    if (!op)
        return algoInvalidArgument("vision_algo.error.param_null");

    MarkPointLocateOutput outPkt;

    switch (*op) {
    case MarkPointLocateOp::SdkReady: {
        outPkt.setSdkReady(sdkReady());
        data.outputs = outPkt;
        return nullptr;
    }
    case MarkPointLocateOp::SdkLoadDetail: {
        outPkt.setSdkDetail(sdkLoadDetail().toStdString());
        data.outputs = outPkt;
        return nullptr;
    }
    case MarkPointLocateOp::Locate:
        break;
    default:
        return algoInvalidArgument("vision_algo.error.unit_type_not_available");
    }

    auto *body = data.inputs.Get<MarkPointLocateInput::Fields>(MarkPointLocateInput::kFields);
    if (!body)
        return algoInvalidArgument("vision_algo.error.mark_locate_context_null");

    MarkPointLocateOutput::Fields output;
    auto &in = *body;

    m_markerInfo    = in.markerInfo;
    m_imagePath     = in.imagePath;
    m_saveImageFlag = in.vision.saveImageFlag;

    const auto rgbImage = ensureRgbProcessImage(in.vision.processImage, in.vision.imageCapture);
    if (!rgbImage || !rgbImage->imageMat)
        return algoInvalidImage("vision_algo.error.invalid_image_not_debayered");

    QVector<char> bgrBuffer = packBgrFromRgbImageInfo(rgbImage);
    if (bgrBuffer.isEmpty())
        return algoInvalidImage("vision_algo.error.invalid_image_bgr_pack_failed");

    const int w = rgbImage->imageMat->cols;
    const int h = rgbImage->imageMat->rows;

    CHECK_RTN_INFO_LOG(
        markDetect(bgrBuffer, static_cast<uint64_t>(w), static_cast<uint64_t>(h)));

    output.pixelX          = point_x_pixel;
    output.pixelY          = point_y_pixel;
    output.meta.imageIndex = in.vision.imageIndex;
    output.setOk(true);
    LOG_INFO("[Mark定位] 定位结果 pixelX={} pixelY={}", output.pixelX, output.pixelY);

    outPkt.setFields(std::move(output));
    data.outputs = outPkt;
    return nullptr;
}

common::errors::ErrorPtr MarkPointLocate::markDetect(const QVector<char> &bgrBuffer,
                                                     uint64_t imageWidth, uint64_t imageHeight)
{
    AlgoHandle marker_handle = MarkerCreateHandle();
    AlgoHandle modle_handle  = nullptr;
    DEFER([&] {
        if (modle_handle)
            ModelDestroyHandle(modle_handle);
        if (marker_handle)
            MarkerDestroyHandle(marker_handle);
    });

    ::MarkerType in_marker_type =
        static_cast<::MarkerType>(static_cast<uint16_t>(m_markerInfo.markType));
    MarkerInfo mark_info{};
    mark_info.mark_wide     = m_markerInfo.markWidth;
    mark_info.mark_high     = m_markerInfo.markHeight;
    mark_info.is_white_mark = m_markerInfo.isWhiteMark;

    CHECK_RTN_INFO_LOG(
        checkAlgoError("MarkerSetTypeInfo",
                       MarkerSetTypeInfo(marker_handle, &in_marker_type, &mark_info)));

    MarkerDetectThresh in_marker_thresh{m_markerInfo.area, m_markerInfo.minThresh,
                                        m_markerInfo.maxThresh};
    MarkerDetectMethod detect_method = Method_MatchGray;
    CHECK_RTN_INFO_LOG(checkAlgoError(
        "MarkerSetMethodAndThresh",
        MarkerSetMethodAndThresh(marker_handle, &detect_method, &in_marker_thresh)));

    if (m_markerInfo.modelFileName.empty()
        || !std::filesystem::exists(m_markerInfo.modelFileName)) {
        LOG_ERROR("[Mark定位] 模板文件不存在或为空：{}", m_markerInfo.modelFileName);
        return algoInvalidArgument("vision_algo.error.mark_model_file_missing");
    }

    modle_handle = ModelCreateHandle();
    CHECK_RTN_INFO_LOG(
        checkAlgoError("ModelLoad", ModelLoad(modle_handle, m_markerInfo.modelFileName.c_str())));

    CHECK_RTN_INFO_LOG(checkAlgoError(
        "MarkerSetTemplate",
        MarkerSetTemplate(marker_handle, modle_handle, m_markerInfo.modelName.c_str())));

    ::AlgoMat detectImageMatDst = AlgoMatCreateShadow(static_cast<int>(imageHeight),
                                                    static_cast<int>(imageWidth), ALGO_8UC3,
                                                    const_cast<char *>(bgrBuffer.constData()));
    detectImageMatDst.len = static_cast<unsigned int>(bgrBuffer.size());

    ::AlgoPoint2f out_result{};
    CHECK_RTN_INFO_LOG(
        checkAlgoError("MarkerDetector",
                       MarkerDetector(marker_handle, &detectImageMatDst, &out_result)));

    point_x_pixel = out_result.x;
    point_y_pixel = out_result.y;

    if (m_saveImageFlag && !m_imagePath.empty()) {
        const AlgoError exportErr =
            MarkerDetectExportResultPng(marker_handle, m_imagePath.c_str());
        if (exportErr != ALGO_PASS) {
            LOG_ERROR("[Mark定位] MarkerExportResult 失败，模块={}，错误码={}",
                      ERROR_MODULE_NAME(exportErr), ERROR_CODE_NO_MODULE(exportErr));
        }
    }

    return nullptr;
}

bool MarkPointLocate::sdkReady()
{
    const AlgoHandle handle = MarkerCreateHandle();
    if (!handle)
        return false;
    MarkerDestroyHandle(handle);
    return true;
}

QString MarkPointLocate::sdkLoadDetail()
{
    if (sdkReady())
        return {};
    return tr("vision_algo.sdk.marker_create_failed");
}

REGISTER_ALGORITHM(kUnitMarkPointLocate, MarkPointLocate);

} // namespace vision_algo
