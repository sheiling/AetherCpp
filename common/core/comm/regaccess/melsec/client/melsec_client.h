#ifndef _MELSEC_CLIENT_HH
#define _MELSEC_CLIENT_HH

#include "melsec_request.h"
#include "melsec_response.h"
#include <comm/tcp/client/tcpclient.h>
#include <mutex>
#include <vector>
#include <functional>
#include <string>
#include <chrono>

#include "../../ireg_access.h"
#include "../../fairlock.h"

namespace common::regaccess {
namespace mc {

// 三菱MC协议地址类型（ASCII首字母）与对应二进制代码的映射表
enum class MelsecAddressType : uint8_t {
    // D寄存器（数据寄存器，Data Register）
    D = 0xA8,
    // M寄存器（继电器，Relay/Coil）
    M = 0x90,
    // X输入继电器
    X = 0x9C,
    // Y输出继电器
    Y = 0x9D,
    // L触点
    L = 0x92,
    // F触点
    F = 0x93,
    // V寄存器
    V = 0x94,
    // B寄存器（链接继电器）
    B = 0xA0,
    // R寄存器
    R = 0xAF,
    // ZR寄存器
    ZR = 0xB0,
    // S步进继电器
    S = 0x98,
    // T定时器的接点
    TN = 0xC2,
    // T定时器的线圈
    TC = 0xC0,
    // T定时器的触点
    TS = 0xC1,
    // C计数器的接点
    CN = 0xC5,
    // C计数器的线圈
    CS = 0xC4,
    // C计数器的触点
    CC = 0xC3,
    // Z变址寄存器
    Z = 0xCC,
    // W链接寄存器
    W = 0xB4,
    // 特殊继电器
    SM = 0x91,
    // 特殊存储器
    SD = 0xA9,
};

// 支持常见三菱PLC MC协议的ASCII首字母到 MelsecAddressType 的映射
inline bool MelsecAddressTypeFromAscii(const std::string& ascii, MelsecAddressType& outType) {
    if (ascii == "D") {
        outType = MelsecAddressType::D; return true;
    }
    if (ascii == "M") {
        outType = MelsecAddressType::M; return true;
    }
    if (ascii == "X") {
        outType = MelsecAddressType::X; return true;
    }
    if (ascii == "Y") {
        outType = MelsecAddressType::Y; return true;
    }
    if (ascii == "L") {
        outType = MelsecAddressType::L; return true;
    }
    if (ascii == "F") {
        outType = MelsecAddressType::F; return true;
    }
    if (ascii == "V") {
        outType = MelsecAddressType::V; return true;
    }
    if (ascii == "B") {
        outType = MelsecAddressType::B; return true;
    }
    if (ascii == "R") {
        outType = MelsecAddressType::R; return true;
    }
    if (ascii == "ZR") {
        outType = MelsecAddressType::ZR; return true;
    }
    if (ascii == "S") {
        outType = MelsecAddressType::S; return true;
    }
    if (ascii == "TN") {
        outType = MelsecAddressType::TN; return true;
    }
    if (ascii == "TC") {
        outType = MelsecAddressType::TC; return true;
    }
    if (ascii == "TS") {
        outType = MelsecAddressType::TS; return true;
    }
    if (ascii == "CN") {
        outType = MelsecAddressType::CN; return true;
    }
    if (ascii == "CS") {
        outType = MelsecAddressType::CS; return true;
    }
    if (ascii == "CC") {
        outType = MelsecAddressType::CC; return true;
    }
    if (ascii == "Z") {
        outType = MelsecAddressType::Z; return true;
    }
    if (ascii == "W") {
        outType = MelsecAddressType::W; return true;
    }
    if (ascii == "SM") {
        outType = MelsecAddressType::SM; return true;
    }
    if (ascii == "SD") {
        outType = MelsecAddressType::SD; return true;
    }
    // 支持小写输入
    std::string up(ascii);
    for (auto& c : up) c = ::toupper(c);
    if (up != ascii) {
        return MelsecAddressTypeFromAscii(up, outType);
    }
    return false;
}

using OnConnectionCB = std::function<void(void*, int, int, bool)>;

class REGACCESS_EXPORT MelsecClient : public IRegAccess {
public:
    MelsecClient() = default;
    ~MelsecClient();
    static MelsecClient& getInstance() {
        static MelsecClient client;
        return client;
    }
    int Init(common::tcp::CreateCliOpts &createCliOpts);
    void Release();

    // IRegAccess 接口实现：Melsec 协议仅支持寄存器级别访问，地址使用十进制寄存器编号
    // 通过统一 ConnectOptions 建立 TCP 连接（底层使用 TcpClient）
    bool Connect(const ConnectOptions &options) override;
    void Disconnect() override { Release(); }

    bool WriteBit(const std::string &address, bool value) override;
    bool ReadBit(const std::string &address, bool &value) override;

    bool WriteUInt16(const std::string &address, std::uint16_t value) override;
    bool ReadUInt16(const std::string &address, std::uint16_t &value) override;

    bool WriteUInt32(const std::string &address, std::uint32_t value) override;
    bool ReadUInt32(const std::string &address, std::uint32_t &value) override;

    bool WriteFloat(const std::string &address, float value) override;
    bool ReadFloat(const std::string &address, float &value) override;

    bool WriteRegisters(const std::string &address,
                        const std::vector<std::uint16_t> &values) override;
    bool ReadRegisters(const std::string &address,
                       std::uint16_t regCnt,
                       std::vector<std::uint16_t> &values) override;

    bool WriteString(const std::string &address,
                     const std::string &value,
                     std::uint16_t regNum) override;
    bool ReadString(const std::string &address,
                    std::uint16_t regNum,
                    std::string &value) override;

    bool WriteBytes(const std::string &address,
                    const std::vector<std::uint8_t> &value) override;
    bool ReadBytes(const std::string &address,
                   std::uint16_t regNum,
                   std::vector<std::uint8_t> &value) override;

    void RegisterOnConnectionCB(OnConnectionCB onConnectionCB) {
        onConnectionCB_ = onConnectionCB;
    }

    bool isConnected();

    //刷新起始的空闲时间点
    void refreshAliveTime();
    //计算连接存活的总时长
    uint64_t getAliveTime();
    int port();
private:
    RegErrorCode lastErrorCode_ = RegErrorCode::Ok;
    std::string  lastErrorMsg_;

    void setError(RegErrorCode code, const std::string &msg) {
        lastErrorCode_ = code;
        lastErrorMsg_  = msg;
    }
    void OnConnection(void *userData, int connfd, int connID, bool isConnected);
    void OnMessage(void *userData, int connfd, int connID, const std::string &payload);
    bool sendAndReceive(const MelsecRequest &request, MelsecResponse &response);
    
private:
    OnConnectionCB onConnectionCB_ = nullptr;
    common::tcp::TcpClient tcpClient_;
    std::condition_variable cond_;
    std::mutex mtx_;
    FairLock lock_;
    std::vector<uint8_t> recvBuf_;
    std::chrono::steady_clock::time_point alivetime_; 
    bool isConnected_ = false;
    int port_ = 6000;
public:
    RegErrorCode GetLastErrorCode() const override { return lastErrorCode_; }
    std::string GetLastErrorMessage() const override { return lastErrorMsg_; }
};

} // namespace mc
} // namespace common::regaccess

#endif