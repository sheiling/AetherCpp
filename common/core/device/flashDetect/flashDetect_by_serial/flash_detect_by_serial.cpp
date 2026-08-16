#include "flash_detect_by_serial.h"

#include <common/core/messagecenter/message_center.h>

#include <Logger.h>
#include <QString>

#include <chrono>
#include <cstring>
#include <thread>

namespace common
{
namespace device
{

namespace
{
// 协议固定帧头：请求帧 0xAA55，响应帧 0x55AA。
constexpr uint16_t kRequestMark = 0xAA55;
constexpr uint16_t kResponseMark = 0x55AA;

enum class TransactionCode : uint8_t
{
    Read = 0x00,
    Write = 0x01,
};

enum class ResponseCode : uint8_t
{
    Success = 0x00,
};

#pragma pack(push, 1)
// 串口协议头（与设备固件保持 1 字节对齐）。
struct PacketHeader
{
    uint16_t mark;
    uint8_t ack;
    uint8_t labels;
    uint8_t srcAddr;
    uint8_t destAddr;
    uint8_t deviceType;
    uint8_t portAddr;
    uint16_t boardAddr;
    uint8_t code;
    uint8_t packetType;
    uint32_t registerUnitAddr;
    uint16_t validDataLen;
};
#pragma pack(pop)

constexpr size_t kHeaderLen = sizeof(PacketHeader);

int NormalizeEnumValue(int value, int minValue, int maxValue, int defaultValue) {
    if (value < minValue || value > maxValue) {
        return defaultValue;
    }
    return value;
}
} // namespace

void FlashDetectBySerial::SerialConnOptions::reflectFromJson(const json& j) {
    getFromJson(j, "deviceName", deviceName);
    getFromJson(j, "portName", portName);
    getFromJson(j, "baudRate", baudRate);
    getFromJson(j, "stopBits", stopBits);
    getFromJson(j, "dataBits", dataBits);
    getFromJson(j, "parity", parity);
    getFromJson(j, "flowControl", flowControl);

    stopBits = NormalizeEnumValue(stopBits, 0, 2, 0);
    dataBits = NormalizeEnumValue(dataBits, 5, 8, 8);
    parity = NormalizeEnumValue(parity, 0, 2, 0);
    flowControl = NormalizeEnumValue(flowControl, 0, 2, 0);
}

void FlashDetectBySerial::SerialConnOptions::reflectToJson(json& j) const {
    j["deviceName"] = deviceName;
    j["portName"] = portName;
    j["baudRate"] = baudRate;
    j["stopBits"] = stopBits;
    j["dataBits"] = dataBits;
    j["parity"] = parity;
    j["flowControl"] = flowControl;
}

void FlashDetectBySerial::FlashCheckOptions::reflectFromJson(const json& j) {
    getFromJson(j, "destAddr", destAddr);
    getFromJson(j, "portAddr", portAddr);
    getFromJson(j, "boardAddr", boardAddr);
    getFromJson(j, "maxRetries", maxRetries);
    getFromJson(j, "retryIntervalMs", retryIntervalMs);
    getFromJson(j, "expectedStatus", expectedStatus);
}

void FlashDetectBySerial::FlashCheckOptions::reflectToJson(json& j) const {
    j["destAddr"] = destAddr;
    j["portAddr"] = portAddr;
    j["boardAddr"] = boardAddr;
    j["maxRetries"] = maxRetries;
    j["retryIntervalMs"] = retryIntervalMs;
    j["expectedStatus"] = expectedStatus;
}

FlashDetectBySerial::FlashDetectBySerial()
    : isOpened_(false),
      cmdStatus_(CmdStatus::Ready) {
    m_typeName = FlashDetectDevType::FD_BY_SERIAL;
    // 初始化接收缓存，避免后续解析读到脏数据。
    recvBuffer_.fill(0);
}

void FlashDetectBySerial::reflectConfigFromJson(const json& j) {
    fromJson(j, "connOpts", connOpts_);
    fromJson(j, "checkOpts", checkOpts_);
}

void FlashDetectBySerial::reflectConfigToJson(json& j) const {
    toJson(j["connOpts"], connOpts_);
    toJson(j["checkOpts"], checkOpts_);
}

void FlashDetectBySerial::SetOpenedState(bool opened) {
    if (isOpened_ == opened) {
        return;
    }
    isOpened_ = opened;

    const QString key = connOpts_.deviceName.empty()
        ? QString::fromStdString(portName_)
        : QString::fromStdString(connOpts_.deviceName);
    MessageCenter::Center::Notify(EVENT_FLASH_DETECT_STATE, key, opened);
}

int FlashDetectBySerial::OpenComm() {
    ISerialPort::Parameters param;
    param.baudRate = connOpts_.baudRate;
    param.stopBits = static_cast<ISerialPort::StopBits>(connOpts_.stopBits);
    param.dataBits = static_cast<ISerialPort::DataBits>(connOpts_.dataBits);
    param.parity = static_cast<ISerialPort::Parity>(connOpts_.parity);
    param.flowControl = static_cast<ISerialPort::FlowControl>(connOpts_.flowControl);
    return OpenComm(connOpts_.portName.c_str(), param);
}

int FlashDetectBySerial::OpenComm(const char* portName, const ISerialPort::Parameters& param) {
    if (isOpened_) {
        LOG_INFO("[FlashDetectBySerial] serial already open, port={}", portName_);        
        return 0;
    }
    if (!serialPort_.Open(portName, param)) {
        LOG_ERROR("[FlashDetectBySerial] open serial failed, port={}", portName);
        return -1;
    }
    portName_ = portName;
    SetOpenedState(true);
    LOG_INFO("[FlashDetectBySerial] open serial success, port={}", portName);
    return 0;
}

int FlashDetectBySerial::CloseComm() {
    if (isOpened_) {
        serialPort_.Close();
        SetOpenedState(false);
        LOG_INFO("[FlashDetectBySerial] close serial success, port={}", portName_);
    }
    return 0;
}

int FlashDetectBySerial::CheckFlash() {
    return CheckFlash(checkOpts_.destAddr, checkOpts_.portAddr, checkOpts_.boardAddr);
}

bool FlashDetectBySerial::SendCommand(const std::vector<uint8_t>& cmdData, CmdType cmdType) {
    if (cmdData.empty()) {
        LOG_ERROR("[FlashDetectBySerial] command packet is empty, cmd={}", static_cast<int>(cmdType));
        return false;
    }

    // 先发后收：每条命令对应一次完整请求-响应交互。
    const int sentLen = serialPort_.Send(cmdData.data(), static_cast<int>(cmdData.size()));
    if (sentLen <= 0) {
        LOG_ERROR("[FlashDetectBySerial] send command failed, cmd={}, req_len={}",
                  static_cast<int>(cmdType),
                  static_cast<int>(cmdData.size()));
        return false;
    }

    LOG_INFO("[FlashDetectBySerial] send command success, cmd={}, req_len={}",
             static_cast<int>(cmdType),
             sentLen);
    const int recvLen = serialPort_.Receive(recvBuffer_.data(), static_cast<int>(recvBuffer_.size()));
    if (recvLen <= static_cast<int>(kHeaderLen)) {
        LOG_WARN("[FlashDetectBySerial] response length invalid, cmd={}, rsp_len={}, header_len={}",
                 static_cast<int>(cmdType),
                 recvLen,
                 static_cast<int>(kHeaderLen));
        cmdStatus_ = CmdStatus::Failed;
        return true;
    }

    // 读取响应头，按 code 区分读命令/写命令的帧长度计算方式。
    auto* header = reinterpret_cast<const PacketHeader*>(recvBuffer_.data());
    const size_t payloadLen = header->validDataLen;
    const size_t packetLen = header->code == static_cast<uint8_t>(TransactionCode::Read)
                                 ? (kHeaderLen + payloadLen + kCheckOutLen)
                                 : (kHeaderLen + kCheckOutLen);
    if (static_cast<size_t>(recvLen) < packetLen) {
        LOG_WARN("[FlashDetectBySerial] response packet incomplete, cmd={}, rsp_len={}, expect_len={}",
                 static_cast<int>(cmdType),
                 recvLen,
                 static_cast<int>(packetLen));
        cmdStatus_ = CmdStatus::Failed;
        return true;
    }

    cmdStatus_ = CmdStatus::Ready;
    if (header->mark == kResponseMark && header->ack == static_cast<uint8_t>(ResponseCode::Success)) {
        cmdStatus_ = CmdStatus::Success;
        if (cmdType == CmdType::ReadResult && payloadLen >= sizeof(uint32_t)) {
            // 最终结果命令的 payload 为 4 字节状态码。
            uint32_t flashStatus = 0;
            memcpy(&flashStatus, recvBuffer_.data() + kHeaderLen, sizeof(uint32_t));
            if (flashStatus != checkOpts_.expectedStatus) {
                cmdStatus_ = CmdStatus::Failed;
            }
            LOG_INFO("[FlashDetectBySerial] flash result status={}, expect={}", flashStatus, checkOpts_.expectedStatus);
        }
    } else {
        LOG_ERROR("[FlashDetectBySerial] response verify failed, cmd={}, mark=0x{:04X}, ack={}",
                  static_cast<int>(cmdType),
                  header->mark,
                  static_cast<int>(header->ack));
        cmdStatus_ = CmdStatus::Failed;
    }
    return true;
}

uint8_t FlashDetectBySerial::NextLabel() {
    // label 在协议中用于请求-响应关联，采用循环递增。
    static uint8_t label = 0x51;
    return label == 255 ? (label = 0) : (++label);
}

void FlashDetectBySerial::FillChecksum(std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        return;
    }

