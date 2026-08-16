#ifndef _LAMPHOUSE_BASE_HH
#define _LAMPHOUSE_BASE_HH

#include <common/utility/json_tool/reflector.h>
#include <common/core/comm/serial/serial_master.h>
#include "lamphouse_global.h"

namespace common
{
namespace device
{

// 光源控制设备品牌/型号定义（同品牌不同硬件型号在驱动子类内用 productModel 区分）
enum LamphouseDevType {
    LDN_HDPS_2C = 0,            // 慕藤光
    LDN_JL_DVC = 1,             // 上海嘉励
    LDN_LTS = 2,                // 东莞乐视
    LDN_CST_DPS = 3,            // 康视达 DPS
    LDN_UNKNOWN                 // 未知型号
};

inline void from_json(const json& j, LamphouseDevType& type)
{
    if (!j.is_string()) {
        type = LamphouseDevType::LDN_UNKNOWN;
        return;
    }
    const std::string name = j.get<std::string>();
    if (name == "LDN_HDPS_2C") {
        type = LamphouseDevType::LDN_HDPS_2C;
    } else if (name == "LDN_JL_DVC") {
        type = LamphouseDevType::LDN_JL_DVC;
    } else if (name == "LDN_LTS" || name == "LDN_LTS_3DPA24" || name == "LDN_LTS_3DPC24") {
        type = LamphouseDevType::LDN_LTS;
    } else if (name == "LDN_CST_DPS") {
        type = LamphouseDevType::LDN_CST_DPS;
    } else {
        type = LamphouseDevType::LDN_UNKNOWN;
    }
}

inline void to_json(json& j, const LamphouseDevType& type)
{
    switch (type) {
    case LamphouseDevType::LDN_HDPS_2C: j = "LDN_HDPS_2C"; break;
    case LamphouseDevType::LDN_JL_DVC: j = "LDN_JL_DVC"; break;
    case LamphouseDevType::LDN_LTS: j = "LDN_LTS"; break;
    case LamphouseDevType::LDN_CST_DPS: j = "LDN_CST_DPS"; break;
    default: j = "LDN_UNKNOWN"; break;
    }
}

// 光源控制设备信息
struct LamphouseDevInfo {
    uint16_t devNO = 0;                                 // 设备编号
    uint16_t channelNumber = 3;                         // 设备编号
    common::serial::SerialInfo serialInfo;              // 串口信息
    void reflectFromJson(const json& j)
    {
        getFromJson(j, "devNO", devNO);
        getFromJson(j, "channelNumber", channelNumber);
        fromJson(j, "serialInfo", serialInfo);
    }
    void reflectToJson(json& j)const
    {
        j["devNO"] = devNO;
        j["channelNumber"] = channelNumber;
        toJson(j["serialInfo"], serialInfo);
    }
};


class LAMPHOUSE_EXPORT LamphouseBase {
public:
    LamphouseBase() = default;
    virtual ~LamphouseBase() = default;
    virtual void reflectFromJson(const json& j);
    virtual void reflectToJson(json& j) const;
public:
    // 初始化串口
    bool Init();
    bool OpenChc(const uint8_t& _ind);
    bool CloseChc(const uint8_t& _ind);
    bool SetBrightness(const uint8_t& _ind, const uint8_t& _val);
    bool OpenAllChc();
    bool CloseAllChc();
    uint16_t GetChannelNum() { return m_lamphouseDevInfo_.channelNumber; }
protected:
    // 打开通道
    virtual const std::string& open_chc(const uint8_t& _ind) = 0;
    // 关闭通道
    virtual const std::string& close_chc(const uint8_t& _ind) = 0;
    // 设置亮度
    virtual const std::string& set_brightness(const int& _ind, const uint8_t& _val) = 0;
    // 打开全部通道
    virtual const std::string& open_all_chc() = 0;
    // 关闭全部通道
    virtual const std::string& close_all_chc() = 0;
protected:
    common::serial::SerialMaster serial_;

    LamphouseDevInfo m_lamphouseDevInfo_;               // 设备信息  
    LamphouseDevType m_typeName = LDN_UNKNOWN;          // 设备型号
};

}

}

#endif
