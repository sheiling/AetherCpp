#pragma once
#ifndef _LTS_LAMPHOUSE_HH
#define _LTS_LAMPHOUSE_HH

#include <common/utility/json_tool/reflector.h>
#include <common/core/device/lamphouse/lamphouse_xor_frame_base.h>

namespace common
{
namespace device
{

// 东莞乐视同品牌下的硬件型号（帧头与通道数等差异在子类内区分）
enum class LtsProductModel {
    LTS_3DPA24,     // 帧头 '#'
    LTS_3DPC24      // 帧头 '$'
};

NLOHMANN_JSON_SERIALIZE_ENUM(LtsProductModel, {
    { LtsProductModel::LTS_3DPA24, "LTS_3DPA24" },
    { LtsProductModel::LTS_3DPC24, "LTS_3DPC24" }
})

class LAMPHOUSE_EXPORT LtsLamphouse : public XorFrameLamphouse {
public:
    LtsLamphouse();
    ~LtsLamphouse() override = default;

    void reflectFromJson(const json& j) override;
    void reflectToJson(json& j) const override;

    const std::string& open_all_chc() override;
    const std::string& close_all_chc() override;

private:
    void applyProductModel();

    LtsProductModel productModel_ = LtsProductModel::LTS_3DPA24;
};

} // namespace device
} // namespace common

#endif