    // 协议校验：从偏移 2 开始累加到倒数第 3 字节，再加固定常量。
    uint16_t checksum = 0;
    for (size_t i = 2; i < data.size() - 2; ++i) {
        checksum += data[i];
    }
    checksum += kCheckSumConstVal;
    data[data.size() - 2] = static_cast<uint8_t>(checksum);
    data[data.size() - 1] = static_cast<uint8_t>(checksum >> 8);
}

std::vector<uint8_t> FlashDetectBySerial::BuildPacket(uint8_t code,
                                                      uint32_t unitAddr,
                                                      uint16_t payloadLen,
                                                      const uint8_t* payloadData,
                                                      uint8_t destAddr,
                                                      uint8_t portAddr,
                                                      uint16_t boardAddr) const {
    // 组装协议头。
    PacketHeader header;
    memset(&header, 0, sizeof(header));
    header.mark = kRequestMark;
    header.labels = NextLabel();
    header.srcAddr = 0xFE;
    header.destAddr = destAddr;
    header.deviceType = 0x01;
    header.portAddr = portAddr;
    header.boardAddr = boardAddr;
    header.code = code;
    header.packetType = 0x00;
    header.registerUnitAddr = unitAddr;
    header.validDataLen = payloadLen;

    const size_t packetLen = kHeaderLen + payloadLen + kCheckOutLen;
    std::vector<uint8_t> packet(packetLen, 0);
    memcpy(packet.data(), &header, kHeaderLen);
    // 按需拷贝 payload（读命令 payload 长度可非 0，但 payload 内容为空）。
    if (payloadLen > 0 && payloadData != nullptr) {
        memcpy(packet.data() + kHeaderLen, payloadData, payloadLen);
    }
    FillChecksum(packet);
    return packet;
}

std::vector<uint8_t> FlashDetectBySerial::BuildRequestCheckPacket(uint8_t destAddr,
                                                                   uint8_t portAddr,
                                                                   uint16_t boardAddr) const {
    const uint8_t payloadData = 0x04;
    return BuildPacket(static_cast<uint8_t>(TransactionCode::Write),
                       0x01000074,
                       0x0001,
                       &payloadData,
                       destAddr,
                       portAddr,
                       boardAddr);
}

std::vector<uint8_t> FlashDetectBySerial::BuildRequestRepeatPacket(uint8_t destAddr,
                                                                    uint8_t portAddr,
                                                                    uint16_t boardAddr) const {
    return BuildPacket(static_cast<uint8_t>(TransactionCode::Read),
                       0x02000200,
                       0x0100,
                       nullptr,
                       destAddr,
                       portAddr,
                       boardAddr);
}

std::vector<uint8_t> FlashDetectBySerial::BuildReadResultPacket(uint8_t destAddr,
                                                                 uint8_t portAddr,
                                                                 uint16_t boardAddr) const {
    return BuildPacket(static_cast<uint8_t>(TransactionCode::Read),
                       0x03003010,
                       0x0004,
                       nullptr,
                       destAddr,
                       portAddr,
                       boardAddr);
}

int FlashDetectBySerial::CheckFlash(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) {
    if (!isOpened_) {
        LOG_ERROR("[FlashDetectBySerial] check flash aborted, serial not open, dest={}, port={}, board={}",
                  static_cast<int>(destAddr),
                  static_cast<int>(portAddr),
                  boardAddr);
        return -1;
    }

    LOG_INFO("[FlashDetectBySerial] check flash begin, dest={}, port={}, board={}",
             static_cast<int>(destAddr),
             static_cast<int>(portAddr),
             boardAddr);
    // 步骤1：写命令触发设备进入 Flash 检测流程。
    const std::vector<uint8_t> checkPacket = BuildRequestCheckPacket(destAddr, portAddr, boardAddr);
    if (!SendCommand(checkPacket, CmdType::WriteRequestCheck) || cmdStatus_ != CmdStatus::Success) {
        LOG_ERROR("[FlashDetectBySerial] step1 trigger check failed");
        return -1;
    }

    // 步骤2：轮询检测完成标记，最多重试 100 次，每次间隔 200ms。
    const std::vector<uint8_t> repeatPacket = BuildRequestRepeatPacket(destAddr, portAddr, boardAddr);
    bool repeatOk = false;
    for (int i = 0; i < checkOpts_.maxRetries; ++i) {
        if (SendCommand(repeatPacket, CmdType::ReadRepeat) && cmdStatus_ == CmdStatus::Success) {
            repeatOk = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(checkOpts_.retryIntervalMs));
    }
    if (!repeatOk) {
        LOG_ERROR("[FlashDetectBySerial] step2 polling failed after max retries");
        return -1;
    }

    // 步骤3：读取最终检测结果。
    const std::vector<uint8_t> resultPacket = BuildReadResultPacket(destAddr, portAddr, boardAddr);
    if (!SendCommand(resultPacket, CmdType::ReadResult) || cmdStatus_ != CmdStatus::Success) {
        LOG_ERROR("[FlashDetectBySerial] step3 read result failed");
        return -1;
    }

    LOG_INFO("[FlashDetectBySerial] check flash success, dest={}, port={}, board={}",
             static_cast<int>(destAddr),
             static_cast<int>(portAddr),
             boardAddr);
    return 0;
}

} // namespace device
} // namespace common
