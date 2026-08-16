#include "algorithm/demo/die_bonding_dl_demo.h"

#include "adapter/dl_model_scanner.h"
#include "chipdetector/src/die_bonding_chip_detect_data.h"

#include <algo_mat.h>
#include <ci_basic_error.h>
#include <ci_utils_algomat.h>
#ifdef ENABLE_DL_BONDING_DETECT
#include <dl_die_bonding_detector.h>
#endif

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <QTextStream>

#include <cstdio>

namespace aoi {
namespace demo {
namespace {

void logStep(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    std::fflush(stderr);
}

void logStep(const QString &message) { logStep(message.toUtf8().constData()); }

QTextStream &out()
{
    static QTextStream stream(stdout);
    static bool        initialized = [] {
        if (QTextCodec *codec = QTextCodec::codecForName("UTF-8"))
            stream.setCodec(codec);
        return true;
    }();
    (void)initialized;
    return stream;
}

QTextStream &err()
{
    static QTextStream stream(stderr);
    static bool        initialized = [] {
        if (QTextCodec *codec = QTextCodec::codecForName("UTF-8"))
            stream.setCodec(codec);
        return true;
    }();
    (void)initialized;
    return stream;
}

struct DemoConfig
{
    float solderWidth       = 109.f;
    float solderHeight      = 45.f;
    float chipGroupWidth    = 109.f;
    float chipGroupHeight   = 175.f;
    float chipWidth         = 51.f;
    float chipHeight        = 20.f;
    float ledGapUmX         = 1250.f;
    float ledGapUmY         = 1250.f;
    float pixelSizeUm       = 2.95f;
    int   alignType         = 0;
    float confThr           = 0.4f;
    float confGroupThr      = 0.6f;
    float chipLimThr        = 0.8f;
    float solderLimThr      = 0.8f;
    bool  isDefectClassify  = false;
    bool  isPolarityDetect  = true;
    int   dlInferenceEngine = 2;
};

bool loadDemoConfigFile(const QString &path, DemoConfig *cfg, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("无法打开配置: %1").arg(path);
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        if (error)
            *error = QStringLiteral("配置 JSON 无效: %1").arg(path);
        return false;
    }
    const QJsonObject root = doc.object();
    auto readFloat = [&](const char *key, float &dst) {
        if (root.contains(QLatin1String(key)))
            dst = static_cast<float>(root.value(QLatin1String(key)).toDouble(dst));
    };
    auto readInt = [&](const char *key, int &dst) {
        if (root.contains(QLatin1String(key)))
            dst = root.value(QLatin1String(key)).toInt(dst);
    };
    auto readBool = [&](const char *key, bool &dst) {
        if (root.contains(QLatin1String(key)))
            dst = root.value(QLatin1String(key)).toBool(dst);
    };

    readFloat("solderWidth", cfg->solderWidth);
    readFloat("solderHeight", cfg->solderHeight);
    readFloat("chipGroupWidth", cfg->chipGroupWidth);
    readFloat("chipGroupHeight", cfg->chipGroupHeight);
    readFloat("chipWidth", cfg->chipWidth);
    readFloat("chipHeight", cfg->chipHeight);
    readFloat("ledGapX", cfg->ledGapUmX);
    readFloat("ledGapY", cfg->ledGapUmY);
    if (root.contains(QStringLiteral("pixeSize")))
        cfg->pixelSizeUm = static_cast<float>(root.value(QStringLiteral("pixeSize")).toDouble());
    if (root.contains(QStringLiteral("pixelSize")))
        cfg->pixelSizeUm = static_cast<float>(root.value(QStringLiteral("pixelSize")).toDouble());
    readInt("alignType", cfg->alignType);
    readFloat("confThr", cfg->confThr);
    readFloat("confGroupThr", cfg->confGroupThr);
    readFloat("chipLimThr", cfg->chipLimThr);
    readFloat("solderLimThr", cfg->solderLimThr);
    readBool("isDefectClassify", cfg->isDefectClassify);
    readBool("isPolarityDetect", cfg->isPolarityDetect);
    readInt("dlInferenceEngine", cfg->dlInferenceEngine);
    return true;
}

QString defaultDemoConfigPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("demo_config.json"));
}

