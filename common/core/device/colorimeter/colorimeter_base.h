#ifndef _COLORIMETER_BASE_HH
#define _COLORIMETER_BASE_HH

#include <stdint.h>
#include <string>
#include <functional>
#include <common/utility/json_tool/reflector.h>
#include <common/core/comm/serial/serial_master_opts.h>
#include "colorimeter_global.h"

namespace common
{
namespace device
{

// 色度计设备型号定义
enum ColorimeterDevType {
    KONICA_MINOLTA = 0,         // 柯尼卡美能达
    UNKNOWN                     // 未知型号
}; 
NLOHMANN_JSON_SERIALIZE_ENUM(ColorimeterDevType, {
{ ColorimeterDevType::KONICA_MINOLTA,       "KONICA_MINOLTA" },
{ ColorimeterDevType::UNKNOWN,              "UNKNOWN" }
    });

// 色度计设备信息
struct ColorimeterDevInfo {
    uint16_t devNO = 0;                             // 设备编号
    std::string ip = "";                            // IP地址
    uint16_t port = 0;                              // 端口
    common::serial::SerialInfo serialInfo;         // 串口信息
    void reflectFromJson(const json& j)
    {
        getFromJson(j, "devNO", devNO);
        getFromJson(j, "ip", ip);
        getFromJson(j, "port", port);
        fromJson(j, "serialInfo", serialInfo);
    }
    void reflectToJson(json& j)const
    {
        j["devNO"] = devNO;
        j["ip"] = ip;
        j["port"] = port;
        toJson(j["serialInfo"], serialInfo);
    }
};


class COLORIMETER_EXPORT ColorimeterBase
{
public:
    ColorimeterBase() = default;
    virtual ~ColorimeterBase() = default;
    void reflectFromJson(const json& j)
    {
        fromJson(j, "connOpts", m_colorimeterDevInfo_);
        getFromJson(j, "type", m_typeName);
    }
    void reflectToJson(json& j)const
    {
        toJson(j["connOpts"], m_colorimeterDevInfo_);
        j["type"] = m_typeName;
    }
public:
    //初始化设备
    virtual bool init() = 0;

    //连接设备
    virtual bool connect() = 0;

    //断开设备
    virtual bool disconnect() = 0;

    //采集数据
    virtual bool measureData(double& cx, double& cy, double& lum) = 0;

    // 连接状态
    virtual bool isConnected() = 0;

protected:
    ColorimeterDevInfo m_colorimeterDevInfo_;                               // 设备信息  
    ColorimeterDevType m_typeName = ColorimeterDevType::KONICA_MINOLTA;     // 设备型号

};

}

}

#endif
