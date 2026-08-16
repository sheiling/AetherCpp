#pragma once

/**
 * @file mark_point_locate_data.h
 * @brief Mark 点定位单元入参/出参（继承 AlgorithmPacket）
 *
 * 定位与 SDK 自检经 execute(kUnitMarkPointLocate, data) 完成。
 * MarkerType / MarkerDetectInfo 与 CISDK marker_detector 取值对齐。
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>
#include <common/utility/json_tool/reflector.h>

#include <cstdint>
#include <string>

namespace vision_algo {

/** Mark 定位单元操作类型 */
enum class MarkPointLocateOp : std::uint8_t
{
    Locate        = 0, ///< 单帧 Mark 点像素定位
    SdkReady,          ///< 查询 marker_detector SDK 是否加载成功
    SdkLoadDetail,     ///< 查询 SDK 加载失败详情
};

/** 与 CISDK marker_detector MarkerType 取值一致（从 1 起，勿改数值） */
enum class MarkerType : std::uint16_t
{
    Circle     = 1, ///< MARKER_CIRCLE      圆形模板
    Cross      = 2, ///< MARKER_CROSS       十字模板
    Rect       = 3, ///< MARKER_RECT        矩形模板
    Ring       = 4, ///< MARKER_RING        圆环
    FalseCross = 5, ///< MARKER_FALSE_CROSS 虚十字
    RectCorner = 6, ///< MARKER_RECT_CORNER 矩形角点定位
    Line       = 7, ///< MARKER_LINE        边缘线定位
};
NLOHMANN_JSON_SERIALIZE_ENUM(MarkerType, {
    {MarkerType::Circle, "Circle"},
    {MarkerType::Cross, "Cross"},
    {MarkerType::Rect, "Rect"},
    {MarkerType::Ring, "Ring"},
    {MarkerType::FalseCross, "FalseCross"},
    {MarkerType::RectCorner, "RectCorner"},
    {MarkerType::Line, "Line"},
})

/** Mark 检测配方参数（与 CISDK MarkerDetectInfo / MarkerInfo 字段对应） */
struct MarkerDetectInfo
{
    MarkerType  markType    = MarkerType::Circle;
    int         markWidth   = 32;     ///< Mark 宽度（像素）
    int         markHeight  = 32;     ///< Mark 高度（像素）
    uint16_t    isWhiteMark = 0;      ///< CISDK is_white_mark：0 白底黑图，1 黑底白图
    float       area        = 100.0f; ///< 最小面积阈值
    float       minThresh   = 0.6f;   ///< 匹配分数下限
    float       maxThresh   = 1.0f;   ///< 匹配分数上限
    std::string modelFileName;        ///< 模板库 .model 相对/绝对路径
    std::string modelName;            ///< 模板名

    void reflectFromJson(const json &j)
    {
        FROM_JSON(j, markType);
        FROM_JSON(j, markWidth);
        FROM_JSON(j, markHeight);
        FROM_JSON(j, isWhiteMark);
        FROM_JSON(j, area);
        FROM_JSON(j, minThresh);
        FROM_JSON(j, maxThresh);
        FROM_JSON(j, modelFileName);
        FROM_JSON(j, modelName);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON(j, markType);
        TO_JSON(j, markWidth);
        TO_JSON(j, markHeight);
        TO_JSON(j, isWhiteMark);
        TO_JSON(j, area);
        TO_JSON(j, minThresh);
        TO_JSON(j, maxThresh);
        TO_JSON(j, modelFileName);
        TO_JSON(j, modelName);
    }
};

inline bool operator==(const MarkerDetectInfo &a, const MarkerDetectInfo &b)
{
    return a.markType == b.markType && a.markWidth == b.markWidth && a.markHeight == b.markHeight
           && a.isWhiteMark == b.isWhiteMark && a.area == b.area && a.minThresh == b.minThresh
           && a.maxThresh == b.maxThresh && a.modelFileName == b.modelFileName
           && a.modelName == b.modelName;
}

inline bool operator!=(const MarkerDetectInfo &a, const MarkerDetectInfo &b)
{
    return !(a == b);
}

/** Mark 定位入参 Packet */
class MarkPointLocateInput : public common::algorithm::AlgorithmPacket
{
public:
    static constexpr const char kOp[]     = "op";     ///< 操作类型键
    static constexpr const char kFields[] = "fields"; ///< 业务字段键

    void setOp(MarkPointLocateOp op) { Set(kOp, op); }
    MarkPointLocateOp       *op() { return Get<MarkPointLocateOp>(kOp); }
    const MarkPointLocateOp *op() const { return Get<MarkPointLocateOp>(kOp); }

    /** Locate 操作业务字段 */
    struct Fields
    {
        VisionImageContext vision;     ///< 图像与采集上下文
        MarkerDetectInfo   markerInfo; ///< Mark 检测参数
        std::string        imagePath;  ///< 可选：磁盘图像路径（优先于 vision.processImage）
    };

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

/** Mark 定位出参 Packet */
class MarkPointLocateOutput : public common::algorithm::AlgorithmPacket
{
public:
    /** Locate 操作结果字段 */
    struct Fields
    {
        VisionResultMeta meta;          ///< 公共出参元信息
        float            pixelX = 0.0f; ///< Mark 中心 X（像素）
        float            pixelY = 0.0f; ///< Mark 中心 Y（像素）

        bool ok() const { return meta.ok; }
        void setOk(bool v) { meta.ok = v; }
    };

    static constexpr const char kFields[]    = "fields";    ///< Locate 结果字段键
    static constexpr const char kSdkReady[]  = "sdkReady";  ///< SDK 是否就绪
    static constexpr const char kSdkDetail[] = "sdkDetail"; ///< SDK 加载详情

    void setFields(Fields f) { Set(kFields, std::move(f)); }
    Fields       *getFields() { return Get<Fields>(kFields); }
    const Fields *getFields() const { return Get<Fields>(kFields); }

    void setSdkReady(bool v) { Set(kSdkReady, v); }
    bool *sdkReady() { return Get<bool>(kSdkReady); }

    void setSdkDetail(std::string detail) { Set(kSdkDetail, std::move(detail)); }
    std::string *sdkDetail() { return Get<std::string>(kSdkDetail); }
};

} // namespace vision_algo
