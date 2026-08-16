#pragma once

/**
 * @file model_processor_data.h
 * @brief ModelProcessor 单元入参/出参（继承 AlgorithmPacket）
 *
 * 模板生成、列举、提取及 SDK 自检均经 execute(kUnitModelProcessor, data) 完成。
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>
#include <common/core/device/imageCapture/image_capture.h>

#include <nlohmann/json.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vision_algo {

/** 模板生成时的处理类别（Mark / Chip） */
enum class ModelProcKind : std::uint8_t
{
    Mark = 0, ///< Mark 点模板
    Chip = 1, ///< 芯片模板
};
NLOHMANN_JSON_SERIALIZE_ENUM(ModelProcKind, {
    { ModelProcKind::Mark, "Mark" },
    { ModelProcKind::Chip, "Chip" },
})

/** ModelProcessor 单元操作类型 */
enum class ModelProcessorOp : std::uint8_t
{
    Generate         = 0, ///< 从 ROI 裁剪并写入模板库
    ListPaths,            ///< 列举目录下 .model 文件路径
    ListNames,            ///< 列举指定 .model 内模板名
    ListAll,              ///< 列举目录下全部模板名映射
    ExtractImage,         ///< 从模板库提取模板图像
    RemoveName,           ///< 从模板库删除指定模板名
    DefaultDirectory,     ///< 查询默认模板目录
    SdkReady,             ///< 查询 model_processor SDK 是否加载成功
    SdkLoadDetail,        ///< 查询 SDK 加载失败详情
};

/** 模板生成请求（ROI 字段语义同 CISDK ::AlgoRect） */
struct ModelGenerateParam
{
    std::shared_ptr<common::device::ImageInfo> sourceImage; ///< 源图（Bayer 或 RGB）
    int                                        roiX      = 0; ///< ROI 左上角 X
    int                                        roiY      = 0; ///< ROI 左上角 Y
    int                                        roiWidth  = 0; ///< ROI 宽度
    int                                        roiHeight = 0; ///< ROI 高度
    std::string                                modelPath;     ///< 目标 .model 文件路径
    std::string                                modelName;     ///< 写入的模板名
    ModelProcKind                              modelKind = ModelProcKind::Mark; ///< Mark 或 Chip
};

/** ModelProcessor 入参 Packet */
class ModelProcessorInput : public common::algorithm::AlgorithmPacket
{
public:
    static constexpr const char kOp[]              = "op";              ///< 操作类型键
    static constexpr const char kDirectory[]       = "directory";       ///< 模板目录键
    static constexpr const char kModelPath[]       = "modelPath";       ///< .model 路径键
    static constexpr const char kModelName[]       = "modelName";       ///< 模板名键
    static constexpr const char kGenerateRequest[] = "generateRequest"; ///< 生成请求键

    void setOp(ModelProcessorOp op) { Set(kOp, op); }
    ModelProcessorOp       *op() { return Get<ModelProcessorOp>(kOp); }
    const ModelProcessorOp *op() const { return Get<ModelProcessorOp>(kOp); }

    void setGenerateRequest(ModelGenerateParam req) { Set(kGenerateRequest, std::move(req)); }
    ModelGenerateParam *generateRequest() { return Get<ModelGenerateParam>(kGenerateRequest); }

    void setDirectory(const std::string &dir) { Set(kDirectory, dir); }
    std::string *directory() { return Get<std::string>(kDirectory); }

    void setModelPath(const std::string &path) { Set(kModelPath, path); }
    std::string *modelPath() { return Get<std::string>(kModelPath); }

    void setModelName(const std::string &name) { Set(kModelName, name); }
    std::string *modelName() { return Get<std::string>(kModelName); }
};

/** ModelProcessor 出参 Packet */
class ModelProcessorOutput : public common::algorithm::AlgorithmPacket
{
public:
    static constexpr const char kPathList[]        = "pathList";        ///< .model 路径列表
    static constexpr const char kNameList[]        = "nameList";        ///< 模板名列表
    static constexpr const char kModelMap[]        = "modelMap";        ///< 路径到模板名映射
    static constexpr const char kExtractImage[]    = "extractImage";    ///< 提取的模板图
    static constexpr const char kDefaultModelDir[] = "defaultModelDir"; ///< 默认模板目录
    static constexpr const char kSdkReady[]        = "sdkReady";        ///< SDK 是否就绪
    static constexpr const char kSdkDetail[]       = "sdkDetail";       ///< SDK 加载详情

    void setPathList(std::vector<std::string> paths) { Set(kPathList, std::move(paths)); }
    std::vector<std::string> *pathList() { return Get<std::vector<std::string>>(kPathList); }

    void setNameList(std::vector<std::string> names) { Set(kNameList, std::move(names)); }
    std::vector<std::string> *nameList() { return Get<std::vector<std::string>>(kNameList); }

    void setModelMap(std::map<std::string, std::vector<std::string>> map)
    {
        Set(kModelMap, std::move(map));
    }
    std::map<std::string, std::vector<std::string>> *modelMap()
    {
        return Get<std::map<std::string, std::vector<std::string>>>(kModelMap);
    }

    void setExtractImage(std::shared_ptr<common::device::ImageInfo> img)
    {
        Set(kExtractImage, std::move(img));
    }
    std::shared_ptr<common::device::ImageInfo> *extractImage()
    {
        return Get<std::shared_ptr<common::device::ImageInfo>>(kExtractImage);
    }

    void setDefaultModelDir(std::string dir) { Set(kDefaultModelDir, std::move(dir)); }
    std::string *defaultModelDir() { return Get<std::string>(kDefaultModelDir); }

    void setSdkReady(bool v) { Set(kSdkReady, v); }
    bool *sdkReady() { return Get<bool>(kSdkReady); }

    void setSdkDetail(std::string detail) { Set(kSdkDetail, std::move(detail)); }
    std::string *sdkDetail() { return Get<std::string>(kSdkDetail); }
};

} // namespace vision_algo
