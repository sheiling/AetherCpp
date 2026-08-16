/**
 * aoi_algo_detect_demo — 华星 CISDK 算法联调控制台
 *
 * 不依赖配方文件、工艺注册或 UI，仅演示：
 *   - SDK 是否可用
 *   - 模型库枚举
 *   - Mark 点定位（AlgorithmManager + MarkPointLocator）
 *   - 固晶深度学习检测（HandlePool + DieBondingChipDetect）
 *
 * 用法示例：
 *   aoi_algo_detect_demo sdk-check
 *   aoi_algo_detect_demo list-dlmodels --dlmodel-dir D:/.../bin/Release/dlmodel
 *   aoi_algo_detect_demo die-bonding-dl -i panel.png --dlmodel-dir D:/.../bin/Release/dlmodel -o result.png
 *   aoi_algo_detect_demo mark -i capture.png --model-file D:/models/mark.model --model-name demo1
 */

#include "algorithm/demo/die_bonding_dl_demo.h"
#include "adapter/dl_model_scanner.h"
#include "algorithm/ess/algorithm_manager.h"
#include "algorithm/qt_bridge/mark_point_locator.h"
#include "algorithm/qt_bridge/model_generator.h"
#include "adapter/cisdk_image_utils.h"

#include <common/base/imgproc/image_type_map.h>
#include <common/core/device/imageCapture/algo_mat_shared.h>
#include <common/core/device/imageCapture/image_capture.h>

#include <vision_algo/units/marker/mark_point_locate_data.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <memory>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>

#if defined(_WIN32)
#include <windows.h>

