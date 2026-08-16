#include "die_bonding_chip_detect.h"

#include "algo_ci_data_convert.h"
#include "algo_ci_error_check.h"
#include "die_bonding_engine_manager.h"
#include "dl_model_scanner.h"
#include "image_utils.h"

#include <common/base/algorithm/algorithm_factory.h>
#include <common/include/macros.h>

#include <Logger.h>

#include <algo_mat.h>
#include <ci_basic_error.h>
#ifdef ENABLE_DL_BONDING_DETECT
#include <dl_die_bonding_detector.h>
#endif

#include <cmath>
#include <cstdint>

#include <QDir>
#include <QVector>

#define _GET_BIT(x, y) (((x >> y) & 0x01) != 0)

namespace vision_algo {

common::errors::ErrorPtr DieBondingChipDetect::runDetect(
    const DieBondingChipDetectInput::DetectFields &in,
    DieBondingChipDetectOutput::DetectResult &out, void *nativeDetectHandle)
{
    DieBondingChipDetect detector;
    const auto &vision = in.vision;

    detector.m_imageIndex       = vision.imageIndex;
    detector.m_iImageIndex      = vision.iImageIndex;
    detector.m_isPolarityDetect = in.isPolarityDetect;
    detector.m_isDefectClassify = in.isDefectClassify;

    AlgoHandle diebondingHandle = static_cast<AlgoHandle>(nativeDetectHandle);
    if (!diebondingHandle)
        return algoInvalidArgument("vision_algo.error.detect_handle_null");

    CHECK_RTN_INFO_LOG(detector.setDetectData(diebondingHandle, in));

    ::AlgoMat     shadowMat{};
    QVector<char> bgrBuffer;

    const auto rgbImage = ensureRgbProcessImage(vision.processImage, vision.imageCapture);
    if (!rgbImage || !rgbImage->imageMat)
        return algoInvalidImage("vision_algo.error.invalid_image_not_debayered");

    bgrBuffer = packBgrFromRgbImageInfo(rgbImage);
    if (bgrBuffer.isEmpty())
        return algoInvalidImage("vision_algo.error.invalid_image_bgr_pack_failed");

    const int w = rgbImage->imageMat->cols;
    const int h = rgbImage->imageMat->rows;
    shadowMat   = AlgoMatCreateShadow(h, w, ALGO_8UC3, bgrBuffer.data());
    shadowMat.len = static_cast<unsigned int>(bgrBuffer.size());
    ::AlgoMat *dstMatPtr = &shadowMat;

    if (vision.saveMatFlag && !vision.amatFileName.empty())
        AlgoMatSave(dstMatPtr, vision.amatFileName.c_str());

    int row      = 0;
    int col      = 0;
    int groupNum = 0;
    std::vector<DieBondingChipGroupInfo> cisdkGroups;
    if (in.isDeepLearning) {
#ifdef ENABLE_DL_BONDING_DETECT
        CHECK_RTN_INFO_LOG(detector.chipDetectDL(diebondingHandle, dstMatPtr, in, cisdkGroups, row,
                                                 col, groupNum));
#else
        return algoInvalidArgument("vision_algo.error.dl_bonding_disabled");
#endif
    } else {
        CHECK_RTN_INFO_LOG(
            detector.chipDetect(diebondingHandle, dstMatPtr, in.alignType, cisdkGroups, row, col));
    }

    out.result.chipGroupInfos = cisdk_convert::fromCisdkGroups(cisdkGroups);
    if (in.isDeepLearning)
        out.result.chipGroupNum = groupNum;
    else
        out.result.chipGroupNum = row * col;
    out.meta.imageIndex = detector.m_imageIndex;

    const bool bOk = detector.isOk(in, cisdkGroups.data(), out.result.chipGroupNum);
    out.setOk(bOk);

    if (vision.saveImageFlag && !vision.detectImageFileName.empty())
        detector.saveDetectImage(diebondingHandle, vision.detectImageFileName, in.isDeepLearning);

    return nullptr;
}

common::errors::ErrorPtr DieBondingChipDetect::setDetectData(
    AlgoHandle handle, const DieBondingChipDetectInput::DetectFields &in)
{
    if (in.isDeepLearning) {
#ifdef ENABLE_DL_BONDING_DETECT
        return checkAlgoError("DLDieBondingSetImageIndex",
                              DLDieBondingSetImageIndex(handle, in.vision.imageIndex.c_str()));
#else
        (void)handle;
        return algoInvalidArgument("vision_algo.error.dl_bonding_disabled");
#endif
    }

    return checkAlgoError("DieBondingSetImageIndex",
                          DieBondingSetImageIndex(handle, m_imageIndex.c_str()));
}

common::errors::ErrorPtr DieBondingChipDetect::chipDetectDL(
    AlgoHandle handle, ::AlgoMat *dstMat, const DieBondingChipDetectInput::DetectFields &in,
    std::vector<DieBondingChipGroupInfo> &groups, int &outRow, int &outCol, int &groupNum)
{
#ifdef ENABLE_DL_BONDING_DETECT
    outRow   = 0;
    outCol   = 0;
    groupNum = 0;

    const bool cropLocate = in.dlCropLocate;

    LOG_INFO(
        "[VirtualInspect] DLDieBondingChipGroupLocate enter {}x{} crop={} imageIndex={} "
        "polarity={} classify={} pixelSizeUm={:.4f}",
        dstMat ? dstMat->cols : 0, dstMat ? dstMat->rows : 0, cropLocate, m_imageIndex,
        m_isPolarityDetect, m_isDefectClassify, in.pixelSize);

    AlgoError err =
        DLDieBondingChipGroupLocate(handle, dstMat, &outRow, &outCol, cropLocate);
    LOG_INFO("[VirtualInspect] DLDieBondingChipGroupLocate done err={} row={} col={}",
             static_cast<int>(err), outRow, outCol);
    if (const auto e = checkAlgoError("DLDieBondingChipGroupLocate", err))
        return e;

    if (outCol <= 0 || outRow <= 0)
        return nullptr;

    LOG_INFO("[VirtualInspect] DLDieBondingDetect begin");
    err = DLDieBondingDetect(handle, in.isDefectClassify, in.isPolarityDetect, false);
    LOG_INFO("[VirtualInspect] DLDieBondingDetect done err={}", static_cast<int>(err));
    if (const auto e = checkAlgoError("DLDieBondingDetect", err))
        return e;

    err = DLDieBondingGetGroupNum(handle, &groupNum);
    if (const auto e = checkAlgoError("DLDieBondingGetGroupNum", err))
        return e;

    if (groupNum <= 0)
        return nullptr;

    groups.resize(static_cast<size_t>(groupNum));
    err = DLDieBondingGetAllChipGroupInfo(handle, groups.data());
    return checkAlgoError("DLDieBondingGetAllChipGroupInfo", err);
#else
    (void)handle;
    (void)dstMat;
    (void)in;
    (void)groups;
    outRow   = 0;
    outCol   = 0;
    groupNum = 0;
    return algoInvalidArgument("vision_algo.error.dl_bonding_disabled");
#endif
}

common::errors::ErrorPtr DieBondingChipDetect::chipDetect(
    AlgoHandle handle, ::AlgoMat *dstMat, int alignType,
    std::vector<DieBondingChipGroupInfo> &groups, int &outRow, int &outCol)
{
    AlgoError err = DieBondingChipGroupLocate(handle, dstMat);
    if (const auto e = checkAlgoError("DieBondingChipGroupLocate", err))
        return e;

    outRow = 0;
    outCol = 0;
    err    = DieBondingCalGroupRowCol(handle, &outCol, &outRow);
    if (const auto e = checkAlgoError("DieBondingCalGroupRowCol", err))
        return e;

    if (outCol <= 0 || outRow <= 0)
        return nullptr;

    auto align_type = static_cast<DieBondingChipAlignType>(alignType);
    err             = DieBondingDetect(handle, align_type);
    if (const auto e = checkAlgoError("DieBondingDetect", err))
        return e;

    groups.resize(static_cast<size_t>(outRow * outCol));
    err = DieBondingGetAllChipGroupInfo(handle, groups.data());
    return checkAlgoError("DieBondingGetAllChipGroupInfo", err);
}

bool DieBondingChipDetect::saveDetectImage(AlgoHandle handle, const std::string &filename,
                                           bool isDeepLearning)
{
    if (isDeepLearning) {
#ifdef ENABLE_DL_BONDING_DETECT
        const AlgoError err = DLDieBondingExportDetectResult(handle, filename.c_str());
        if (err != ALGO_PASS) {
            LOG_ERROR("[固晶检测] DLDieBondingExportDetectResult 失败，模块={}，错误码={}",
                      ERROR_MODULE_NAME(err), ERROR_CODE_NO_MODULE(err));
            return false;
        }
        return true;
#else
        (void)handle;
        (void)filename;
        return false;
#endif
    }

    const AlgoError err = DieBondingExportDetectResult(handle, filename.c_str());
    if (err != ALGO_PASS) {
        LOG_ERROR("[固晶检测] DieBondingExportDetectResult 失败，模块={}，错误码={}",
                  ERROR_MODULE_NAME(err), ERROR_CODE_NO_MODULE(err));
        return false;
    }
    return true;
}

bool DieBondingChipDetect::isOk(const DieBondingChipDetectInput::DetectFields &in,
                                DieBondingChipGroupInfo *chip_group_infos, int groupNum)
{
    for (int i = 0; i < groupNum; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            const DieBondingChipInfo chipInfo = chip_group_infos[i].chip_rgb_info[j];

            if (in.chipMiss && _GET_BIT(chipInfo.chip_defect, 0))
                return false;
            if (in.chipDirty && _GET_BIT(chipInfo.chip_defect, 1))
                return false;
            if (in.chipBroken && _GET_BIT(chipInfo.chip_defect, 2))
                return false;
            if (in.chipForeignObject && _GET_BIT(chipInfo.chip_defect, 3))
                return false;
            if (in.chipBondingError && _GET_BIT(chipInfo.chip_defect, 5))
                return false;

            const bool bChipPolar =
                (chipInfo.polar_type != 4) && (chipInfo.polar_type != in.polar);
            if (in.chipPolar && bChipPolar)
                return false;

            float deflection_x = 0.f;
            float deflection_y = 0.f;
            if (in.skewType == 0) {
                deflection_x = chipInfo.solder_chip_deflection.x;
                deflection_y = chipInfo.solder_chip_deflection.y;
            } else if (in.skewType == 1) {
                const bool bChannel =
                    (std::fabs(chipInfo.channel_coordinate.x)
                     + std::fabs(chipInfo.channel_coordinate.y)
                     > 0.000001f);
                const bool bChip = (std::fabs(chipInfo.chip_coordinate.x)
                                    + std::fabs(chipInfo.chip_coordinate.y)
                                    > 0.000001f);
                if (bChannel && bChip) {
                    deflection_x = chipInfo.channel_coordinate.x - chipInfo.chip_coordinate.x;
                    deflection_y = chipInfo.channel_coordinate.y - chipInfo.chip_coordinate.y;
                }
            }

            const bool bChipSkew =
                (std::fabs(deflection_x) > in.skewX / in.pixelSize)
                || (std::fabs(deflection_y) > in.skewY / in.pixelSize)
                || (std::fabs(chipInfo.chip_angle) > in.skewA);
            if (in.chipSkew && bChipSkew)
                return false;

            if (in.chipOtherDefect && _GET_BIT(chipInfo.chip_defect, 13))
                return false;
        }
    }
    return true;
}

