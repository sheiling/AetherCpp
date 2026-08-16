#include "kv_client.h"
#include <iostream>
#include <cstring>

namespace common::regaccess {
namespace kv {
bool KVClient::Connect(const ConnectOptions &options)
{
    common::tcp::CreateCliOpts cliOpts;
    // 使用统一的 TCP 连接参数配置 TcpClient
    cliOpts.serverIP_        = options.host;
    cliOpts.serverPort_      = options.port;
    cliOpts.reconnSwitch_    = options.tcpReconnEnable;
    cliOpts.reconnMinDelay_  = options.tcpReconnMinDelay;
    cliOpts.reconnMaxDelay_  = options.tcpReconnMaxDelay;
    cliOpts.reconnDelayPolicy_ = options.tcpReconnPolicy;

    int ret = Init(cliOpts);
    if (ret != 0) {
        // Init 已经填充了 lastErrorCode_/lastErrorMsg_
        return false;
    }
    setError(RegErrorCode::Ok, "");
    return true;
}

KVClient::~KVClient()
{
    Release();
}

int KVClient::Init(common::tcp::CreateCliOpts &createCliOpts)
{
    createCliOpts.userdata_ = this;
    createCliOpts.onConnectionCB_ = std::bind(&KVClient::OnConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    createCliOpts.onMessageCB_ = std::bind(&KVClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    if (tcpClient_.Start(createCliOpts) != 0) {
        setError(RegErrorCode::TransportError, "keyence tcp client start failed");
        return -1;
    }
    
    std::unique_lock<std::mutex> lock(mtx_);
    if (!cond_.wait_for(lock, std::chrono::milliseconds(10000), [this]() { return connected_; })) {
        setError(RegErrorCode::Timeout, "keyence tcp connect timeout");
        return -1;
    }

    return 0;
}

void KVClient::Release()
{
    if (!connected_) return;
    // 断开连接
    tcpClient_.Stop();
    connected_ = false;
}

bool KVClient::WriteBit(const std::string &address, bool value)
{
    const char* cmd = value ? Command7 : Command8;
    std::string str = std::string(Command7) + SP + address + CR;
    return CheckError(SendRecive(str));
}

bool KVClient::ReadBit(const std::string &address, bool &value)
{
    // 读取位（bool）数据
    // std::string result = get(address, DataType::B);
    // if (!CheckError(result)) {
    //     return false;
    // }
    // // KV 协议通常返回 "1" 或 "0"
    // if (result == "1") {
    //     value = true;
    // } else if (result == "0") {
    //     value = false;
    // } else {
    //     setError(RegErrorCode::ProtocolError, "Unexpected result for ReadBit: " + result);
    //     return false;
    // }
    return false;
}

bool KVClient::WriteUInt16(const std::string &address, std::uint16_t value)
{
    return CheckError(put(address, std::to_string(value), DataType::U));
}

bool KVClient::ReadUInt16(const std::string &address, std::uint16_t &value)
{
    std::string result = get(address, DataType::U);
    if (!CheckError(result)) {
        return false;
    }
    value = std::stoi(result);
    return true;
}

bool KVClient::WriteUInt32(const std::string &address, std::uint32_t value)
{
    return CheckError(put(address, CalInt32ToInt16(value), DataType::D));
}

bool KVClient::ReadUInt32(const std::string &address, std::uint32_t &value)
{
    std::string result = get(address, 2, DataType::D);
    if (!CheckError(result)) {
        return false;
    }
    value = CalInt16ToInt32(splitString(result));
    return true;
}

bool KVClient::WriteFloat(const std::string &address, float value)
{
    return CheckError(put(address, CalFloatToInt16(value), DataType::N));
}

bool KVClient::ReadFloat(const std::string &address, float &value)
{
    std::string result = get(address, 2, DataType::N);
    if (!CheckError(result)) {
        return false;
    }
    value = CalInt16ToFlaot(splitString(result));
    return true;
}

bool KVClient::WriteRegisters(const std::string &address,
                        const std::vector<std::uint16_t> &values)
{
    // 写连续的16位无符号数到地址
    // Keyence KV 协议采用连续字写入接口（如put(address, vector<string>, DataType)）
    // 先把uint16_t数组转为字符串数组，协议类型为U（unsigned word）
    std::vector<std::string> strValues;
    strValues.reserve(values.size());
    for (const auto &v : values) {
        strValues.push_back(std::to_string(v));
    }
    // 通过put方法批量写入
    return CheckError(put(address, strValues, DataType::U));
}

bool KVClient::ReadRegisters(const std::string &address,
                       std::uint16_t regCnt,
                       std::vector<std::uint16_t> &values)
{
    // 读取 regCnt 个连续寄存器的 16 位无符号数
    values.clear();
    std::string result = get(address, regCnt, DataType::U);
    if (!CheckError(result)) {
        return false;
    }
    // 分割获得的字符串为每个寄存器的数值字符串
    std::vector<std::string> strValues = splitString(result);
    if (strValues.size() < regCnt) {
        setError(RegErrorCode::ProtocolError, "keyence ReadRegisters: response count mismatch, expected " + std::to_string(regCnt) + ", got " + std::to_string(strValues.size()));
        return false;
    }
    try {
        for (size_t i = 0; i < regCnt; ++i) {
            values.push_back(static_cast<std::uint16_t>(std::stoi(strValues[i])));
        }
    } catch (const std::exception& e) {
        setError(RegErrorCode::ProtocolError, std::string("keyence ReadRegisters value parse error: ") + e.what());
        return false;
    }
    return true;
}

bool KVClient::WriteString(const std::string &address,
                     const std::string &value,
                     std::uint16_t /*regNum*/)
{
    return CheckError(put(address, StrToASCII(value), DataType::N));
}

bool KVClient::ReadString(const std::string &address,
                    std::uint16_t regNum,
                    std::string &value)
{
    std::string result = get(address, regNum, DataType::N);
    if (!CheckError(result)) {
        return false;
    }
    value = ASCIIToStr(splitString(result));
    return true;
}

bool KVClient::WriteBytes(const std::string &address,
                    const std::vector<std::uint8_t> &value)
{
    return CheckError(put(address, CalBytesToInt16(value), DataType::N));
}

bool KVClient::ReadBytes(const std::string &address,
                   std::uint16_t regNum,
                   std::vector<std::uint8_t> &value)
{
    std::string result = get(address, regNum, DataType::N);
    if (!CheckError(result)) {
        return false;
    }
    value = CalInt16ToBytes(splitString(result));
    return true;
}

bool KVClient::ChangeCPU(bool run)
{
    std::string str = std::string(Command1) + (run ? "1" : "0") + CR;
    bool ok = CheckError(SendRecive(str));
    if (!ok) {
        setError(RegErrorCode::ProtocolError, "keyence ChangeCPU command failed");
    } else {
        setError(RegErrorCode::Ok, "");
    }
    return ok;
}

std::string KVClient::SeeCPUState()
{
    std::string str = std::string(Command5) + CR;
    auto rsp = SendRecive(str);
    if (!CheckError(rsp)) {
        setError(RegErrorCode::ProtocolError, "keyence SeeCPUState command failed");
    } else {
        setError(RegErrorCode::Ok, "");
    }
    return rsp;
}

std::string KVClient::put(const std::string &address, const std::string &value, DataType tpye)
{
    std::string str = std::string(Command14) + SP + address + GetDataType(tpye) + SP + value + CR;
    return SendRecive(str);
}

std::string KVClient::put(const std::string &address, const std::vector<std::string> &value, DataType tpye)
{
    std::string str = std::string(Command15) + SP + address + GetDataType(tpye) + SP + std::to_string(value.size());
    for (int i = 0; i < value.size(); i++)
    {
        str.append(SP + value[i]);
    }
    str.append(CR);
    return SendRecive(str);
}

std::string KVClient::get(const std::string &address, DataType tpye)
{
    std::string str = std::string(Command11) + SP + address + GetDataType(tpye) + CR;
    return SendRecive(str);
}

std::string KVClient::get(const std::string &address, int count, DataType tpye)
{
    std::string str = std::string(Command12) + SP + address + GetDataType(tpye) + SP + std::to_string(count) + CR;
    return SendRecive(str);
}

void KVClient::OnConnection(void *userData, int connfd, int connID, bool isConnected)
{
    if (isConnected) {
        fprintf(stdout, "Connfd=%d connID=%u, connected.\n", connfd, connID);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            connected_ = true;
        }
        cond_.notify_one();  
    } else {
        fprintf(stdout, "Connfd=%d connID=%u, disConnected.\n", connfd, connID);
        connected_ = false;
    }
    // 回调
    if (onConnectionCB_) {
        onConnectionCB_(userData, connfd, connID, isConnected);
    }
}

void KVClient::OnMessage(void *userData, int connfd, int connID, const std::string &payload)
{
    std::unique_lock<std::mutex> lck(mtx_);
    recvFlag_ = true;
    recvStr_ = payload;
    cond_.notify_one();
}

std::string KVClient::SendRecive(const std::string &str)
{
    FairLock::ScopedLock lock(lock_);
    if (!connected_) {
        setError(RegErrorCode::NotConnected, "keyence tcp not connected");
        return "";
    }

    tcpClient_.AsyncSend(str);
    // 等待回复
    std::unique_lock<std::mutex> lck(mtx_);
    if(!cond_.wait_for(lck, std::chrono::milliseconds(30000), [&] () { return recvFlag_; })) {
        recvFlag_ = false;
        setError(RegErrorCode::Timeout, "keyence request timeout");
        return "";
    }
    recvFlag_ = false;

    // KV 协议错误由 CheckError 统一判断，这里仅记录传输成功
    setError(RegErrorCode::Ok, "");
    return recvStr_;
}

std::vector<std::string> KVClient::CalFloatToInt16(float value)
{
    uint32_t floatAsInt = reinterpret_cast<uint32_t&>(value);
    uint16_t r1 = static_cast<uint16_t>(floatAsInt);
    uint16_t r2 = static_cast<uint16_t>(floatAsInt >> 16);
    std::vector<std::string> data = {std::to_string(r1), std::to_string(r2)};
    return data;
}

float KVClient::CalInt16ToFlaot(const std::vector<std::string> &data)
{
    uint16_t r1 = std::stoi(data[0]);
    uint16_t r2 = std::stoi(data[1]);
    uint32_t floatAsInt = 0;
    floatAsInt += r1;
    floatAsInt += (r2 << 16);
    float val = reinterpret_cast<float&>(floatAsInt);
    return val;
}

std::vector<std::string> KVClient::CalInt32ToInt16(uint32_t value)
{
    uint16_t r1 = static_cast<uint16_t>(value);
    uint16_t r2 = static_cast<uint16_t>(value >> 16);
    std::vector<std::string> data = {std::to_string(r1), std::to_string(r2)};
    return data;
}

uint32_t KVClient::CalInt16ToInt32(const std::vector<std::string> &data)
{
    uint16_t r1 = std::stoi(data[0]);
    uint16_t r2 = std::stoi(data[1]);
    uint32_t val = 0;
    val += r1;
    val += (r2 << 16);
    return val;
}

std::vector<std::string> KVClient::StrToASCII(const std::string &value)
{
    int byteSize = (value.size() % 2 == 0) ? value.size() : value.size() + 1;
    std::vector<uint8_t> temp(byteSize, 0); 
    for (int i = 0; i < value.size(); i++)
    {
        temp[i] = static_cast<uint8_t>(value[i]);
    }

    std::vector<std::string> data(byteSize / 2, ""); 
    for (int i = 0; i < byteSize / 2; i++)
    {
        uint16_t tempData = 0;
        tempData += (temp[i*2] << 8);
        tempData += temp[i*2 + 1];
        data[i] = std::to_string(tempData);
    }
    return data;
}

std::string KVClient::ASCIIToStr(const std::vector<std::string> &data)
{
    std::vector<uint8_t> temp(data.size() * 2, 0);
    for (size_t i = 0; i < data.size(); i++)
    {
        uint16_t r = std::stoi(data[i]);
        temp[i*2] = static_cast<uint8_t>(r >> 8);
        temp[i*2 + 1] = static_cast<uint8_t>(r);
    }
    
    std::string str(temp.begin(), temp.end());
    return str;
}

std::vector<std::string> KVClient::CalBytesToInt16(const std::vector<uint8_t> &value)
{
    int byteSize = (value.size() % 2 == 0) ? value.size() : value.size() + 1;
    std::vector<std::string> data(byteSize / 2, ""); 
    for (int i = 0; i < byteSize / 2; i++)
    {
        uint16_t tempData = 0;
        tempData += (value[i*2] << 8);
        tempData += value[i*2 + 1];
        data[i] = std::to_string(tempData);
    }

    return data;
}

std::vector<uint8_t> KVClient::CalInt16ToBytes(const std::vector<std::string> &data)
{
    std::vector<uint8_t> vec(data.size() * 2, 0);
    for (size_t i = 0; i < data.size(); i++)
    {
        uint16_t r = std::stoi(data[i]);
        vec[i*2] = static_cast<uint8_t>(r >> 8);
        vec[i*2 + 1] = static_cast<uint8_t>(r);
    }
    
    return vec;
}

std::vector<std::string> KVClient::splitString(const std::string &str, const std::string &symbol)
{
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(symbol);
    
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + symbol.size();
        end = str.find(symbol, start);
    }

    if (start < str.length()) { // 处理最后一个元素后面可能有分隔符的情况
        result.push_back(str.substr(start));
    }

    return result;
}

std::string KVClient::GetDataType(DataType dataType)
{
    std::string str;
    switch (dataType)
    {
    case DataType::U:
        str = ".U";
        break;
    case DataType::S:
        str = ".S";
        break;
    case DataType::D:
        str = ".D";
        break;
    case DataType::L:
        str = ".L";
        break;
    case DataType::H:
        str = ".H";
        break;
    case DataType::N:
        str = "";
        break;
    default:
        str = "";
        break;
    }
    return str;
}

bool KVClient::CheckError(const std::string &str)
{
    if (str.empty()) {
        setError(RegErrorCode::ProtocolError, "keyence empty response");
        return false;
    }
    if (str.substr(0, 1) == "E") {
        setError(RegErrorCode::ProtocolError, "keyence response error: " + str);
        return false;
    }
    // 正常响应不带前缀 E，清理为 Ok
    setError(RegErrorCode::Ok, "");
    return true;
}
} // namespace kv   
} // namespace common::regaccess