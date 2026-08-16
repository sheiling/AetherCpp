#ifndef __COMMON_CORE_COMM_SERIAL_SERIAL_MASTER_OPTS_H__
#define __COMMON_CORE_COMM_SERIAL_SERIAL_MASTER_OPTS_H__

#include <string>
#include <common/utility/json_tool/reflector.h>

namespace common
{

namespace serial
{

enum class ParityMode {
    None = 0,
    Odd = 1,
    Even = 2,
    Mark = 3,
    Space = 4
};
NLOHMANN_JSON_SERIALIZE_ENUM(ParityMode, {
{ ParityMode::None,     "None" },
{ ParityMode::Odd,      "Odd" },
{ ParityMode::Even,     "Even" },
{ ParityMode::Mark,     "Mark" },
{ ParityMode::Space,    "Space" }
    });

struct SerialInfo {

    uint32_t rspTimeout_;       // 响应超时时间(单位:微秒)[0,999999]
    std::string com_;           // 串口号(com口)
    int baud_;                  // 波特率
    ParityMode parity_;         // 串口通信校验方式(N:none, E:even, O:odd)
    int dataBit_;               // 串口通信数据位(5/6/7/8)
    int stopBit_;               // 串口通信停止位(1/2)
    int slaveID_;               // 从站节点

    SerialInfo() {
        rspTimeout_ = 1000 * 1000;
        com_ = "COM10";
        baud_ = 115200;
        dataBit_ = 8;
        parity_ = ParityMode::None;
        stopBit_ = 1;
        slaveID_ = 1;
    }
    void reflectFromJson(const json& j)
    {
        getFromJson(j, "rspTimeout_", rspTimeout_);
        getFromJson(j, "com_", com_);
        getFromJson(j, "baud_", baud_);
        getFromJson(j, "dataBit_", dataBit_);
        getFromJson(j, "parity_", parity_);
        getFromJson(j, "stopBit_", stopBit_);
        getFromJson(j, "slaveID_", slaveID_);
    }
    void reflectToJson(json& j)const
    {
        j["rspTimeout_"] = rspTimeout_;
        j["com_"] = com_;
        j["baud_"] = baud_;
        j["dataBit_"] = dataBit_;
        j["parity_"] = parity_;
        j["stopBit_"] = stopBit_;
        j["slaveID_"] = slaveID_;
    }
};

} 

}

#endif