void setupExeDllDirectory()
{
    wchar_t path[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return;
    wchar_t *slash = wcsrchr(path, L'\\');
    if (!slash)
        return;
    *slash = L'\0';
    SetDllDirectoryW(path);
}
#endif

namespace {

QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

QTextStream &err()
{
    static QTextStream stream(stderr);
    return stream;
}

void setupConsoleUtf8()
{
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    static bool streamsReady = false;
    if (streamsReady)
        return;
    streamsReady = true;
    if (QTextCodec *codec = QTextCodec::codecForName("UTF-8")) {
        QTextCodec::setCodecForLocale(codec);
        out().setCodec(codec);
        err().setCodec(codec);
    }
}

int runSdkCheck()
{
    out() << "=== CISDK / AlgorithmManager ===\n";
    out() << "model_processor: " << (aoi::ModelGenerator::sdkReady() ? "OK" : "FAIL") << "\n";
    if (!aoi::ModelGenerator::sdkReady())
        err() << "  detail: " << aoi::ModelGenerator::sdkLoadDetail() << "\n";

    out() << "mark detector: " << (aoi::MarkPointLocator::sdkReady() ? "OK" : "FAIL") << "\n";
    if (!aoi::MarkPointLocator::sdkReady())
        err() << "  detail: " << aoi::MarkPointLocator::sdkLoadDetail() << "\n";

    aoi::ess::AlgorithmManager::instance().init();
    const bool markHandler = aoi::ess::AlgorithmManager::instance().handler("MarkPointLocate") != nullptr;
    out() << "handler \"MarkPointLocate\": " << (markHandler ? "registered" : "missing") << "\n";

    const bool dieHandler =
        aoi::ess::AlgorithmManager::instance().handler("DieBondingChipDetect") != nullptr;
    out() << "handler \"DieBondingChipDetect\": " << (dieHandler ? "registered" : "missing")
          << "\n";

    return aoi::MarkPointLocator::sdkReady() ? 0 : 1;
}

int runListModels(const QString &directory)
{
    QMap<QString, QStringList> models;
    const aoi::ModelOpResult   result = aoi::ModelGenerator::listAllModels(models, directory);
    if (!result.ok) {
        err() << "list-models failed: " << result.errorKey << "\n";
        return 1;
    }

    const QString dir = directory.isEmpty() ? aoi::ModelGenerator::defaultModelDir() : directory;
    out() << "model directory: " << dir << "\n";
    if (models.isEmpty()) {
        out() << "(no .model files found)\n";
        return 0;
    }

    for (auto it = models.constBegin(); it != models.constEnd(); ++it) {
        out() << QFileInfo(it.key()).fileName() << ":\n";
        for (const QString &name : it.value())
            out() << "  - " << name << "\n";
    }
    return 0;
}

vision_algo::MarkerType parseMarkType(const QString &text)
{
    const QString t = text.trimmed().toLower();
    if (t == QLatin1String("cross") || t == QLatin1String("2"))
        return vision_algo::MarkerType::Cross;
    if (t == QLatin1String("rect") || t == QLatin1String("3"))
        return vision_algo::MarkerType::Rect;
    if (t == QLatin1String("ring") || t == QLatin1String("4"))
        return vision_algo::MarkerType::Ring;
    if (t == QLatin1String("false_cross") || t == QLatin1String("false-cross") || t == QLatin1String("5"))
        return vision_algo::MarkerType::FalseCross;
    if (t == QLatin1String("rect_corner") || t == QLatin1String("rect-corner") || t == QLatin1String("6"))
        return vision_algo::MarkerType::RectCorner;
    if (t == QLatin1String("line") || t == QLatin1String("7"))
        return vision_algo::MarkerType::Line;
    return vision_algo::MarkerType::Circle;
}

uint16_t parseIsWhiteMark(const QString &text)
{
    const QString t = text.trimmed().toLower();
    if (t == QLatin1String("white"))
        return 1;
    return 0;
}

std::shared_ptr<common::device::ImageInfo> loadRgbImageInfoFromFile(const QString &path)
{
    const cv::Mat bgr = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    if (bgr.empty())
        return nullptr;

    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);

    const int        w   = rgb.cols;
    const int        h   = rgb.rows;
    const size_t     len = static_cast<size_t>(w) * h * 3u;
    const QByteArray buffer(reinterpret_cast<const char *>(rgb.data),
                            static_cast<int>(len));

    AlgoMat mat = AlgoMatCreateFromMem(h, w, ALGO_8UC3, buffer.constData(),
                                       static_cast<unsigned int>(len));
    if (AlgoMatIsEmpty(&mat))
        return nullptr;

    auto out         = std::make_shared<common::device::ImageInfo>();
    out->imageMat    = common::device::adoptAlgoMat(std::move(mat));
    out->imageFormat = common::imgproc::ImageFormat::RGB24;
    out->bayerType   = COLOR_BayerNone;
    return out;
}

bool saveRgbImageInfoToFile(const std::shared_ptr<common::device::ImageInfo> &info,
                            const QString &path)
{
    if (!aoi::isValidRgbImageInfo(info))
        return false;

    const AlgoMat &mat = *info->imageMat;
    cv::Mat        rgb(mat.rows, mat.cols, CV_8UC3, mat.data);
    cv::Mat        bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
    return cv::imwrite(path.toStdString(), bgr);
}

int runMarkDetect(const QCommandLineParser &parser)
{
    const QString imagePath   = parser.value(QStringLiteral("image"));
    const QString modelFile   = parser.value(QStringLiteral("model-file"));
    const QString modelName   = parser.value(QStringLiteral("model-name"));
    const QString outPath     = parser.value(QStringLiteral("out"));
    const QString markType    = parser.value(QStringLiteral("mark-type"));
    const QString background  = parser.value(QStringLiteral("background"));

    if (imagePath.isEmpty() || modelFile.isEmpty() || modelName.isEmpty()) {
        err() << "mark 需要 --image、--model-file、--model-name\n";
        return 2;
    }

    if (!QFileInfo::exists(imagePath)) {
        err() << "image not found: " << imagePath << "\n";
        return 1;
    }
    if (!QFileInfo::exists(modelFile)) {
        err() << "model file not found: " << modelFile << "\n";
        return 1;
    }

    auto processImage = loadRgbImageInfoFromFile(imagePath);
    if (!processImage) {
        err() << "failed to load image: " << imagePath << "\n";
        return 1;
    }

    aoi::ess::AlgorithmManager::instance().init();

    aoi::MarkPointConfig config;
    config.marker.modelFileName = modelFile.toStdString();
    config.marker.modelName     = modelName.toStdString();
    config.marker.markType      = parseMarkType(markType);
    config.marker.markWidth     = parser.value(QStringLiteral("mark-width")).toInt();
    config.marker.markHeight    = parser.value(QStringLiteral("mark-height")).toInt();
    config.marker.area          = static_cast<float>(parser.value(QStringLiteral("area-threshold")).toInt());
    config.marker.minThresh     = static_cast<float>(parser.value(QStringLiteral("match-min")).toDouble());
    config.marker.maxThresh     = static_cast<float>(parser.value(QStringLiteral("match-max")).toDouble());
    config.marker.isWhiteMark   = parseIsWhiteMark(background);

    if (config.marker.markWidth <= 0)
        config.marker.markWidth = 32;
    if (config.marker.markHeight <= 0)
        config.marker.markHeight = 32;
    if (config.marker.area <= 0.f)
        config.marker.area = 120.f;
    if (config.marker.minThresh <= 0.0f)
        config.marker.minThresh = 0.75f;
    if (config.marker.maxThresh <= 0.0f)
        config.marker.maxThresh = 0.95f;

    const AlgoMat &loaded = *processImage->imageMat;
    out() << "image: " << imagePath << " (" << loaded.cols << "x" << loaded.rows << ")\n";
    out() << "model: " << modelFile << " / " << modelName << "\n";

    const aoi::MarkLocateResult result = aoi::MarkPointLocator::locate(processImage, config);
    if (!result.ok) {
        err() << "mark detect failed: " << result.errorKey << "\n";
        return 1;
    }

    out() << "mark pixel: (" << result.pixelX << ", " << result.pixelY << ")\n";

    if (!outPath.isEmpty()) {
        if (!saveRgbImageInfoToFile(result.resultImage, outPath)) {
            err() << "failed to save output image: " << outPath << "\n";
            return 1;
        }
        out() << "result image saved: " << outPath << "\n";
    }

    return 0;
}

void printUsage(const QCommandLineParser &parser)
{
    err() << parser.helpText() << QLatin1Char('\n');
    err() << QStringLiteral(
        "commands:\n"
        "  sdk-check                                   检查 CISDK 与 AlgorithmManager\n"
        "  list-models [--dir <path>]                    枚举 .model 模板库\n"
        "  list-dlmodels [--dlmodel-dir <path>]          枚举 dlmodel 深度学习模型\n"
        "  die-bonding-dl -i <img> [--dlmodel-dir <path>] 固晶深度学习检测（需 Release 构建）\n"
        "    对齐华星 examples：CISDK DL 参数单位为 px；点间距 um 会除以采集精度转为 px\n"
        "    默认读取 exe 旁 demo_config.json，可用 --config 指定\n"
        "    例：-i test.jpg --group-pitch-x 1250 --chip-group-width 109 --pixel-size 2.95\n"
        "  mark -i <img> --model-file <f> --model-name <n> Mark 点定位\n");
}

} // namespace

