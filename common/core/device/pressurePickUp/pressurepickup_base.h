#ifndef _PRESSUREPICKUP_BASE_HH
#define _PRESSUREPICKUP_BASE_HH

#include <stdint.h>
#include <string>
#include <vector>
#include <common/core/comm/regaccess/modbus/master/modbus_master.h>
#include <common/core/comm/regaccess/modbus/master/modbus_master_options.h>
#include <common/core/comm/serial/serial_master_opts.h>
#include <common/utility/json_tool/reflector.h>
#include "pressurepickup_global.h"

namespace common
{
namespace device
{
// 压力传感器设备型号定义
enum PressurePickupDevType {
    PPDN_UNKNOWN = 0            // 未知型号
};
NLOHMANN_JSON_SERIALIZE_ENUM(PressurePickupDevType, {
{ PressurePickupDevType::PPDN_UNKNOWN,      "PPDN_UNKNOWN" }
    });
// 压力传感器设备信息
struct PressurePickupDevInfo {
    uint16_t devNO = 0;                                     // 设备编号
    std::string protocolType = "TCP";                       // 协议类型(TCP/RTU)
    std::string ip = "";                                    // IP地址
    uint16_t port = 0;                                      // 端口
    common::serial::SerialInfo serialInfo;                  // 串口信息
    void reflectFromJson(const json& j)
    {
        getFromJson(j, "devNO", devNO);
        getFromJson(j, "protocolType", protocolType);
        getFromJson(j, "ip", ip);
        getFromJson(j, "port", port);
        fromJson(j, "serialInfo", serialInfo);
    }
    void reflectToJson(json& j)const
    {
        j["devNO"] = devNO;
        j["protocolType"] = protocolType;
        j["ip"] = ip;
        j["port"] = port;
        toJson(j["serialInfo"], serialInfo);
    }
};

class PRESSUREPICKUP_EXPORT PressurePickupBase {
public:
    PressurePickupBase() = default;
    virtual ~PressurePickupBase() = default;
    void reflectFromJson(const json& j)
    {
        fromJson(j, "connOpts", m_pressurePickupDevInfo_);
        getFromJson(j, "type", m_typeName);
    }
    void reflectToJson(json& j)const
    {
        toJson(j["connOpts"], m_pressurePickupDevInfo_);
        j["type"] = m_typeName;
    }
public:
    virtual bool Init();
private:
    virtual bool InitByTCP();
    virtual bool InitByRTU();
public:
    // 只有Serial RTU才有切换salve节点ID，TCP是点对点的所以不需要
    virtual int ChangeSlave(int slaveID);
public:
    virtual int WriteBit(int address, uint8_t bitValue);
    virtual int ReadBit(int address, uint8_t &bitValue);
    virtual int WriteBits(int address, const std::vector<uint8_t> &bitValues);
    virtual int ReadBits(int address, uint16_t bitCnt, std::vector<uint8_t> &bitValues);
    virtual int WriteRegister(int address, uint16_t dataValue);
    virtual int WriteRegister(int address, uint32_t dataValue);
    virtual int WriteRegister(int address, float dataValue);
    virtual int ReadRegister(int address, uint16_t &dataValue);
    virtual int ReadRegister(int address, uint32_t &dataValue);
    virtual int ReadRegister(int address, float &dataValue);
    virtual int WriteRegisters(int address, const std::vector<uint16_t> &dataValues);
    virtual int ReadRegisters(int address, uint16_t dataCnt, std::vector<uint16_t> &dataValues);
protected:
    common::regaccess::modbus::ModbusMaster modbusMaster_;
    PressurePickupDevInfo m_pressurePickupDevInfo_;     // 设备信息  
    PressurePickupDevType m_typeName = PPDN_UNKNOWN;    // 设备型号

};

}

}

#endif
