#pragma once

/**
 * @file chip_detector_data.h
 * @brief 芯片检测单元入参/出参（继承 AlgorithmPacket，无 CISDK 依赖）
 *
 * 经 IVisionAlgoService::execute(kUnitChipDetector, data) 调用；
 * 当前仅支持 ChipDetectorOp::Detect。
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>

#include <nlohmann/json.hpp>

#include <string>

namespace vision_algo {

/** 芯片 RGB 类型，对应 CISDK ChipDetectRGBType */
enum class ChipDetectRgbType : int
{
    PanelChipR   = 1,
    PanelChipG   = 2,
    PanelChipB   = 3,
    PanelChipMip = 4,
};
NLOHMANN_JSON_SERIALIZE_ENUM(ChipDetectRgbType, {
    {ChipDetectRgbType::PanelChipR, "PanelChipR"},
    {ChipDetectRgbType::PanelChipG, "PanelChipG"},
    {ChipDetectRgbType::PanelChipB, "PanelChipB"},
    {ChipDetectRgbType::PanelChipMip, "PanelChipMip"},
})

/** 芯片 RGB 排列方式，对应 CISDK ChipDetectRGBAlignType */
enum class ChipDetectRgbAlignType : int
{
    PanelChipRgbH   = 1,
    PanelChipBgrH   = 2,
    PanelChipRgbV   = 3,
    PanelChipBgrV   = 4,
    PanelChipSingle = 5,
    PanelChipCog    = 6,
};
NLOHMANN_JSON_SERIALIZE_ENUM(ChipDetectRgbAlignType, {
    {ChipDetectRgbAlignType::PanelChipRgbH, "PanelChipRgbH"},
    {ChipDetectRgbAlignType::PanelChipBgrH, "PanelChipBgrH"},
    {ChipDetectRgbAlignType::PanelChipRgbV, "PanelChipRgbV"},
    {ChipDetectRgbAlignType::PanelChipBgrV, "PanelChipBgrV"},
    {ChipDetectRgbAlignType::PanelChipSingle, "PanelChipSingle"},
    {ChipDetectRgbAlignType::PanelChipCog, "PanelChipCog"},
})

/** 芯片检测阈值，对应 CISDK ChipDetectThresh */
struct ChipDetectThreshold
{
    float matchThresh = 0.f; ///< 匹配阈值（in_match_thresh）
    float lowThresh   = 0.f; ///< 低阈值（in_low_thresh）
    float highThresh  = 0.f; ///< 高阈值（in_high_thresh）
    float areaThresh  = 0.f; ///< 面积阈值（in_area_thresh）
};

/** 芯片检测单元操作类型 */
enum class ChipDetectorOp : std::uint8_t
{
    Detect = 0, ///< 单帧芯片定位与角度检测
};

/** 芯片检测入参 Packet */
class ChipDetectorInput : public common::algorithm::AlgorithmPacket
{
public:
    /** Detect 操作业务字段 */
    struct Fields
    {
        VisionImageContext     vision;              ///< 图像与采集上下文
        std::string            modelFile;           ///< 模板库 .model 文件路径
        std::string            chipGroupModeName;     ///< 芯片组模板名
        std::string            chipModeName;          ///< 单芯片模板名
        ChipDetectRgbType      chipDetectRgbType{};   ///< 检测 RGB 通道类型
        ChipDetectRgbAlignType chipDetectRgbAlignType{}; ///< RGB 对齐方式
        AlgoSize2f             chipSize{};            ///< 芯片物理尺寸（像素）
        ChipDetectThreshold    chipThresh{};          ///< 匹配/分割阈值
        AlgoSize               chipROI{};             ///< 芯片搜索 ROI 尺寸
        std::string            outImageFile;          ///< 可选：结果图保存路径
    };

    static constexpr const char kOp[]     = "op";     ///< 操作类型键
    static constexpr const char kFields[] = "fields"; ///< 业务字段键

    void setOp(ChipDetectorOp op) { Set(kOp, op); }
    ChipDetectorOp       *op() { return Get<ChipDetectorOp>(kOp); }
    const ChipDetectorOp *op() const { return Get<ChipDetectorOp>(kOp); }

    Fields &fields()
    {
        if (!Get<Fields>(kFields))
            Set(kFields, Fields{});
        return *Get<Fields>(kFields);
    }
    void setFields(Fields f) { Set(kFields, std::move(f)); }
    Fields       *getFields() { return Get<Fields>(kFields); }
    const Fields *getFields() const { return Get<Fields>(kFields); }
};

/** 芯片检测出参 Packet */
class ChipDetectorOutput : public common::algorithm::AlgorithmPacket
{
public:
    /** Detect 操作结果字段 */
    struct Fields
    {
        VisionResultMeta meta;           ///< 公共出参元信息
        float            outChipAngle   = 0.0f; ///< 芯片角度（度）
        float            outChipCenterX = 0.0f; ///< 芯片中心 X（像素）
        float            outChipCenterY = 0.0f; ///< 芯片中心 Y（像素）

        bool ok() const { return meta.ok; }
        void setOk(bool v) { meta.ok = v; }
    };

    static constexpr const char kFields[] = "fields"; ///< 结果字段键

    void setFields(Fields f) { Set(kFields, std::move(f)); }
    Fields       *getFields() { return Get<Fields>(kFields); }
    const Fields *getFields() const { return Get<Fields>(kFields); }
};

} // namespace vision_algo