float readPxOption(const QCommandLineParser &parser, const QString &name, float fallback)
{
    if (!parser.isSet(name))
        return fallback;
    return parser.value(name).toFloat();
}

float umPitchToPx(float um, float pixelSizeUm)
{
    if (pixelSizeUm <= 0.f)
        return 0.f;
    return um / pixelSizeUm;
}

struct DetectRuntime
{
    vision_algo::DieBondingChipDetectParam param;
    bool                                         isDefectClassify = false;
    bool                                         isPolarityDetect = true;
};

DetectRuntime buildRuntime(const QCommandLineParser &parser, const DemoConfig &defaults)
{
    DemoConfig cfg = defaults;
    if (parser.isSet(QStringLiteral("config"))) {
        QString configError;
        loadDemoConfigFile(parser.value(QStringLiteral("config")), &cfg, &configError);
    }
    if (parser.isSet(QStringLiteral("pixel-size")))
        cfg.pixelSizeUm = static_cast<float>(parser.value(QStringLiteral("pixel-size")).toDouble());
    if (parser.isSet(QStringLiteral("capture-accuracy")))
        cfg.pixelSizeUm =
            static_cast<float>(parser.value(QStringLiteral("capture-accuracy")).toDouble());

    DetectRuntime rt;
    rt.param.solderWidth     = readPxOption(parser, QStringLiteral("solder-width"), cfg.solderWidth);
    rt.param.solderHeight    = readPxOption(parser, QStringLiteral("solder-height"), cfg.solderHeight);
    rt.param.chipWidth       = readPxOption(parser, QStringLiteral("chip-width"), cfg.chipWidth);
    rt.param.chipHeight      = readPxOption(parser, QStringLiteral("chip-height"), cfg.chipHeight);
    rt.param.chipGroupWidth  = readPxOption(parser, QStringLiteral("chip-group-width"),
                                           readPxOption(parser, QStringLiteral("chip-group-res-x"),
                                                        cfg.chipGroupWidth));
    rt.param.chipGroupHeight = readPxOption(parser, QStringLiteral("chip-group-height"),
                                            readPxOption(parser, QStringLiteral("chip-group-res-y"),
                                                         cfg.chipGroupHeight));

    const float ledGapUmX = parser.isSet(QStringLiteral("group-pitch-x"))
                                ? parser.value(QStringLiteral("group-pitch-x")).toFloat()
                            : parser.isSet(QStringLiteral("led-gap-x"))
                                ? parser.value(QStringLiteral("led-gap-x")).toFloat()
                                : cfg.ledGapUmX;
    const float ledGapUmY = parser.isSet(QStringLiteral("group-pitch-y"))
                                ? parser.value(QStringLiteral("group-pitch-y")).toFloat()
                            : parser.isSet(QStringLiteral("led-gap-y"))
                                ? parser.value(QStringLiteral("led-gap-y")).toFloat()
                                : cfg.ledGapUmY;
    rt.param.ledGapX = umPitchToPx(ledGapUmX, cfg.pixelSizeUm);
    rt.param.ledGapY = umPitchToPx(ledGapUmY, cfg.pixelSizeUm);

    rt.param.alignType = parser.isSet(QStringLiteral("align-type"))
                             ? parser.value(QStringLiteral("align-type")).toInt()
                             : cfg.alignType;
    rt.param.confThr   = parser.isSet(QStringLiteral("conf-thr"))
                             ? parser.value(QStringLiteral("conf-thr")).toFloat()
                             : cfg.confThr;
    rt.param.confGroupThr = parser.isSet(QStringLiteral("conf-group-thr"))
                                ? parser.value(QStringLiteral("conf-group-thr")).toFloat()
                                : cfg.confGroupThr;
    rt.param.chipLimThr = parser.isSet(QStringLiteral("chip-lim-thr"))
                              ? parser.value(QStringLiteral("chip-lim-thr")).toFloat()
                              : cfg.chipLimThr;
    rt.param.solderLimThr = parser.isSet(QStringLiteral("solder-lim-thr"))
                                ? parser.value(QStringLiteral("solder-lim-thr")).toFloat()
                                : cfg.solderLimThr;
    rt.param.dlInferenceEngine = parser.isSet(QStringLiteral("dl-engine"))
                                     ? parser.value(QStringLiteral("dl-engine")).toInt()
                                     : cfg.dlInferenceEngine;

    rt.isDefectClassify = parser.isSet(QStringLiteral("defect-classify"))
                              ? true
                              : parser.isSet(QStringLiteral("no-defect-classify"))
                                  ? false
                                  : cfg.isDefectClassify;
    rt.isPolarityDetect = parser.isSet(QStringLiteral("polarity-detect"))
                              ? true
                              : parser.isSet(QStringLiteral("no-polarity-detect"))
                                  ? false
                                  : cfg.isPolarityDetect;
    return rt;
}

