#include "model_generator.h"

#include <vision_algo/units/model/model_processor_data.h>

#include <common/utility/errors/include/error.h>

#include <common/base/algorithm/algorithm_packet.h>

#include <map>

namespace aoi {
namespace {

ModelOpResult fromError(const common::errors::ErrorPtr &err)
{
    ModelOpResult result;
    result.ok = !err;
    if (err)
        result.errorKey = QString::fromStdString(err->Message());
    return result;
}

vision_algo::IVisionAlgoService *algoService()
{
    return vision_algo::service();
}

ModelOpResult runModelOp(const vision_algo::ModelProcessorInput &inPkt,
                         vision_algo::ModelProcessorOutput       *outPkt = nullptr)
{
    auto *svc = algoService();
    if (!svc || !svc->isReady()) {
        return fromError(
            common::errors::New(1, std::string("vision_algo.error.service_not_ready")));
    }

    common::algorithm::AlgorithmExecuteData data;
    data.inputs = static_cast<const common::algorithm::AlgorithmPacket &>(inPkt);
    if (const auto err = svc->execute(vision_algo::kUnitModelProcessor, data))
        return fromError(err);
    if (outPkt) {
        if (auto *paths = data.outputs.Get<std::vector<std::string>>(vision_algo::ModelProcessorOutput::kPathList))
            outPkt->setPathList(*paths);
        if (auto *names = data.outputs.Get<std::vector<std::string>>(vision_algo::ModelProcessorOutput::kNameList))
            outPkt->setNameList(*names);
        if (auto *map = data.outputs.Get<std::map<std::string, std::vector<std::string>>>(
                vision_algo::ModelProcessorOutput::kModelMap))
            outPkt->setModelMap(*map);
        if (auto *img = data.outputs.Get<std::shared_ptr<common::device::ImageInfo>>(
                vision_algo::ModelProcessorOutput::kExtractImage))
            outPkt->setExtractImage(*img);
        if (auto *dir = data.outputs.Get<std::string>(vision_algo::ModelProcessorOutput::kDefaultModelDir))
            outPkt->setDefaultModelDir(*dir);
    }
    return fromError(nullptr);
}

} // namespace

bool ModelGenerator::sdkReady()
{
    auto *svc = algoService();
    if (!svc || !svc->isReady())
        return false;
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::SdkReady);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = static_cast<const common::algorithm::AlgorithmPacket &>(inPkt);
    if (svc->execute(vision_algo::kUnitModelProcessor, data))
        return false;
    if (auto *ready = data.outputs.Get<bool>(vision_algo::ModelProcessorOutput::kSdkReady))
        return *ready;
    return false;
}

QString ModelGenerator::sdkIssue()
{
    return vision_algo::modelSdkIssue();
}

QString ModelGenerator::defaultModelDir()
{
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::DefaultDirectory);
    vision_algo::ModelProcessorOutput outPkt;
    if (!runModelOp(inPkt, &outPkt).ok)
        return {};
    if (auto *dir = outPkt.defaultModelDir())
        return QString::fromStdString(*dir);
    return {};
}

ModelOpResult ModelGenerator::generate(const ModelGenerateRequest &request)
{
    vision_algo::ModelGenerateParam param;
    param.sourceImage = request.sourceImage;
    param.roiX        = request.roi.x();
    param.roiY        = request.roi.y();
    param.roiWidth    = request.roi.width();
    param.roiHeight   = request.roi.height();
    param.modelPath   = request.modelPath.toStdString();
    param.modelName   = request.modelName.toStdString();
    param.modelKind   = request.modelKind;

    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::Generate);
    inPkt.setGenerateRequest(std::move(param));
    return runModelOp(inPkt);
}

ModelOpResult ModelGenerator::listModelPaths(QStringList &outFileNames, const QString &directory)
{
    outFileNames.clear();
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::ListPaths);
    inPkt.setDirectory(directory.toStdString());
    vision_algo::ModelProcessorOutput outPkt;
    const ModelOpResult result = runModelOp(inPkt, &outPkt);
    if (!result.ok)
        return result;
    if (auto *paths = outPkt.pathList()) {
        for (const std::string &path : *paths)
            outFileNames.append(QString::fromStdString(path));
    }
    return result;
}

ModelOpResult ModelGenerator::listModelNames(const QString &modelPath, QStringList &outNames)
{
    outNames.clear();
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::ListNames);
    inPkt.setModelPath(modelPath.toStdString());
    vision_algo::ModelProcessorOutput outPkt;
    const ModelOpResult result = runModelOp(inPkt, &outPkt);
    if (!result.ok)
        return result;
    if (auto *names = outPkt.nameList()) {
        for (const std::string &name : *names)
            outNames.append(QString::fromStdString(name));
    }
    return result;
}

ModelOpResult ModelGenerator::listAllModels(QMap<QString, QStringList> &outMap,
                                              const QString &directory)
{
    outMap.clear();
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::ListAll);
    inPkt.setDirectory(directory.toStdString());
    vision_algo::ModelProcessorOutput outPkt;
    const ModelOpResult result = runModelOp(inPkt, &outPkt);
    if (!result.ok)
        return result;
    if (auto *map = outPkt.modelMap()) {
        for (const auto &entry : *map) {
            QStringList names;
            for (const std::string &name : entry.second)
                names.append(QString::fromStdString(name));
            outMap.insert(QString::fromStdString(entry.first), names);
        }
    }
    return result;
}

ModelOpResult ModelGenerator::extractModelImage(const QString &modelPath, const QString &modelName,
                                                std::shared_ptr<common::device::ImageInfo> &outImage)
{
    outImage.reset();
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::ExtractImage);
    inPkt.setModelPath(modelPath.toStdString());
    inPkt.setModelName(modelName.toStdString());
    vision_algo::ModelProcessorOutput outPkt;
    const ModelOpResult result = runModelOp(inPkt, &outPkt);
    if (!result.ok)
        return result;
    if (auto *img = outPkt.extractImage())
        outImage = *img;
    return result;
}

ModelOpResult ModelGenerator::removeModelName(const QString &modelPath, const QString &modelName)
{
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::RemoveName);
    inPkt.setModelPath(modelPath.toStdString());
    inPkt.setModelName(modelName.toStdString());
    return runModelOp(inPkt);
}

} // namespace aoi
