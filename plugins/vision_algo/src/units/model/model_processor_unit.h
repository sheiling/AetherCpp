#pragma once

/**
 * @file model_processor_unit.h
 * @brief .model 模板文件管理单元（经 ModelProcessorOp 分发）
 */

#include <common/base/algorithm/algorithm_base.h>

#include <vision_algo/units/model/model_processor_data.h>

#include <QString>

namespace vision_algo {

/** CISDK model_processor 底层实现（单元内部使用） */
class ModelProcessorCore
{
public:
    static common::errors::ErrorPtr generate(const ModelGenerateParam &request);
    static common::errors::ErrorPtr listModelPaths(const std::string &directory,
                                                   std::vector<std::string> &outFileNames);
    static common::errors::ErrorPtr listModelNames(const std::string &modelPath,
                                                   std::vector<std::string> &outNames);
    static common::errors::ErrorPtr listAllModels(
        const std::string &directory, std::map<std::string, std::vector<std::string>> &outMap);
    static common::errors::ErrorPtr extractModelImage(
        const std::string &modelPath, const std::string &modelName,
        std::shared_ptr<common::device::ImageInfo> &outImage);
    static common::errors::ErrorPtr removeModelName(const std::string &modelPath,
                                                    const std::string &modelName);
    static std::string defaultModelDirectory();
    static bool        loadModelFile(AlgoHandle modelHandle, const QString &modelPath);
    static bool        sdkReady();
    static QString     sdkLoadDetail();
};

class ModelProcessorUnit final : public common::algorithm::AlgorithmBase
{
public:
    std::string Name() const override;
    common::errors::ErrorPtr Execute(common::algorithm::AlgorithmExecuteData &data) override;
};

} // namespace vision_algo
