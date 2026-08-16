#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <common/3rdparty/cserialport/include/CSerialPort/SerialPort.h>
#include <common/utility/json_tool/reflector.h>
namespace common
{
namespace device
{
enum class BatteryMode {
    Mode_VI,//普通模式，支持
    Mode_CP,//恒功率模式，支持
    Mode_SEQ,//序列模式，未支持
};
struct LampDevInfo {
    std::string port{""};
    uint32_t baudrate = 115200;
    uint16_t slaveId = 160;
    void reflectFromJson(const json& j) {
        getFromJson(j, "port", port);
        getFromJson(j, "baudrate", baudrate);
        getFromJson(j, "slaveId", slaveId);
    }
    void reflectToJson(json& j)const {
        j["port"] = port;
        j["baudrate"] = baudrate;
        j["slaveId"] = slaveId;
    }
};

//分段控制参数
struct ControlParam {
    uint8_t id = 0;//序号
    float u = 0.0;//单位：V
    float i = 0.0;//单位：A
    uint32_t time = 0;//单位：ms
};
class HalogenLamp {
public:

    void reflectFromJson(const json& j) {
        fromJson(j, "devInfo", _devInfo);
    }
    void reflectToJson(json& j)const {
        toJson(j["devInfo"], _devInfo);
    }

    HalogenLamp();
    virtual ~HalogenLamp();
    bool initSerialPort(const LampDevInfo& lampParam);//初始化串口

    bool setVoltage(float value);
    bool setCurrent(float value);
    bool setParam(float voltage, float current);

    bool getVoltage(float& value);
    bool getCurrent(float& value);

protected:
    bool prepareDevice();//设备初始化
    void setBatteryMode(BatteryMode mode);//设置电源工作模式
    void setU(float u);//设置电压
    void setI(float i);//设置电流
    //void setP(float p);//设置功率

    float getU();//获取电压
    float getI();//获取电流
    //float getP();//获取功率

private:
    //设备开关
    bool deviceSwitch(bool on);
    //设定电源工作模式
    bool setDeviceBatteryMode();
    //设置CV/CC优先
    bool setDeviceCccvPriority();
    //设置设备电压
    bool setDeviceU();
    //设置设备电流
    bool setDeviceI();
    //设置设备功率
    //bool setDeviceP();
    bool setFloatValue(const uint16_t _addr, const float& value);
    float getFloatValue(const uint16_t _addr);

private:
    //读寄存器数据
    bool getRegistersValues(const uint16_t& regAddr, std::vector<uint8_t>& value, const uint16_t regCount = 2);
    //写寄存器数据
    bool setRegistersValues(const uint16_t& regAddr, const std::vector<uint16_t>& value, const uint16_t regCount = 2);
    //串口发送数据
    bool sendData(const unsigned char* data, const int& length);
    //串口读取数据
    bool readData(unsigned char* data, int& length);
    //接收数据的CRC校验
    bool checkRecvData(unsigned char* data, const int& length);

private:
    LampDevInfo _devInfo;
    itas109::CSerialPort _serialPort;//串口
    uint8_t _stationId = 160;//设备站号

    BatteryMode _batteryMode = BatteryMode::Mode_VI;//电源工作模式
    float _u = 1.0;//待设置电压
    float _i = 8.0;//待设置电流
    //float _p = 1.0;//待设置功率

    //功能码
    const uint8_t _funcCodeRead = 0x03;
    const uint8_t _funcCodeWrite = 0x10;

    //常用寄存器地址
    //const uint16_t _addrDevState = 0x0002;		//设备状态，r，uint32
    //const uint16_t _addrDevEvent = 0x0004;		//设备事件，wr，uint32

    //获取数据
    const uint16_t _addrGetU = 0x0006;			//回显电压，r，单位V，float
    const uint16_t _addrGetI = 0x0008;			//回显电流，r，单位A，float
    //const uint16_t _addrGetP = 0x000A;			//回显功率，r，单位W，float

    const uint16_t _addrSwitch = 0x0014;		//ON/OFF输出开关，rw，uint32，0关，1开
    const uint16_t _addrBatteryMode = 0x0016;	//电源工作模式，rw，uint32，0普通，126外部编程，127恒功率，128序列SEQ
    const uint16_t _addrCccvPriority = 0x001A;	//CV/CC 优先设置，rw，uint32，1-CV 优先；2-CC 优先，应该是优先电压或者优先电流

    //普通模式VI
    const uint16_t _addrVISetU = 0x0028;		//电压设定，float，单位V
    //const uint16_t _addrVISetUT = 0x0047;		//电压上升时间，uint32，单位ms
    const uint16_t _addrVISetI = 0x002A;		//电流设定，float，单位A
    //const uint16_t _addrVISetIT = 0x0048;		//电流上升时间，uint32，单位ms
    //const uint16_t _addrVISetR = 0x002C;		//电源内阻，float，单位mΩ

    //恒功率模式CP
    //const uint16_t _addrCPSetU = 0x0030;		//电压设定，float，单位V
    //const uint16_t _addrCPSetI = 0x0032;		//电流设定，float，单位A
    //const uint16_t _addrCPSetP = 0x0034;		//功率设定，float，单位W

    //序列模式SEQ
    std::mutex _mutex;
};

}

}