int main(int argc, char *argv[])
{
    setupConsoleUtf8();
#if defined(_WIN32)
    setupExeDllDirectory();
#endif

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("aoi_algo_detect_demo"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("AOI 算法检测联调（不依赖配方/工艺注册）"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("command"),
                                 QStringLiteral("sdk-check | list-models | list-dlmodels | die-bonding-dl | mark"));

    QCommandLineOption dirOpt({QStringLiteral("d"), QStringLiteral("dir")},
                              QStringLiteral("模型目录（list-models）"), QStringLiteral("path"));
    QCommandLineOption imageOpt({QStringLiteral("i"), QStringLiteral("image")},
                                QStringLiteral("待检图像（mark）"), QStringLiteral("path"));
    QCommandLineOption modelFileOpt(QStringLiteral("model-file"),
                                    QStringLiteral("模型库文件 .model（mark）"),
                                    QStringLiteral("path"));
    QCommandLineOption modelNameOpt(QStringLiteral("model-name"),
                                    QStringLiteral("模板名称（mark）"), QStringLiteral("name"));
    QCommandLineOption outOpt({QStringLiteral("o"), QStringLiteral("out")},
                              QStringLiteral("输出标注图（mark，可选）"), QStringLiteral("path"));
    QCommandLineOption markTypeOpt(QStringLiteral("mark-type"),
                                   QStringLiteral("circle|cross|rect|ring|false_cross|rect_corner|line（默认 circle）"),
                                   QStringLiteral("type"), QStringLiteral("circle"));
    QCommandLineOption backgroundOpt(QStringLiteral("background"),
                                     QStringLiteral("black | white（默认 black）"),
                                     QStringLiteral("bg"), QStringLiteral("black"));
    QCommandLineOption markWidthOpt(QStringLiteral("mark-width"),
                                    QStringLiteral("Mark 宽（默认 32）"), QStringLiteral("px"),
                                    QStringLiteral("32"));
    QCommandLineOption markHeightOpt(QStringLiteral("mark-height"),
                                     QStringLiteral("Mark 高（默认 32）"), QStringLiteral("px"),
                                     QStringLiteral("32"));
    QCommandLineOption areaOpt(QStringLiteral("area-threshold"),
                               QStringLiteral("面积阈值（默认 120）"), QStringLiteral("n"),
                               QStringLiteral("120"));
    QCommandLineOption matchMinOpt(QStringLiteral("match-min"),
                                   QStringLiteral("匹配下限（默认 0.75）"), QStringLiteral("f"),
                                   QStringLiteral("0.75"));
    QCommandLineOption matchMaxOpt(QStringLiteral("match-max"),
                                   QStringLiteral("匹配上限（默认 0.95）"), QStringLiteral("f"),
                                   QStringLiteral("0.95"));
    QCommandLineOption dlModelDirOpt(QStringLiteral("dlmodel-dir"),
                                     QStringLiteral("深度学习模型目录（默认 exe 旁 dlmodel）"),
                                     QStringLiteral("path"));
    QCommandLineOption solderWidthOpt(QStringLiteral("solder-width"), QStringLiteral("焊盘宽 (px)"),
                                      QStringLiteral("px"));
    QCommandLineOption solderHeightOpt(QStringLiteral("solder-height"), QStringLiteral("焊盘高 (px)"),
                                       QStringLiteral("px"));
    QCommandLineOption chipWidthOpt(QStringLiteral("chip-width"), QStringLiteral("芯片宽 (px)"),
                                    QStringLiteral("px"));
    QCommandLineOption chipHeightOpt(QStringLiteral("chip-height"), QStringLiteral("芯片高 (px)"),
                                     QStringLiteral("px"));
    QCommandLineOption chipGroupWidthOpt(QStringLiteral("chip-group-width"),
                                         QStringLiteral("芯片组宽 (px)，同 --chip-group-res-x"),
                                         QStringLiteral("px"));
    QCommandLineOption chipGroupHeightOpt(QStringLiteral("chip-group-height"),
                                          QStringLiteral("芯片组高 (px)，同 --chip-group-res-y"),
                                          QStringLiteral("px"));
    QCommandLineOption chipGroupResXOpt(QStringLiteral("chip-group-res-x"),
                                        QStringLiteral("芯片组分辨率宽 (px)"), QStringLiteral("px"));
    QCommandLineOption chipGroupResYOpt(QStringLiteral("chip-group-res-y"),
                                        QStringLiteral("芯片组分辨率高 (px)"), QStringLiteral("px"));
    QCommandLineOption pixelSizeOpt(QStringLiteral("pixel-size"),
                                    QStringLiteral("采集精度 (um/px)，同 --capture-accuracy"),
                                    QStringLiteral("um/px"), QStringLiteral("2.95"));
    QCommandLineOption captureAccuracyOpt(QStringLiteral("capture-accuracy"),
                                          QStringLiteral("采集精度 (um/px)"), QStringLiteral("um/px"),
                                          QStringLiteral("2.95"));
    QCommandLineOption groupPitchXOpt(QStringLiteral("group-pitch-x"),
                                      QStringLiteral("芯片组点间距 X (um)，同 --led-gap-x"),
                                      QStringLiteral("um"));
    QCommandLineOption groupPitchYOpt(QStringLiteral("group-pitch-y"),
                                      QStringLiteral("芯片组点间距 Y (um)，同 --led-gap-y"),
                                      QStringLiteral("um"));
    QCommandLineOption ledGapXOpt(QStringLiteral("led-gap-x"), QStringLiteral("点间距 X (um，旧名)"),
                                  QStringLiteral("um"));
    QCommandLineOption ledGapYOpt(QStringLiteral("led-gap-y"), QStringLiteral("点间距 Y (um，旧名)"),
                                  QStringLiteral("um"));
    QCommandLineOption configOpt(QStringLiteral("config"),
                                 QStringLiteral("华星 demo_config.json（默认 exe 旁 demo_config.json）"),
                                 QStringLiteral("path"));
    QCommandLineOption alignTypeOpt(QStringLiteral("align-type"),
                                    QStringLiteral("芯片排列（0=CHIP_RGB_H）"), QStringLiteral("n"),
                                    QStringLiteral("0"));
    QCommandLineOption confThrOpt(QStringLiteral("conf-thr"), QStringLiteral("芯片置信度"),
                                  QStringLiteral("f"), QStringLiteral("0.4"));
    QCommandLineOption confGroupThrOpt(QStringLiteral("conf-group-thr"),
                                       QStringLiteral("芯片组置信度"), QStringLiteral("f"),
                                       QStringLiteral("0.6"));
    QCommandLineOption chipLimThrOpt(QStringLiteral("chip-lim-thr"),
                                     QStringLiteral("缺陷分类芯片阈值"), QStringLiteral("f"),
                                     QStringLiteral("0.8"));
    QCommandLineOption solderLimThrOpt(QStringLiteral("solder-lim-thr"),
                                       QStringLiteral("缺陷分类焊盘阈值"), QStringLiteral("f"),
                                       QStringLiteral("0.8"));
    QCommandLineOption defectClassifyOpt(QStringLiteral("defect-classify"),
                                         QStringLiteral("启用缺陷分类模型"));
    QCommandLineOption noDefectClassifyOpt(QStringLiteral("no-defect-classify"),
                                           QStringLiteral("禁用缺陷分类模型"));
    QCommandLineOption polarityDetectOpt(QStringLiteral("polarity-detect"),
                                         QStringLiteral("启用极性检测模型（默认开）"));
    QCommandLineOption noPolarityDetectOpt(QStringLiteral("no-polarity-detect"),
                                           QStringLiteral("禁用极性检测模型"));
    QCommandLineOption dlEngineOpt(QStringLiteral("dl-engine"),
                                   QStringLiteral("推理引擎 0=ONNX_CPU 1=ONNX_GPU 2=TRT（默认 2）"),
                                   QStringLiteral("n"), QStringLiteral("2"));

    parser.addOption(dirOpt);
    parser.addOption(imageOpt);
    parser.addOption(modelFileOpt);
    parser.addOption(modelNameOpt);
    parser.addOption(outOpt);
    parser.addOption(markTypeOpt);
    parser.addOption(backgroundOpt);
    parser.addOption(markWidthOpt);
    parser.addOption(markHeightOpt);
    parser.addOption(areaOpt);
    parser.addOption(matchMinOpt);
    parser.addOption(matchMaxOpt);
    parser.addOption(dlModelDirOpt);
    parser.addOption(solderWidthOpt);
    parser.addOption(solderHeightOpt);
    parser.addOption(chipWidthOpt);
    parser.addOption(chipHeightOpt);
    parser.addOption(chipGroupWidthOpt);
    parser.addOption(chipGroupHeightOpt);
    parser.addOption(chipGroupResXOpt);
    parser.addOption(chipGroupResYOpt);
    parser.addOption(pixelSizeOpt);
    parser.addOption(captureAccuracyOpt);
    parser.addOption(groupPitchXOpt);
    parser.addOption(groupPitchYOpt);
    parser.addOption(ledGapXOpt);
    parser.addOption(ledGapYOpt);
    parser.addOption(configOpt);
    parser.addOption(alignTypeOpt);
    parser.addOption(confThrOpt);
    parser.addOption(confGroupThrOpt);
    parser.addOption(chipLimThrOpt);
    parser.addOption(solderLimThrOpt);
    parser.addOption(defectClassifyOpt);
    parser.addOption(noDefectClassifyOpt);
    parser.addOption(polarityDetectOpt);
    parser.addOption(noPolarityDetectOpt);
    parser.addOption(dlEngineOpt);
    parser.process(app);

    const QStringList positional = parser.positionalArguments();
    if (positional.isEmpty()) {
        printUsage(parser);
        return 2;
    }

    const QString command = positional.constFirst().toLower();
    if (command == QLatin1String("sdk-check"))
        return runSdkCheck();
    if (command == QLatin1String("list-models"))
        return runListModels(parser.value(dirOpt));
    if (command == QLatin1String("list-dlmodels"))
        return aoi::demo::listDlModels(parser.value(dlModelDirOpt));
    if (command == QLatin1String("die-bonding-dl"))
        return aoi::demo::runDieBondingDl(parser);
    if (command == QLatin1String("mark"))
        return runMarkDetect(parser);

    err() << "unknown command: " << command << "\n";
    printUsage(parser);
    return 2;
}
