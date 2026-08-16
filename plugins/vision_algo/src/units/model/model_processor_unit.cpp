/**
 * @file model_processor.cpp
 * @brief CISDK model_processor 封装实现
 */

#include "model_processor_unit.h"

#include "algo_ci_error_check.h"

#include "image_utils.h"
#include "vision_algo_i18n.h"

#include <vision_algo/units/model/model_processor_data.h>

#include <common/base/algorithm/algorithm_factory.h>

#include <ci_basic_error.h>
#include <ci_utils_algomat.h>
#include <model_processor.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRect>
#include <QString>
#include <QVector>

#include <memory>

namespace vision_algo {
namespace {

constexpr int kModelNameBufferSize = 20480;

QString resolveModelDirectory(const std::string &directory)
{
    if (!directory.empty())
        return QDir(QString::fromStdString(directory)).absolutePath();
    return QString::fromStdString(ModelProcessorCore::defaultModelDirectory());
}

struct ModelHandleGuard
{
    AlgoHandle modelHandle = nullptr;

    ~ModelHandleGuard()
    {
        if (modelHandle)
            ModelDestroyHandle(modelHandle);
    }
};

QStringList parseModelNameList(const char *buffer)
{
    if (!buffer || !buffer[0])
        return {};
    const QString raw = QString::fromLocal8Bit(buffer).trimmed();
    if (raw.isEmpty())
        return {};
    return raw.split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

ModelProcType toCisdkProcType(ModelProcKind kind)
{
    return kind == ModelProcKind::Chip ? MODEL_PROC_CHIP : MODEL_PROC_MARK;
}

common::errors::ErrorPtr openLoadedModel(const QString &modelPath, AlgoHandle *outModelHandle,
                                         std::unique_ptr<ModelHandleGuard> &outGuard)
{
    const QString path = normalizeModelPath(modelPath);
    if (path.isEmpty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.path_required");
    if (!QFile::exists(path))
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.model_not_found");

    if (!ModelProcessorCore::sdkReady())
        return algoErrorFromKey(common::errors::algo_module::kSdkError,
                                "vision_algo.model.sdk_missing");

    const AlgoHandle rawHandle = ModelCreateHandle();
    if (!rawHandle)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    auto guard = std::make_unique<ModelHandleGuard>();
    guard->modelHandle = rawHandle;

    if (!ModelProcessorCore::loadModelFile(rawHandle, path))
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.model_load_failed");

    *outModelHandle = rawHandle;
    outGuard        = std::move(guard);
    return nullptr;
}

} // namespace

#ifdef _MSC_VER
namespace {
bool loadModelFileSeh(AlgoHandle modelHandle, const char *nativePath)
{
    __try {
        return ModelLoad(modelHandle, nativePath) == ALGO_PASS;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
} // namespace
#endif

bool ModelProcessorCore::loadModelFile(AlgoHandle modelHandle, const QString &modelPath)
{
    const QByteArray nativePath = toNativePathBytes(modelPath);
#ifdef _MSC_VER
    return loadModelFileSeh(modelHandle, nativePath.constData());
#else
    return ModelLoad(modelHandle, nativePath.constData()) == ALGO_PASS;
#endif
}

std::string ModelProcessorCore::defaultModelDirectory()
{
    return defaultModelDir().toStdString();
}

common::errors::ErrorPtr ModelProcessorCore::generate(const ModelGenerateParam &request)
{
    if (!isValidRgbImageInfo(request.sourceImage))
        return algoErrorFromKey(common::errors::algo_module::kInvalidImage,
                                "vision_algo.model.no_preview");
    if (request.modelPath.empty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.path_required");
    if (request.modelName.empty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.name_required");
    if (request.roiWidth < 1 || request.roiHeight < 1)
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.roi_required");

    if (!ModelProcessorCore::sdkReady())
        return algoErrorFromKey(common::errors::algo_module::kSdkError,
                                "vision_algo.model.sdk_missing");

    const ::AlgoMat &rgbMat = *request.sourceImage->imageMat;
    const QRect    imageRect(0, 0, rgbMat.cols, rgbMat.rows);
    const QRect    roi(request.roiX, request.roiY, request.roiWidth, request.roiHeight);
    const QRect    clipped = roi.intersected(imageRect);
    if (clipped.width() < 1 || clipped.height() < 1)
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.roi_required");

    QVector<char> bgrBuffer = packBgr888FromRgbMat(&rgbMat);
    if (bgrBuffer.isEmpty())
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    ::AlgoMat fullMat =
        AlgoMatCreateShadow(rgbMat.rows, rgbMat.cols, ALGO_8UC3, bgrBuffer.data());
    if (AlgoMatIsEmpty(&fullMat))
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    ::AlgoRect algoRoi{clipped.x(), clipped.y(), clipped.width(), clipped.height()};

    std::unique_ptr<::AlgoMat, decltype(&AlgoMatDelete)> roiMat(AlgoMatNew(), AlgoMatDelete);
    if (!roiMat)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    if (AlgoMatGetRect(&fullMat, &algoRoi, roiMat.get()) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");
    if (AlgoMatIsEmpty(roiMat.get()))
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    const QString      modelPath = normalizeModelPath(QString::fromStdString(request.modelPath));
    QDir().mkpath(QFileInfo(modelPath).absolutePath());
    const QByteArray modelPathNative = toNativePathBytes(modelPath);

    AlgoHandle rawHandle = ModelCreateHandle();
    if (!rawHandle)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    ModelHandleGuard guard;
    guard.modelHandle = rawHandle;

    if (QFileInfo::exists(modelPath) && QFileInfo(modelPath).size() > 0) {
        if (!ModelProcessorCore::loadModelFile(rawHandle, modelPath)) {
            ModelDestroyHandle(rawHandle);
            rawHandle = ModelCreateHandle();
            if (!rawHandle)
                return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                        "vision_algo.model.make_failed");
            guard.modelHandle = rawHandle;
        }
    }

    const QByteArray nameLocal = toLocalNameBytes(QString::fromStdString(request.modelName));
    if (ModelSetType(rawHandle, toCisdkProcType(request.modelKind)) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");
    if (ModelAdd(rawHandle, nameLocal.constData(), roiMat.get()) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");
    if (ModelSave(rawHandle, modelPathNative.constData()) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    return nullptr;
}

common::errors::ErrorPtr ModelProcessorCore::listModelPaths(const std::string &directory,
                                                        std::vector<std::string> &outFileNames)
{
    outFileNames.clear();
    const QDir dir(resolveModelDirectory(directory));
    if (!dir.exists())
        return nullptr;

    const QFileInfoList files =
        dir.entryInfoList({QStringLiteral("*.model")}, QDir::Files, QDir::Name);
    for (const QFileInfo &info : files)
        outFileNames.push_back(info.fileName().toStdString());
    return nullptr;
}

common::errors::ErrorPtr ModelProcessorCore::listModelNames(const std::string &modelPath,
                                                        std::vector<std::string> &outNames)
{
    outNames.clear();

    AlgoHandle                         modelHandle = nullptr;
    std::unique_ptr<ModelHandleGuard> guard;
    if (const auto err = openLoadedModel(QString::fromStdString(modelPath), &modelHandle, guard))
        return err;

    QVector<char> buffer(kModelNameBufferSize, 0);
    if (ModelGetAllName(modelHandle, buffer.data()) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    const QStringList names = parseModelNameList(buffer.constData());
    for (const QString &name : names)
        outNames.push_back(name.toStdString());
    return nullptr;
}

common::errors::ErrorPtr ModelProcessorCore::listAllModels(
    const std::string &directory, std::map<std::string, std::vector<std::string>> &outMap)
{
    outMap.clear();

    std::vector<std::string> paths;
    if (const auto err = listModelPaths(directory, paths))
        return err;

    const QDir dir(resolveModelDirectory(directory));
    for (const std::string &fileName : paths) {
        const QString fullPath = dir.absoluteFilePath(QString::fromStdString(fileName));
        if (!QFile::exists(fullPath))
            continue;

        std::vector<std::string> names;
        if (listModelNames(fullPath.toStdString(), names))
            continue;
        outMap[fileName] = std::move(names);
    }
    return nullptr;
}

common::errors::ErrorPtr ModelProcessorCore::extractModelImage(
    const std::string &modelPath, const std::string &modelName,
    std::shared_ptr<common::device::ImageInfo> &outImage)
{
    outImage.reset();
    if (modelName.empty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.name_required");

    AlgoHandle                         modelHandle = nullptr;
    std::unique_ptr<ModelHandleGuard> guard;
    if (const auto err = openLoadedModel(QString::fromStdString(modelPath), &modelHandle, guard))
        return err;

    std::unique_ptr<::AlgoMat, decltype(&AlgoMatDelete)> tmpl(AlgoMatNew(), AlgoMatDelete);
    if (!tmpl)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    const QByteArray nameLocal = toLocalNameBytes(QString::fromStdString(modelName));
    if (ModelGet(modelHandle, nameLocal.constData(), tmpl.get(), 1) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");

    outImage = cloneAlgoMatToRgbImageInfo(tmpl.get());
    if (!outImage)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.make_failed");
    return nullptr;
}

common::errors::ErrorPtr ModelProcessorCore::removeModelName(const std::string &modelPath,
                                                         const std::string &modelName)
{
    if (modelPath.empty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.path_required");
    if (modelName.empty())
        return algoErrorFromKey(common::errors::algo_module::kInvalidArgument,
                                "vision_algo.model.name_required");

    AlgoHandle                         modelHandle = nullptr;
    std::unique_ptr<ModelHandleGuard> guard;
    if (const auto err = openLoadedModel(QString::fromStdString(modelPath), &modelHandle, guard))
        return err;

    const QByteArray nameLocal = toLocalNameBytes(QString::fromStdString(modelName));
    if (ModelRemove(modelHandle, nameLocal.constData()) != ALGO_PASS)
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.template_delete_failed");

    const QString    path            = normalizeModelPath(QString::fromStdString(modelPath));
    const QByteArray modelPathNative = toNativePathBytes(path);
    if (ModelSave(modelHandle, modelPathNative.constData()) != ALGO_PASS) {
        guard.reset();
        if (QFile::remove(path))
            return nullptr;
        return algoErrorFromKey(common::errors::algo_module::kModelLoadFailed,
                                "vision_algo.model.template_delete_failed");
    }

    return nullptr;
}

bool ModelProcessorCore::sdkReady()
{
    const AlgoHandle handle = ModelCreateHandle();
    if (!handle)
        return false;
    ModelDestroyHandle(handle);
    return true;
}

QString ModelProcessorCore::sdkLoadDetail()
{
    if (sdkReady())
        return {};
    return tr("vision_algo.sdk.model_create_failed");
}

std::string ModelProcessorUnit::Name() const
{
    return kUnitModelProcessor;
}

common::errors::ErrorPtr ModelProcessorUnit::Execute(common::algorithm::AlgorithmExecuteData &data)
{
    auto *op = data.inputs.Get<ModelProcessorOp>(ModelProcessorInput::kOp);
    if (!op)
        return algoInvalidArgument("vision_algo.error.param_null");

    ModelProcessorOutput outPkt;

    switch (*op) {
    case ModelProcessorOp::Generate: {
        auto *req = data.inputs.Get<ModelGenerateParam>(ModelProcessorInput::kGenerateRequest);
        if (!req)
            return algoInvalidArgument("vision_algo.error.param_null");
        return ModelProcessorCore::generate(*req);
    }
    case ModelProcessorOp::ListPaths: {
        const std::string directory =
            data.inputs.Get<std::string>(ModelProcessorInput::kDirectory)
                ? *data.inputs.Get<std::string>(ModelProcessorInput::kDirectory)
                : std::string{};
        std::vector<std::string> paths;
        if (const auto err = ModelProcessorCore::listModelPaths(directory, paths))
            return err;
        outPkt.setPathList(std::move(paths));
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::ListNames: {
        auto *modelPath = data.inputs.Get<std::string>(ModelProcessorInput::kModelPath);
        if (!modelPath)
            return algoInvalidArgument("vision_algo.model.path_required");
        std::vector<std::string> names;
        if (const auto err = ModelProcessorCore::listModelNames(*modelPath, names))
            return err;
        outPkt.setNameList(std::move(names));
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::ListAll: {
        const std::string directory =
            data.inputs.Get<std::string>(ModelProcessorInput::kDirectory)
                ? *data.inputs.Get<std::string>(ModelProcessorInput::kDirectory)
                : std::string{};
        std::map<std::string, std::vector<std::string>> map;
        if (const auto err = ModelProcessorCore::listAllModels(directory, map))
            return err;
        outPkt.setModelMap(std::move(map));
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::ExtractImage: {
        auto *modelPath = data.inputs.Get<std::string>(ModelProcessorInput::kModelPath);
        auto *modelName = data.inputs.Get<std::string>(ModelProcessorInput::kModelName);
        if (!modelPath || !modelName)
            return algoInvalidArgument("vision_algo.model.name_required");
        std::shared_ptr<common::device::ImageInfo> image;
        if (const auto err =
                ModelProcessorCore::extractModelImage(*modelPath, *modelName, image))
            return err;
        outPkt.setExtractImage(std::move(image));
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::RemoveName: {
        auto *modelPath = data.inputs.Get<std::string>(ModelProcessorInput::kModelPath);
        auto *modelName = data.inputs.Get<std::string>(ModelProcessorInput::kModelName);
        if (!modelPath || !modelName)
            return algoInvalidArgument("vision_algo.model.name_required");
        return ModelProcessorCore::removeModelName(*modelPath, *modelName);
    }
    case ModelProcessorOp::DefaultDirectory: {
        outPkt.setDefaultModelDir(ModelProcessorCore::defaultModelDirectory());
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::SdkReady: {
        outPkt.setSdkReady(ModelProcessorCore::sdkReady());
        data.outputs = outPkt;
        return nullptr;
    }
    case ModelProcessorOp::SdkLoadDetail: {
        outPkt.setSdkDetail(ModelProcessorCore::sdkLoadDetail().toStdString());
        data.outputs = outPkt;
        return nullptr;
    }
    default:
        return algoInvalidArgument("vision_algo.error.unit_type_not_available");
    }
}

REGISTER_ALGORITHM(kUnitModelProcessor, ModelProcessorUnit);

} // namespace vision_algo
