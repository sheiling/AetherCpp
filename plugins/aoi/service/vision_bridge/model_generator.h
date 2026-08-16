#pragma once

/**
 * @file model_generator.h
 * @brief AOI 侧模板制作门面（委派 VisionAlgo ModelProcessor 单元）
 */

#include <vision_algo/core/algorithm_unit_type.h>
#include <vision_algo/units/model/model_processor_data.h>
#include <vision_algo/core/service_access.h>

#include <common/core/device/imageCapture/image_capture.h>

#include <QMap>
#include <QRect>
#include <QString>
#include <QStringList>

#include <memory>

namespace aoi {

struct ModelOpResult
{
    bool    ok = false;
    QString errorKey;
};

struct ModelGenerateRequest
{
    std::shared_ptr<common::device::ImageInfo> sourceImage;
    QRect                                      roi;
    QString                                    modelPath;
    QString                                    modelName;
    vision_algo::ModelProcKind                 modelKind = vision_algo::ModelProcKind::Mark;
};

class ModelGenerator
{
public:
    static bool sdkReady();
    static QString sdkIssue();
    static QString defaultModelDir();

    static ModelOpResult generate(const ModelGenerateRequest &request);
    static ModelOpResult listModelPaths(QStringList &outFileNames, const QString &directory = {});
    static ModelOpResult listModelNames(const QString &modelPath, QStringList &outNames);
    static ModelOpResult listAllModels(QMap<QString, QStringList> &outMap,
                                       const QString &directory = {});
    static ModelOpResult extractModelImage(const QString &modelPath, const QString &modelName,
                                           std::shared_ptr<common::device::ImageInfo> &outImage);
    static ModelOpResult removeModelName(const QString &modelPath, const QString &modelName);
};

} // namespace aoi
