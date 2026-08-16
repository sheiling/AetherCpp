#ifndef _FLASH_DETECT_BY_SERIAL_HH
#define _FLASH_DETECT_BY_SERIAL_HH

#include "../flash_detect_base.h"
#include "flash_detect_serial_port.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace common
{
namespace device
{

class FLASH_DETECT_EXPORT FlashDetectBySerial : public FlashDetectBase
{
public:
    struct SerialConnOptions
    {
        std::string deviceName = "";
        std::string portName = "COM1";
        int baudRate = 115200;
        int stopBits = 0;
        int dataBits = 8;
        int parity = 0;
        int flowControl = 0;

        void reflectFromJson(const json& j);
        void reflectToJson(json& j) const;
    };

    struct FlashCheckOptions
    {
        uint8_t destAddr = 1;
        uint8_t portAddr = 1;
        uint16_t boardAddr = 1;
        int maxRetries = 100;
        int retryIntervalMs = 200;
        uint32_t expectedStatus = 0x00000005;

        void reflectFromJson(const json& j);
        void reflectToJson(json& j) const;
    };

    // Flash 检测协议的三段命令类型：写触发、读轮询、读最终结果。
    enum class CmdType : uint8_t
    {
        WriteRequestCheck = 0,
        ReadRepeat,
        ReadResult,
    };

    // 单次命令执行结果。
    enum class CmdStatus : uint8_t
    {
        Ready = 0,
        Success,
        Failed,
    };

    FlashDetectBySerial();
    ~FlashDetectBySerial() override = default;

    bool IsOpened() const override { return isOpened_; }

    int OpenComm() override;
    int OpenComm(const char* portName, const ISerialPort::Parameters& param);
    int CloseComm() override;
    int CheckFlash() override;
    int CheckFlash(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) override;

protected:
    void reflectConfigFromJson(const json& j) override;
    void reflectConfigToJson(json& j) const override;

private:
    // 发送单条命令并解析响应，结果写入 cmdStatus_。
    bool SendCommand(const std::vector<uint8_t>& cmdData, CmdType cmdType);
    // 构建“写请求触发检测”报文。
    std::vector<uint8_t> BuildRequestCheckPacket(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) const;
    // 构建“读轮询状态”报文。
    std::vector<uint8_t> BuildRequestRepeatPacket(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) const;
    // 构建“读最终结果”报文。
    std::vector<uint8_t> BuildReadResultPacket(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) const;
    // 按协议头+payload+checksum 组装完整报文。
    std::vector<uint8_t> BuildPacket(uint8_t code,
                                     uint32_t unitAddr,
                                     uint16_t payloadLen,
                                     const uint8_t* payloadData,
                                     uint8_t destAddr,
                                     uint8_t portAddr,
                                     uint16_t boardAddr) const;
    // 写入协议校验和（末尾 2 字节，小端）。
    static void FillChecksum(std::vector<uint8_t>& data);
    // 生成协议标签号（循环递增）。
    static uint8_t NextLabel();
    // 更新串口打开状态，状态变化时通过 MessageCenter 通知。
    void SetOpenedState(bool opened);

    // 最大接收缓冲区长度（字节）。
    static constexpr size_t kRecvPacketLength = 2048;
    // 协议校验常量。
    static constexpr uint16_t kCheckSumConstVal = 0x5555;
    // 校验字段长度（字节）。
    static constexpr size_t kCheckOutLen = 2;

    std::string portName_;
    bool isOpened_;
    CmdStatus cmdStatus_;
    FlashDetectSerialPort serialPort_;
    std::array<uint8_t, kRecvPacketLength> recvBuffer_;
    SerialConnOptions connOpts_;
    FlashCheckOptions checkOpts_;
};

} // namespace device
} // namespace common

#endif