void printDetectParam(const vision_algo::DieBondingChipDetectParam &param, float pixelSizeUm)
{
    out() << "panel param (CISDK DL 单位: px):\n";
    out() << "  capture_accuracy: " << pixelSizeUm << " um/px\n";
    out() << "  solder (px): " << param.solderWidth << " x " << param.solderHeight << "\n";
    out() << "  chip (px): " << param.chipWidth << " x " << param.chipHeight << "\n";
    out() << "  chip_group (px): " << param.chipGroupWidth << " x " << param.chipGroupHeight << "\n";
    out() << "  group_pitch (px): " << param.ledGapX << " x " << param.ledGapY << "  ["
          << param.ledGapX * pixelSizeUm << " x " << param.ledGapY * pixelSizeUm << " um]\n";
    out() << "  align_type: " << param.alignType << "\n";
    out() << "  conf: chip=" << param.confThr << " group=" << param.confGroupThr
          << " chip_lim=" << param.chipLimThr << " solder_lim=" << param.solderLimThr << "\n";
}

void printGroupSummary(int groupNum, const DieBondingChipGroupInfo *groups)
{
    out() << "chip groups: " << groupNum << "\n";
    const char *channelNames[] = {"R", "G", "B"};
    for (int gi = 0; gi < groupNum; ++gi) {
        const auto &group = groups[gi];
        out() << "group[" << gi << "] row=" << group.row << " col=" << group.col
              << " center=(" << group.coordinate.x << ", " << group.coordinate.y << ")\n";
        for (int ci = 0; ci < 3; ++ci) {
            const auto &chip = group.chip_rgb_info[ci];
            out() << "  " << channelNames[ci] << ": defect=0x" << Qt::hex << chip.chip_defect
                  << Qt::dec << " polar=" << chip.polar_type
                  << " angle=" << chip.chip_angle << "\n";
        }
    }
}

