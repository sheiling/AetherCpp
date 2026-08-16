#ifndef _KV_CLIENT_HH
#define _KV_CLIENT_HH

#include <variant>
#include <vector>
#include <memory>
#include <initializer_list>
#include <string>
#include <comm/tcp/client/tcpclient.h>
#include "../../ireg_access.h"
#include "../../fairlock.h"

namespace common::regaccess {
namespace kv {
// 更改模式
constexpr const char* Command1 = "M";
// 清除错误
constexpr const char* Command2 = "ER";
// 检查错误编号
constexpr const char* Command3 = "?E";
// 查询机型
constexpr const char* Command4 = "?K";
// 检查运行模式
constexpr const char* Command5 = "?M";
// 时间设定
constexpr const char* Command6 = "WRT";
// 强制置位
constexpr const char* Command7 = "ST";
// 强制复位
constexpr const char* Command8 = "RS";
// 连续强制置位
constexpr const char* Command9 = "STS";
// 连续强制复位
constexpr const char* Command10 = "RSS";
// 读取数据
constexpr const char* Command11 = "RD";
// 读取连续数据
constexpr const char* Command12 = "RDS";
// 读取连续数据
constexpr const char* Command13 = "RDE";
// 写入数据
constexpr const char* Command14 = "WR";
// 写入连续数据
constexpr const char* Command15 = "WRS";
// 写入连续数据
constexpr const char* Command16 = "WRE";
// 写入设定值
constexpr const char* Command17 = "WS";
// 写入连续设定值
constexpr const char* Command18 = "WSS";
// 监控器登录
constexpr const char* Command19 = "MBS";
// 监控器登录
constexpr const char* Command20 = "MWS";
// 读取监控器
constexpr const char* Command21 = "MBR";
// 读取监控器
constexpr const char* Command22 = "MWR";
// 注释读取
constexpr const char* Command23 = "RDC";
// 存储体切换
constexpr const char* Command24 = "BE";
// 读取扩展单元缓冲存储器
constexpr const char* Command25 = "URD";
// 写入扩展单元缓冲存储器
constexpr const char* Command26 = "UWR";
// 回车
constexpr const char* CR = "\r";
// 空格
constexpr const char* SP = " ";

using OnConnectionCB = std::function<void(void*, int, int, bool)>;

class REGACCESS_EXPORT KVClient : public IRegAccess
{
public:
    KVClient() = default;
    ~KVClient();
    enum class DataType
    {
        N = 0,
        U,		     
        S,
        D,
        L,
        H
    };

    // 原有初始化/释放接口（保持兼容）
    int Init(common::tcp::CreateCliOpts &createCliOpts);
    void Release();

    // IRegAccess 接口实现（上层通过统一接口访问寄存器），通过统一 ConnectOptions 建立 TCP 连接
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
                     std::uint16_t /*regNum*/) override;
    bool ReadString(const std::string &address,
                    std::uint16_t regNum,
                    std::string &value) override;

    bool WriteBytes(const std::string &address,
                    const std::vector<std::uint8_t> &value) override;
    bool ReadBytes(const std::string &address,
                   std::uint16_t regNum,
                   std::vector<std::uint8_t> &value) override;

    // 改变PLC运行状态等协议特性，仅在类内部通过 IRegAccess 使用，不再对外暴露独立接口
    bool ChangeCPU(bool run = true);
    std::string SeeCPUState();
private:
    RegErrorCode lastErrorCode_ = RegErrorCode::Ok;
    std::string  lastErrorMsg_;

    void setError(RegErrorCode code, const std::string &msg) {
        lastErrorCode_ = code;
        lastErrorMsg_  = msg;
    }

    // 写入字数据
    std::string put(const std::string &address, const std::string &value, DataType tpye);
    // 写入连续字数据
    std::string put(const std::string &address, const std::vector<std::string> &value, DataType tpye);
    // 读取字数据
    std::string get(const std::string &address, DataType tpye = DataType::N);
    // 读取连续字数据
    std::string get(const std::string &address, int count, DataType tpye = DataType::N);

    void OnConnection(void *userData, int connfd, int connID, bool isConnected); 
    void OnMessage(void *userData, int connfd, int connID, const std::string &payload);
    std::string SendRecive(const std::string &str);
    std::vector<std::string> CalFloatToInt16(float value);
    float CalInt16ToFlaot(const std::vector<std::string> &data);
    std::vector<std::string> CalInt32ToInt16(uint32_t value);
    uint32_t CalInt16ToInt32(const std::vector<std::string> &data);
    std::vector<std::string> StrToASCII(const std::string &value);
    std::string ASCIIToStr(const std::vector<std::string> &data);
    std::vector<std::string> CalBytesToInt16(const std::vector<uint8_t> &value);
    std::vector<uint8_t> CalInt16ToBytes(const std::vector<std::string> &data);
    std::vector<std::string> splitString(const std::string& str, const std::string& symbol = " ");
    std::string GetDataType(DataType dataType);
    bool CheckError(const std::string& str);
public:
    RegErrorCode GetLastErrorCode() const override { return lastErrorCode_; }
    std::string GetLastErrorMessage() const override { return lastErrorMsg_; }
private:
    OnConnectionCB onConnectionCB_ = nullptr;
    common::tcp::TcpClient tcpClient_;
    bool connected_ = false;
    std::condition_variable cond_;
    std::mutex mtx_;
    FairLock lock_;
    std::string recvStr_ {""};
    bool recvFlag_ = false;
};
} // namespace kv
} // namespace common::regaccess

#endif