std::string DieBondingChipDetect::Name() const
{
    return kUnitDieBondingChipDetect;
}

common::errors::ErrorPtr DieBondingChipDetect::Execute(common::algorithm::AlgorithmExecuteData &data)
{
    auto *op = data.inputs.Get<DieBondingChipDetectOp>(DieBondingChipDetectInput::kOp);
    if (!op)
        return algoInvalidArgument("vision_algo.error.param_null");

    DieBondingChipDetectOutput outPkt;

    switch (*op) {
    case DieBondingChipDetectOp::InitEngine: {
        auto *param = data.inputs.Get<DieBondingChipDetectParam>(DieBondingChipDetectInput::kParam);
        if (!param)
            return algoInvalidArgument("vision_algo.error.param_null");
        const std::size_t poolSize =
            data.inputs.Get<std::size_t>(DieBondingChipDetectInput::kPoolSize)
                ? *data.inputs.Get<std::size_t>(DieBondingChipDetectInput::kPoolSize)
                : kDefaultDieBondingPoolSize;
        const int dlOverride =
            data.inputs.Get<int>(DieBondingChipDetectInput::kDlInferenceEngineOverride)
                ? *data.inputs.Get<int>(DieBondingChipDetectInput::kDlInferenceEngineOverride)
                : -1;
        std::string err;
        std::size_t actualPool = 0;
        const bool  ready      = die_bonding_internal::acquireEngine(
            *param, poolSize, dlOverride, &err, &actualPool);
        outPkt.setReady(ready);
        outPkt.setPoolSize(actualPool);
        data.outputs = outPkt;
        if (!ready)
            return common::errors::New(common::errors::algo_module::kSdkError, err);
        return nullptr;
    }
    case DieBondingChipDetectOp::SyncParam: {
        auto *param = data.inputs.Get<DieBondingChipDetectParam>(DieBondingChipDetectInput::kParam);
        if (!param)
            return algoInvalidArgument("vision_algo.error.param_null");
        std::string err;
        if (!die_bonding_internal::syncEngineParam(*param, &err))
            return common::errors::New(common::errors::algo_module::kInvalidArgument, err);
        outPkt.setReady(die_bonding_internal::engineReady());
        data.outputs = outPkt;
        return nullptr;
    }
    case DieBondingChipDetectOp::ScanDlModels: {
        auto *param = data.inputs.Get<DieBondingChipDetectParam>(DieBondingChipDetectInput::kParam);
        if (!param)
            return algoInvalidArgument("vision_algo.error.param_null");
        const std::string directory =
            data.inputs.Get<std::string>(DieBondingChipDetectInput::kDirectory)
                ? *data.inputs.Get<std::string>(DieBondingChipDetectInput::kDirectory)
                : std::string{};
        const bool preferOnnx =
            data.inputs.Get<bool>(DieBondingChipDetectInput::kPreferOnnx)
                ? *data.inputs.Get<bool>(DieBondingChipDetectInput::kPreferOnnx)
                : true;
        QString scanError;
        if (!scanDlModelDirectory(QString::fromStdString(directory), *param, nullptr, &scanError,
                                  preferOnnx)) {
            return common::errors::New(common::errors::algo_module::kInvalidArgument,
                                       scanError.toStdString());
        }
        outPkt.setParam(*param);
        data.outputs = outPkt;
        return nullptr;
    }
    case DieBondingChipDetectOp::Detect: {
        auto *fields =
            data.inputs.Get<DieBondingChipDetectInput::DetectFields>(
                DieBondingChipDetectInput::kDetectFields);
        if (!fields)
            return algoInvalidArgument("vision_algo.error.param_null");
        auto *engine = die_bonding_internal::activeEngine();
        if (!engine || !engine->isReady())
            return algoInvalidArgument("vision_algo.error.engine_not_initialized");
        DieBondingChipDetectOutput::DetectResult result;
        if (const auto err = engine->runDetect(*fields, result))
            return err;
        outPkt.setDetectResult(std::move(result));
        data.outputs = outPkt;
        return nullptr;
    }
    default:
        return algoInvalidArgument("vision_algo.error.unit_type_not_available");
    }
}

REGISTER_ALGORITHM(kUnitDieBondingChipDetect, DieBondingChipDetect);

} // namespace vision_algo
