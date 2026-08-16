#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "regaccess_global.h"

namespace common::regaccess
{

enum class RegErrorCode : std::uint8_t
{
    Ok = 0,
    InvalidAddress,
    NotConnected,
    Timeout,
    ProtocolError,
    TransportError,
    Unknown
};

/**
 * @brief 统一寄存器访问接口
 *
 * 设计目标：
 *   - 为上层提供与具体协议无关的寄存器读写接口
 *   - 支持常用标量类型：uint16/uint32/float 以及字符串/字节序列
 *   - 以字符串形式统一描述地址，由各协议自行解释（如：modbus 纯数字地址，melsec/keyence 支持 D100 等）
 */

/// 连接类型
enum class ConnectType : std::uint8_t
{
    Tcp  = 0,
    SerialRtu = 1,
};

/// 连接参数统一结构体（所有具体协议按需使用其中字段）
struct ConnectOptions
{
    /// 连接类型：TCP 或 串口 RTU
    ConnectType type = ConnectType::Tcp;

    // ---- TCP 参数 --------------------------------------------------------
    /// 远端主机/IP（如 "192.168.0.10"）
    std::string host;
    /// 远端端口（如 502、6000 等）
    int         port        = 0;

    /// 是否开启重连（仅对基于 TCP 的客户端有效，例如 Keyence/Melsec）
    bool        tcpReconnEnable   = true;
    /// 重连最小/最大间隔（毫秒）
    int         tcpReconnMinDelay = 3000;
    int         tcpReconnMaxDelay = 9000;
    /// 重连延迟策略：0=fixed, 1=linear, 其他=exponential（对应 `CreateCliOpts::reconnDelayPolicy_`）
    int         tcpReconnPolicy   = 0;

    // ---- 串口 RTU 参数 ---------------------------------------------------
    /// 串口设备名（如 "COM3" 或 "/dev/ttyUSB0"），仅在 SerialRtu 模式下使用
    std::string serialDevice;
    int         serialBaud   = 115200;
    char        serialParity = 'N';   ///< 'N'/'E'/'O'
    int         serialDataBits = 8;
    int         serialStopBits = 1;

    /// 连接/响应超时时间（毫秒），具体协议可自行映射为内部超时配置
    int         timeoutMs   = 2000;
    /// 可选的从站/节点 ID，仅 Modbus 等需要从站概念的协议使用
    int         slaveId     = 0;
};

class REGACCESS_EXPORT IRegAccess
{
public:
    virtual ~IRegAccess() = default;

    /// 建立连接/初始化底层通信（如 TCP/RTU 等），入参采用统一的 ConnectOptions
    virtual bool Connect(const ConnectOptions &options) = 0;

    /// 关闭连接/释放资源
    virtual void Disconnect() = 0;

    // ── 位 / 标量寄存器读写（bit / 16 / 32 / float）───────────────────────────────
    /// 单个位（线圈、软元件）的读写
    virtual bool WriteBit(const std::string &address, bool value) = 0;
    virtual bool ReadBit(const std::string &address, bool &value) = 0;

    virtual bool WriteUInt16(const std::string &address, std::uint16_t value) = 0;
    virtual bool ReadUInt16(const std::string &address, std::uint16_t &value) = 0;

    virtual bool WriteUInt32(const std::string &address, std::uint32_t value) = 0;
    virtual bool ReadUInt32(const std::string &address, std::uint32_t &value) = 0;

    virtual bool WriteFloat(const std::string &address, float value) = 0;
    virtual bool ReadFloat(const std::string &address, float &value) = 0;

    // ── 多寄存器批量读写（word 数组）───────────────────────────────────────────────
    virtual bool WriteRegisters(const std::string &address,
                                const std::vector<std::uint16_t> &values) = 0;
    virtual bool ReadRegisters(const std::string &address,
                               std::uint16_t regCnt,
                               std::vector<std::uint16_t> &values) = 0;

    // ── 字符串与原始字节序读写（按寄存器数 regNum 控制长度）────────────────────
    /// 写入字符串数据（底层实现负责将字符串拆分为寄存器）
    virtual bool WriteString(const std::string &address,
                             const std::string &value,
                             std::uint16_t regNum) = 0;

    /// 读取字符串数据（底层实现负责将寄存器合成为字符串）
    virtual bool ReadString(const std::string &address,
                            std::uint16_t regNum,
                            std::string &value) = 0;

    /// 写入原始字节序（常用于结构体或自定义协议封装）
    virtual bool WriteBytes(const std::string &address,
                            const std::vector<std::uint8_t> &value) = 0;

    /// 读取原始字节序（regNum 为寄存器数量，实际字节数为 regNum * 2）
    virtual bool ReadBytes(const std::string &address,
                           std::uint16_t regNum,
                           std::vector<std::uint8_t> &value) = 0;

    /// 获取最近一次操作的错误码
    virtual RegErrorCode GetLastErrorCode() const = 0;
    /// 获取最近一次操作的错误描述（仅用于日志/调试）
    virtual std::string GetLastErrorMessage() const = 0;
};

} // namespace common::regaccess