#ifdef ENABLE_DL_BONDING_DETECT
bool initDlHandle(const DetectRuntime &rt, AlgoHandle *handle, QString *error)
{
    const auto &p = rt.param;
    AlgoHandle  h = nullptr;
    AlgoError   err = DLDieBondingCreateHandle(&h);
    if (err != ALGO_PASS || !h) {
        if (error)
            *error = QStringLiteral("DLDieBondingCreateHandle failed");
        return false;
    }

    err = DLDieBondingLoadModel(h, p.chipGroupModel.c_str(), p.chipGroupKey.c_str(),
                                CHIP_GROUP_LOCATE_MODEL, 4, p.dlInferenceEngine);
    if (err != ALGO_PASS)
        goto fail;
    err = DLDieBondingLoadModel(h, p.chipDetectModel.c_str(), p.chipDetectKey.c_str(),
                                CHIP_DETECT_MODEL, 64, p.dlInferenceEngine);
    if (err != ALGO_PASS)
        goto fail;
    // 与 DLDieBondingDetect 开关一致：未启用时不加载，减少 GPU 显存占用
    if (rt.isDefectClassify && !p.detectClassifyModel.empty()) {
        err = DLDieBondingLoadModel(h, p.detectClassifyModel.c_str(), p.detectClassifyKey.c_str(),
                                    CHIP_DEFECT_CLASSIFY_MODEL, 64, p.dlInferenceEngine);
        if (err != ALGO_PASS)
            goto fail;
    }
    if (rt.isPolarityDetect && !p.polarDetectModel.empty()) {
        err = DLDieBondingLoadModel(h, p.polarDetectModel.c_str(), p.polarDetectKey.c_str(),
                                    CHIP_POLARITY_CLASSIFY_MODEL, 64, p.dlInferenceEngine);
        if (err != ALGO_PASS)
            goto fail;
    }

    {
        DLDieBondingParameter para{{p.solderWidth, p.solderHeight},
                                     {p.chipWidth, p.chipHeight},
                                     {p.chipGroupWidth, p.chipGroupHeight},
                                     {p.ledGapX, p.ledGapY},
                                     static_cast<DieBondingChipAlignType>(p.alignType)};
        err = DLDieBondingSetParameter(h, &para);
        if (err != ALGO_PASS)
            goto fail;
    }
    err = DLDieBondingSetGroupConfThresh(h, p.confGroupThr);
    if (err != ALGO_PASS)
        goto fail;
    err = DLDieBondingSetChipConfThresh(h, p.confThr);
    if (err != ALGO_PASS)
        goto fail;
    err = DLDieBondingSetDefectThresh(h, p.chipLimThr, p.solderLimThr);
    if (err != ALGO_PASS)
        goto fail;
    err = DLDieBondingSetImageIndex(h, "aoi_algo_detect_demo");
    if (err != ALGO_PASS)
        goto fail;

    *handle = h;
    return true;

fail:
    if (error)
        *error = QStringLiteral("init DL handle failed, alg_module=%1 alg_code=%2")
                     .arg(ERROR_MODULE_NAME(err))
                     .arg(ERROR_CODE_NO_MODULE(err));
    DLDieBondingDestroyHandle(h);
    return false;
}

bool detectImageFile(AlgoHandle handle, const QString &imagePath, const DetectRuntime &rt,
                     const QString &outPath, QString *error)
{
    AlgoMat *img = AlgoMatNew();
    if (!img) {
        if (error)
            *error = QStringLiteral("AlgoMatNew failed");
        return false;
    }

    logStep(QStringLiteral("AlgoMatReadCvImg..."));
    const QByteArray pathBytes = QFileInfo(imagePath).absoluteFilePath().toUtf8();
    AlgoError        err     = AlgoMatReadCvImg(img, pathBytes.constData());
    if (err != ALGO_PASS) {
        if (error)
            *error = QStringLiteral("AlgoMatReadCvImg failed");
        AlgoMatDelete(img);
        return false;
    }

    logStep(QString("image mat: %1x%2").arg(img->cols).arg(img->rows));

    int row = 0;
    int col = 0;
    logStep(QStringLiteral("DLDieBondingChipGroupLocate..."));
    err     = DLDieBondingChipGroupLocate(handle, img, &row, &col, true);
    if (err != ALGO_PASS) {
        if (error)
            *error = QStringLiteral("DLDieBondingChipGroupLocate failed");
        AlgoMatDelete(img);
        return false;
    }
    out() << "locate grid: row=" << row << " col=" << col << "\n";
    AlgoMatDelete(img);

    err = DLDieBondingDetect(handle, rt.isDefectClassify, rt.isPolarityDetect, false);
    if (err != ALGO_PASS) {
        if (error)
            *error = QStringLiteral("DLDieBondingDetect failed");
        return false;
    }

    int groupNum = 0;
    err          = DLDieBondingGetGroupNum(handle, &groupNum);
    if (err != ALGO_PASS || groupNum <= 0) {
        if (error)
            *error = QStringLiteral("DLDieBondingGetGroupNum failed or zero groups");
        return false;
    }

    DieBondingChipGroupInfo *groups = new DieBondingChipGroupInfo[static_cast<size_t>(groupNum)];
    err = DLDieBondingGetAllChipGroupInfo(handle, groups);
    if (err != ALGO_PASS) {
        delete[] groups;
        if (error)
            *error = QStringLiteral("DLDieBondingGetAllChipGroupInfo failed");
        return false;
    }

    printGroupSummary(groupNum, groups);
    delete[] groups;

    if (!outPath.isEmpty()) {
        const QByteArray outBytes = QFileInfo(outPath).absoluteFilePath().toUtf8();
        err                       = DLDieBondingExportDetectResult(handle, outBytes.constData());
        if (err != ALGO_PASS) {
            if (error)
                *error = QStringLiteral("DLDieBondingExportDetectResult failed");
            return false;
        }
    }
    return true;
}
#endif

} // namespace

