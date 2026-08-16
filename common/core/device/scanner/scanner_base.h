#ifndef _SCANNER_BASE_HH
#define _SCANNER_BASE_HH

#include <stdint.h>
#include <string>
#include <functional>
#include <common/utility/json_tool/reflector.h>
#include <common/core/comm/serial/serial_master_opts.h>
#include "scanner_global.h"

namespace common
{
namespace device
{

// 扫码器设备型号定义
enum ScannerDevType {
    SSDN_HAIKANG = 0,           // 海康
    SSDN_CINO = 1,              // CINO
    SSDN_UNKNOWN                // 未知型号
}; 
NLOHMANN_JSON_SERIALIZE_ENUM(ScannerDevType, {
{ ScannerDevType::SSDN_HAIKANG,      "SSDN_HAIKANG" },
{ ScannerDevType::SSDN_CINO,         "SSDN_CINO" },
{ ScannerDevType::SSDN_UNKNOWN,      "SSDN_UNKNOWN" }
    });

// 扫码器设备信息
struct ScannerDevInfo {
    uint16_t devNO = 0;                             // 设备编号
    std::string ip = "";                            // IP地址
    uint16_t port = 0;                              // 端口
    common::serial::SerialInfo serialInfo;          // 串口信息
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


using ScannerRspCBFunc =  std::function<void(const std::string &payload)>;

class SCANNER_EXPORT ScannerBase {
public:
    ScannerBase() = default;
    virtual ~ScannerBase() = default;
    void reflectFromJson(const json& j)
    {
        fromJson(j, "connOpts", m_scannerDevInfo_);
        getFromJson(j, "type", m_typeName);
		getFromJson(j, "deviceName", m_deviceName);
    }
    void reflectToJson(json& j)const
    {
        toJson(j["connOpts"], m_scannerDevInfo_);
        j["type"] = m_typeName;
		j["deviceName"] = m_deviceName;
    }
public:
	virtual bool Init() = 0;
    virtual bool Request(const std::string &reqData) = 0;
    virtual bool SyncRequest(const std::string& reqData, std::string& rspData) = 0;

    void RegisterRspCB(ScannerRspCBFunc scannerRspCBFunc) {
        scannerRspCBFunc_ = scannerRspCBFunc;
    }

    bool IsConnected() const { return isConnected_; }

protected:
    void SetConnectedState(bool connected);

    std::string m_deviceName = "";              // 设备名称（配置键名）
    ScannerDevInfo m_scannerDevInfo_;           // 设备信息  
    ScannerDevType m_typeName = SSDN_UNKNOWN;   // 设备型号
    ScannerRspCBFunc scannerRspCBFunc_ = nullptr;

    bool isConnected_ = false;
};

}

}

#endif
