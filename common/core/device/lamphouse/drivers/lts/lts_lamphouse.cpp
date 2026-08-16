#include "lts_lamphouse.h"

namespace common
{
namespace device
{

LtsLamphouse::LtsLamphouse()
    : XorFrameLamphouse('#')
{
}

void LtsLamphouse::applyProductModel()
{
    switch (productModel_) {
    case LtsProductModel::LTS_3DPC24:
        setFrameHeader('$');
        break;
    case LtsProductModel::LTS_3DPA24:
    default:
        setFrameHeader('#');
        break;
    }
}

void LtsLamphouse::reflectFromJson(const json& j)
{
    LamphouseBase::reflectFromJson(j);

    if (j.contains("type") && j["type"].is_string()) {
        const std::string typeName = j["type"].get<std::string>();
        if (typeName == "LDN_LTS_3DPC24") {
            productModel_ = LtsProductModel::LTS_3DPC24;
        } else if (typeName == "LDN_LTS_3DPA24" || typeName == "LDN_LTS") {
            productModel_ = LtsProductModel::LTS_3DPA24;
        }
    }

    if (j.contains("connOpts") && !j["connOpts"].is_null()) {
        LtsProductModel model = productModel_;
        getFromJson(j["connOpts"], "productModel", model);
        productModel_ = model;
    }

    applyProductModel();
}

void LtsLamphouse::reflectToJson(json& j) const
{
    LamphouseBase::reflectToJson(j);
    j["connOpts"]["productModel"] = productModel_;
}

const std::string& LtsLamphouse::open_all_chc()
{
    return encodeAllChannelsCmd('5');
}

const std::string& LtsLamphouse::close_all_chc()
{
    return encodeAllChannelsCmd('6');
}

} // namespace device
} // namespace common