int listDlModels(const QString &dlModelDir)
{
    vision_algo::DieBondingChipDetectParam param;
    QStringList                                  found;
    QString                                      scanError;
    out() << "dlmodel directory: " << (dlModelDir.isEmpty() ? QStringLiteral("(default)") : dlModelDir)
          << "\n";
    if (!vision_algo::scanDlModelDirectory(dlModelDir, param, &found, &scanError)) {
        err() << scanError << "\n";
        return 1;
    }

    for (const QString &name : found)
        out() << "  - " << name << "\n";

    out() << "chip_group: " << QString::fromStdString(param.chipGroupModel) << "\n";
    out() << "chip_detect: " << QString::fromStdString(param.chipDetectModel) << "\n";
    if (!param.polarDetectModel.empty())
        out() << "polar_detect: " << QString::fromStdString(param.polarDetectModel) << "\n";
    if (!param.detectClassifyModel.empty())
        out() << "defect_cls: " << QString::fromStdString(param.detectClassifyModel) << "\n";
    return 0;
}

int runDieBondingDl(const QCommandLineParser &parser)
{
#ifndef ENABLE_DL_BONDING_DETECT
    (void)parser;
    err() << "die-bonding-dl 需要 Release 构建（dl_die_bonding_detector 暂无 Debug 库）\n";
    return 1;
#else
    logStep(QStringLiteral("[1/4] 加载检测图像..."));
    const QString imagePath  = parser.value(QStringLiteral("image"));
    const QString dlModelDir = parser.value(QStringLiteral("dlmodel-dir"));
    const QString outPath    = parser.value(QStringLiteral("out"));
    if (imagePath.isEmpty() || !QFileInfo::exists(imagePath)) {
        err() << "image not found\n";
        return 1;
    }

    DemoConfig defaults;
    const QString bundledConfig = defaultDemoConfigPath();
    if (QFileInfo::exists(bundledConfig))
        loadDemoConfigFile(bundledConfig, &defaults, nullptr);

    DetectRuntime rt = buildRuntime(parser, defaults);

    logStep(QStringLiteral("[2/4] 扫描 dlmodel..."));
    QString scanError;
    if (!vision_algo::scanDlModelDirectory(dlModelDir, rt.param, nullptr, &scanError, false)) {
        err() << scanError << "\n";
        return 1;
    }

    out() << "image: " << imagePath << "\n";
    out() << "dlmodel: " << modelDir << "\n";
    out() << "inference engine: " << rt.param.dlInferenceEngine << "\n";
    out() << "defect_classify: " << (rt.isDefectClassify ? "on" : "off")
          << "  polarity_detect: " << (rt.isPolarityDetect ? "on" : "off") << "\n";
    printDetectParam(rt.param, defaults.pixelSizeUm);
    out().flush();

    logStep(QStringLiteral("[3/4] 初始化句柄（华星 demo_2 直连 CISDK）..."));
    AlgoHandle handle = nullptr;
    QString    initError;
    if (!initDlHandle(rt, &handle, &initError)) {
        err() << initError << "\n";
        return 1;
    }
    logStep(QStringLiteral("[3/4] 模型与参数就绪"));

    logStep(QStringLiteral("[4/4] 检测..."));
    QString detectError;
    if (!detectImageFile(handle, imagePath, rt, outPath, &detectError)) {
        err() << detectError << "\n";
        DLDieBondingRelease(handle);
        DLDieBondingDestroyHandle(handle);
        return 1;
    }

    DLDieBondingRelease(handle);
    DLDieBondingDestroyHandle(handle);
    if (!outPath.isEmpty())
        out() << "result image: " << outPath << "\n";
    out() << "done.\n";
    return 0;
#endif
}

} // namespace demo
} // namespace aoi
