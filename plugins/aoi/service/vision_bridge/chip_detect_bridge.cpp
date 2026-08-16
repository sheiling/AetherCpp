#include "chip_detect_bridge.h"

#include <vision_algo/core/service_access.h>

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <ivision_algo_service.h>
#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>

#include <Logger.h>

#include <common/base/imgproc/image_type_map.h>

#include <QDir>
#include <QtGlobal>

#include <cmath>
#include <cstring>
#include <memory>

namespace aoi {
namespace {

vision_algo::IVisionAlgoService *algoService()
{
    return vision_algo::service();
}

bool runDieBondingExecute(vision_algo::DieBondingChipDetectInput &inPkt,
                          vision_algo::DieBondingChipDetectOutput *outPkt, QString *errorOut)
{
    auto *svc = algoService();
    if (!svc || !svc->isReady()) {
        if (errorOut)
            *errorOut = QStringLiteral("VisionAlgo 服务不可用");
        return false;
    }
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = static_cast<const common::algorithm::AlgorithmPacket &>(inPkt);
    if (const auto err = svc->execute(vision_algo::kUnitDieBondingChipDetect, data)) {
        if (errorOut)
            *errorOut = QString::fromStdString(err->Message());
        return false;
    }
    if (outPkt)
        static_cast<common::algorithm::AlgorithmPacket &>(*outPkt) = data.outputs;
    return true;
}

/** 华星产线 UI / demo_config 参考（capture_accuracy=2.95 μm/px） */
constexpr double kHuaxingRefUmPerPx     = 2.95;
constexpr float  kHuaxingRefSolderW   = 109.f;
constexpr float  kHuaxingRefSolderH   = 45.f;
constexpr float  kHuaxingRefChipW     = 51.f;
constexpr float  kHuaxingRefChipH     = 20.f;
constexpr float  kHuaxingRefGroupW    = 109.f;
constexpr float  kHuaxingRefGroupH    = 175.f;
constexpr float  kHuaxingRefPitchUmX  = 1250.f;
constexpr float  kHuaxingRefPitchUmY  = 1250.f;

/** 閰嶆柟 camera.captureAccuracy锛毼糾/px锛堜笌 fov_planner銆佸崕鏄?demo pixelSizeUm 涓€鑷达級 */
double recipeCaptureUmPerPixel(const RecipeDocument &recipe)
{
    return recipe.camera.captureAccuracy > 0.0 ? recipe.camera.captureAccuracy : 1.0;
}

/** 鐗╃悊灏哄 mm 鈫?CISDK 鍍忕礌鍙傛暟 */
float mmToPx(double mm, double umPerPixel)
{
    if (umPerPixel <= 0.0)
        return static_cast<float>(mm * 1000.0);
    return static_cast<float>(mm * 1000.0 / umPerPixel);
}

bool isRgbProcessImage(const std::shared_ptr<common::device::ImageInfo> &info)
{
    return info && info->imageMat && info->imageMat->data && info->imageMat->cols > 0
           && info->imageMat->rows > 0
           && info->imageFormat == common::imgproc::ImageFormat::RGB24;
}

bool nearlyEqual(float a, float b)
{
    return qAbs(a - b) < 1e-5f;
}

} // namespace

QString recipeDetectConfigKey(const RecipeDocument &recipe)
{
    const double umPerPx = recipeCaptureUmPerPixel(recipe);
    return QStringLiteral(
               "%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12|%13")
        .arg(recipe.basic.padWidthMm, 0, 'f', 6)
        .arg(recipe.basic.padHeightMm, 0, 'f', 6)
        .arg(recipe.basic.chipWidthMm, 0, 'f', 6)
        .arg(recipe.basic.chipHeightMm, 0, 'f', 6)
        .arg(recipe.basic.groupWidthMm, 0, 'f', 6)
        .arg(recipe.basic.groupHeightMm, 0, 'f', 6)
        .arg(recipe.basic.groupPitchXmm, 0, 'f', 6)
        .arg(recipe.basic.groupPitchYmm, 0, 'f', 6)
        .arg(recipe.basic.chipPitchInGroupMm, 0, 'f', 6)
        .arg(recipe.algo.chipConfidence, 0, 'f', 6)
        .arg(recipe.algo.groupConfidence, 0, 'f', 6)
        .arg(recipe.algo.chipDefectClass, 0, 'f', 6)
        .arg(recipe.algo.padDefectClass, 0, 'f', 6)
        .arg(umPerPx, 0, 'f', 9);
}

bool dieBondingDetectParamsEqual(const vision_algo::DieBondingChipDetectParam &a,
                                 const vision_algo::DieBondingChipDetectParam &b)
{
    return nearlyEqual(a.solderWidth, b.solderWidth) && nearlyEqual(a.solderHeight, b.solderHeight)
           && nearlyEqual(a.chipWidth, b.chipWidth) && nearlyEqual(a.chipHeight, b.chipHeight)
           && nearlyEqual(a.chipGroupWidth, b.chipGroupWidth)
           && nearlyEqual(a.chipGroupHeight, b.chipGroupHeight) && nearlyEqual(a.ledGapX, b.ledGapX)
           && nearlyEqual(a.ledGapY, b.ledGapY)
           && nearlyEqual(a.chipPitchInGroupMm, b.chipPitchInGroupMm)
           && nearlyEqual(a.confThr, b.confThr) && nearlyEqual(a.confGroupThr, b.confGroupThr)
           && nearlyEqual(a.chipLimThr, b.chipLimThr) && nearlyEqual(a.solderLimThr, b.solderLimThr)
           && a.chipGroupModel == b.chipGroupModel && a.chipGroupKey == b.chipGroupKey
           && a.chipDetectModel == b.chipDetectModel && a.chipDetectKey == b.chipDetectKey
           && a.detectClassifyModel == b.detectClassifyModel
           && a.detectClassifyKey == b.detectClassifyKey && a.polarDetectModel == b.polarDetectModel
           && a.polarDetectKey == b.polarDetectKey && a.dlInferenceEngine == b.dlInferenceEngine;
}

void logDieBondingDetectParam(const char *tag,
                              const vision_algo::DieBondingChipDetectParam &param,
                              double umPerPx, const RecipeDocument *recipe, int detectImageW,
                              int detectImageH)
{
    const float refPitchPx =
        static_cast<float>(kHuaxingRefPitchUmX / static_cast<float>(kHuaxingRefUmPerPx));

    auto toIntPx = [](float v) { return static_cast<int>(std::lround(v)); };

    if (detectImageW > 0 && detectImageH > 0) {
        LOG_INFO("[VirtualInspect] {} === 閫佹鍥惧儚锛堝師鍥撅紝鏃犵缉鏀撅級{}x{} px ===", tag, detectImageW,
                 detectImageH);
    }

    LOG_INFO(
        "[VirtualInspect] {} === CISDK DieBonding DL 鍏ュ弬 (px) ===\n"
        "  umPerPx={:.4f} engine={}\n"
        "  CISDK int: solder={}x{} chip={}x{} group={}x{} pitch={}x{}\n"
        "  CISDK float: solder={:.2f}x{:.2f} chip={:.2f}x{:.2f} group={:.2f}x{:.2f} "
        "pitch={:.2f}x{:.2f}\n"
        "  chip_pitch_in_group={:.3f} mm align={} conf_chip={:.3f} conf_group={:.3f} "
        "chip_lim={:.3f} solder_lim={:.3f}\n"
        "  models: group={} detect={} polar={} classify={}",
        tag, umPerPx, param.dlInferenceEngine, toIntPx(param.solderWidth),
        toIntPx(param.solderHeight), toIntPx(param.chipWidth), toIntPx(param.chipHeight),
        toIntPx(param.chipGroupWidth), toIntPx(param.chipGroupHeight), toIntPx(param.ledGapX),
        toIntPx(param.ledGapY), param.solderWidth, param.solderHeight, param.chipWidth,
        param.chipHeight, param.chipGroupWidth, param.chipGroupHeight, param.ledGapX, param.ledGapY,
        param.chipPitchInGroupMm, param.alignType, param.confThr, param.confGroupThr,
        param.chipLimThr, param.solderLimThr, param.chipGroupModel, param.chipDetectModel,
        param.polarDetectModel, param.detectClassifyModel);

    LOG_INFO(
        "[VirtualInspect] {} === 鍗庢槦鍙傝€?@ {:.2f} 渭m/px (鎴浘/demo) ===\n"
        "  solder={:.0f}x{:.0f} chip={:.0f}x{:.0f} group={:.0f}x{:.0f} pitch={:.1f}x{:.1f} px "
        "({:.0f}x{:.0f} 渭m)",
        tag, kHuaxingRefUmPerPx, kHuaxingRefSolderW, kHuaxingRefSolderH, kHuaxingRefChipW,
        kHuaxingRefChipH, kHuaxingRefGroupW, kHuaxingRefGroupH, refPitchPx, refPitchPx,
        kHuaxingRefPitchUmX, kHuaxingRefPitchUmY);

    if (umPerPx > 0.0 && std::fabs(umPerPx - kHuaxingRefUmPerPx) > 1e-3) {
        const double scale = kHuaxingRefUmPerPx / umPerPx;
        LOG_INFO(
            "[VirtualInspect] {} === 鍗庢槦鍙傝€冩寜褰撳墠 um/px 绛夋瘮缂╂斁 (scale={:.4f}) ===\n"
            "  solder={:.1f}x{:.1f} chip={:.1f}x{:.1f} group={:.1f}x{:.1f} pitch={:.1f}x{:.1f} px",
            tag, scale, kHuaxingRefSolderW * static_cast<float>(scale),
            kHuaxingRefSolderH * static_cast<float>(scale), kHuaxingRefChipW * static_cast<float>(scale),
            kHuaxingRefChipH * static_cast<float>(scale), kHuaxingRefGroupW * static_cast<float>(scale),
            kHuaxingRefGroupH * static_cast<float>(scale), refPitchPx * static_cast<float>(scale),
            refPitchPx * static_cast<float>(scale));
    }

    if (recipe) {
        LOG_INFO(
            "[VirtualInspect] {} === 閰嶆柟婧愭暟鎹?(mm / 渭m) ===\n"
            "  camera: {}x{} capture_accuracy={:.4f} 渭m/px\n"
            "  pad={:.5f}x{:.5f} mm chip={:.5f}x{:.5f} mm group={:.5f}x{:.5f} mm\n"
            "  group_pitch={:.5f}x{:.5f} mm ({:.0f}x{:.0f} 渭m) chip_pitch_in_group={:.3f} mm\n"
            "  algo: offset=({:.1f},{:.1f}) 渭m angle={:.3f}掳 conf chip={:.3f} group={:.3f}",
            tag, recipe->camera.imageWidth, recipe->camera.imageHeight,
            recipe->camera.captureAccuracy, recipe->basic.padWidthMm, recipe->basic.padHeightMm,
            recipe->basic.chipWidthMm, recipe->basic.chipHeightMm, recipe->basic.groupWidthMm,
            recipe->basic.groupHeightMm, recipe->basic.groupPitchXmm, recipe->basic.groupPitchYmm,
            recipe->basic.groupPitchXmm * 1000.0, recipe->basic.groupPitchYmm * 1000.0,
            recipe->basic.chipPitchInGroupMm, recipe->algo.offsetXUm, recipe->algo.offsetYUm,
            recipe->algo.angleOffsetDeg, recipe->algo.chipConfidence, recipe->algo.groupConfidence);
    }
}

bool buildDieBondingDetectParam(const RecipeDocument &recipe,
                                vision_algo::DieBondingChipDetectParam *outParam,
                                QString *errorOut)
{
    if (!outParam)
        return false;

    const double umPerPx = recipeCaptureUmPerPixel(recipe);

    vision_algo::DieBondingChipDetectParam param;
    param.solderWidth        = mmToPx(recipe.basic.padWidthMm, umPerPx);
    param.solderHeight       = mmToPx(recipe.basic.padHeightMm, umPerPx);
    param.chipWidth          = mmToPx(recipe.basic.chipWidthMm, umPerPx);
    param.chipHeight         = mmToPx(recipe.basic.chipHeightMm, umPerPx);
    param.chipGroupWidth     = mmToPx(recipe.basic.groupWidthMm, umPerPx);
    param.chipGroupHeight    = mmToPx(recipe.basic.groupHeightMm, umPerPx);
    param.ledGapX            = mmToPx(recipe.basic.groupPitchXmm, umPerPx);
    param.ledGapY            = mmToPx(recipe.basic.groupPitchYmm, umPerPx);
    param.chipPitchInGroupMm = static_cast<float>(recipe.basic.chipPitchInGroupMm);
    param.confThr            = static_cast<float>(recipe.algo.chipConfidence);
    param.confGroupThr       = static_cast<float>(recipe.algo.groupConfidence);
    param.chipLimThr         = static_cast<float>(recipe.algo.chipDefectClass);
    param.solderLimThr       = static_cast<float>(recipe.algo.padDefectClass);
    param.dlInferenceEngine  = 0;

    if (auto *svc = algoService()) {
        vision_algo::DieBondingChipDetectParam dlParam = param;
        dlParam.dlInferenceEngine                      = 2;
        vision_algo::DieBondingChipDetectInput inPkt;
        inPkt.setOp(vision_algo::DieBondingChipDetectOp::ScanDlModels);
        inPkt.setParam(dlParam);
        inPkt.setDirectory(std::string{});
        inPkt.setPreferOnnx(false);
        vision_algo::DieBondingChipDetectOutput outPkt;
        QString scanError;
        if (runDieBondingExecute(inPkt, &outPkt, &scanError)) {
            if (auto *scanned = outPkt.param())
                param = *scanned;
        }
    }

    *outParam = param;
    return true;
}

bool initDieBondingEngine(const vision_algo::DieBondingChipDetectParam &param, std::size_t poolSize,
                          int dlInferenceEngineOverride, QString *errorOut)
{
    vision_algo::DieBondingChipDetectInput inPkt;
    inPkt.setOp(vision_algo::DieBondingChipDetectOp::InitEngine);
    inPkt.setParam(param);
    inPkt.setPoolSize(poolSize);
    inPkt.setDlInferenceEngineOverride(dlInferenceEngineOverride);
    vision_algo::DieBondingChipDetectOutput outPkt;
    if (!runDieBondingExecute(inPkt, &outPkt, errorOut))
        return false;
    return outPkt.ready() && *outPkt.ready();
}

bool syncDieBondingEngineParam(const vision_algo::DieBondingChipDetectParam &param,
                               QString *errorOut)
{
    vision_algo::DieBondingChipDetectInput inPkt;
    inPkt.setOp(vision_algo::DieBondingChipDetectOp::SyncParam);
    inPkt.setParam(param);
    return runDieBondingExecute(inPkt, nullptr, errorOut);
}

ChipDetectOutcome detectFovProcessImage(const RecipeDocument &recipe,
                                        std::shared_ptr<common::device::ImageInfo> processImage,
                                        int fovIndex, bool useDeepLearning, bool dlCropLocate)
{
    ChipDetectOutcome outcome;
    outcome.record.fovIndex = fovIndex;

    if (!isRgbProcessImage(processImage)) {
        outcome.error = QStringLiteral("FOV 处理图为空");
        return outcome;
    }

    vision_algo::DieBondingChipDetectInput::DetectFields fields;
    auto &vision        = fields.vision;
    vision.iImageIndex  = fovIndex;
    vision.imageIndex   = std::to_string(fovIndex);
    fields.isDeepLearning   = useDeepLearning;
    fields.isPolarityDetect = true;
    fields.isDefectClassify = false;
    fields.skewX            = recipe.algo.offsetXUm;
    fields.skewY            = recipe.algo.offsetYUm;
    fields.skewA            = recipe.algo.angleOffsetDeg;
    fields.pixelSize        = recipeCaptureUmPerPixel(recipe);
    vision.processImage     = processImage;
    fields.dlCropLocate     = dlCropLocate;

    const int matW = processImage->imageMat->cols;
    const int matH = processImage->imageMat->rows;
    {
        const double umPerPx = recipeCaptureUmPerPixel(recipe);
        vision_algo::DieBondingChipDetectParam detectParam;
        if (buildDieBondingDetectParam(recipe, &detectParam, nullptr))
            logDieBondingDetectParam("detectFovProcessImage", detectParam, umPerPx, &recipe, matW,
                                     matH);
    }
    LOG_INFO(
        "[VirtualInspect] detectFovProcessImage runtime: fov={} dl={} polarity={} classify={} "
        "cropLocate={} pixelSizeUm={:.4f} skew=({:.1f},{:.1f}) deg={:.3f} image={}x{}",
        fovIndex, useDeepLearning, fields.isPolarityDetect, fields.isDefectClassify,
        fields.dlCropLocate, fields.pixelSize, fields.skewX, fields.skewY, fields.skewA, matW,
        matH);

    vision_algo::DieBondingChipDetectInput inPkt;
    inPkt.setOp(vision_algo::DieBondingChipDetectOp::Detect);
    inPkt.setDetectFields(std::move(fields));
    vision_algo::DieBondingChipDetectOutput outPkt;
    if (!runDieBondingExecute(inPkt, &outPkt, &outcome.error)) {
        LOG_ERROR("[VirtualInspect] execute failed: {}", outcome.error.toStdString());
        return outcome;
    }

    auto *detectOut = outPkt.detectResult();
    if (!detectOut) {
        outcome.error = QStringLiteral("检测无输出");
        return outcome;
    }

    LOG_INFO("[VirtualInspect] DieBondingChipDetect::execute pass groupNum={}",
             detectOut->result.chipGroupNum);

    outcome.record.ok       = detectOut->ok();
    outcome.record.groupNum = detectOut->result.chipGroupNum;
    outcome.record.groups   = std::move(detectOut->result.chipGroupInfos);
    outcome.ok              = true;
    return outcome;
}

} // namespace aoi
