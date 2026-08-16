#pragma once

/**
 * @file barcode_scan_data.h
 * @brief 条码/二维码扫描单元入参、出参（继承 AlgorithmPacket）
 *
 * 经 execute(kUnitBarcodeScan, data) 调用；解码基于 ZXing。
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>

#include <nlohmann/json.hpp>

#include <QPointF>
#include <QString>
#include <QVector>

namespace vision_algo {

/** 识别到的码制类型 */
enum class BarcodeCodeType
{
    Unknown = 0, ///< 未识别或未知
    Barcode = 1, ///< 一维条码
    QrCode  = 2, ///< 二维码
};
NLOHMANN_JSON_SERIALIZE_ENUM(BarcodeCodeType, {
    { BarcodeCodeType::Unknown, "Unknown" },
    { BarcodeCodeType::Barcode, "Barcode" },
    { BarcodeCodeType::QrCode, "QrCode" },
})

/** 条码扫描入参 Packet */
class BarcodeScanInput : public common::algorithm::AlgorithmPacket
{
public:
    static constexpr const char kVision[] = "vision"; ///< 图像上下文键

    void setVision(VisionImageContext ctx) { Set(kVision, std::move(ctx)); }
    VisionImageContext       *vision() { return Get<VisionImageContext>(kVision); }
    const VisionImageContext *vision() const { return Get<VisionImageContext>(kVision); }
};

/** 条码扫描出参 Packet */
class BarcodeScanOutput : public common::algorithm::AlgorithmPacket
{
public:
    /** 解码结果字段 */
    struct Fields
    {
        VisionResultMeta meta;                        ///< 公共出参元信息
        BarcodeCodeType  codeType = BarcodeCodeType::Unknown; ///< 码制类型
        QString          content;                     ///< 解码文本内容
        QString          symbology;                   ///< ZXing 符号名（如 QRCode）
        QVector<QPointF> cornerPoints;                ///< 四角像素坐标（若有）

        bool ok() const { return meta.ok; }
        void setOk(bool v) { meta.ok = v; }

        bool operator==(const Fields &o) const
        {
            return meta.ok == o.meta.ok && codeType == o.codeType && content == o.content
                   && symbology == o.symbology && cornerPoints == o.cornerPoints;
        }
        bool operator!=(const Fields &o) const { return !(*this == o); }
    };

    static constexpr const char kResult[] = "result"; ///< 解码结果键

    void setResult(Fields r) { Set(kResult, std::move(r)); }
    Fields       *result() { return Get<Fields>(kResult); }
    const Fields *result() const { return Get<Fields>(kResult); }
};

/** 条码扫描业务出参（AOI / 配方页展示用） */
using BarcodeScanResult = BarcodeScanOutput::Fields;

} // namespace vision_algo
