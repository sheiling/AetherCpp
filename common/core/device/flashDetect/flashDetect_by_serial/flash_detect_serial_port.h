#ifndef _FLASH_DETECT_SERIAL_PORT_HH
#define _FLASH_DETECT_SERIAL_PORT_HH

#include "../flash_detect_global.h"

namespace common
{
namespace device
{

class ISerialPort
{
public:
    // 串口流控模式。
    enum class FlowControl
    {
        NoFlowControl = 0,
        HardwareControl,
        SoftwareControl
    };

    // 串口奇偶校验模式。
    enum class Parity
    {
        NoParity = 0,
        OddParity,
        EvenParity
    };

    // 串口停止位配置。
    enum class StopBits
    {
        One = 0,
        OneAndHalf,
        Two
    };

    // 串口数据位配置。
    enum class DataBits
    {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8
    };

    // 串口打开参数。
    struct Parameters
    {
        int baudRate = 9600;
        StopBits stopBits = StopBits::One;
        DataBits dataBits = DataBits::Data8;
        Parity parity = Parity::NoParity;
        FlowControl flowControl = FlowControl::NoFlowControl;
    };

    virtual ~ISerialPort() = default;
    // 打开串口。
    virtual bool Open(const char* portName, const Parameters& param) = 0;
    // 关闭串口。
    virtual void Close() = 0;
    // 发送数据，返回实际发送字节数，失败返回 0。
    virtual int Send(const void* buffer, int len) = 0;
    // 接收数据，返回实际接收字节数，失败返回 0。
    virtual int Receive(void* buffer, int maxLen) = 0;
};

class FLASH_DETECT_EXPORT FlashDetectSerialPort : public ISerialPort
{
public:
    FlashDetectSerialPort();
    ~FlashDetectSerialPort() override;

    // ISerialPort 实现：Windows 下基于 CreateFile/ReadFile/WriteFile。
    bool Open(const char* portName, const Parameters& param) override;
    void Close() override;
    int Send(const void* buffer, int len) override;
    int Receive(void* buffer, int maxLen) override;

private:
    // Windows 串口句柄（void* 仅为头文件去平台类型依赖）。
    void* handle_;
    // 是否使用同步 IO（false 时采用 Overlapped 模式）。
    bool useSynchronousIo_;
};

} // namespace device
} // namespace common

#endif
