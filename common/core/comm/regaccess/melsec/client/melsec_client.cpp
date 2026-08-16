#include "melsec_client.h"
#include <iostream>
#include "../utils/Buffer.h"

namespace common::regaccess {
namespace mc {
    bool MelsecClient::Connect(const ConnectOptions &options)
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
            // Init 内部已经设置了详细错误码与描述
            return false;
        }
        setError(RegErrorCode::Ok, "");
        return true;
    }

    MelsecClient::~MelsecClient()
    {
        Release();
    }

    int MelsecClient::Init(common::tcp::CreateCliOpts &createCliOpts)
    {
        createCliOpts.userdata_ = this;
        createCliOpts.onConnectionCB_ = std::bind(&MelsecClient::OnConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        createCliOpts.onMessageCB_ = std::bind(&MelsecClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        if (tcpClient_.Start(createCliOpts) != 0) {
            setError(RegErrorCode::TransportError, "melsec tcp client start failed");
            return -1;
        }
        
        std::unique_lock<std::mutex> lock(mtx_);
        if (!cond_.wait_for(lock, std::chrono::milliseconds(10000), [this]() { return isConnected_; })) {
            setError(RegErrorCode::Timeout, "melsec tcp connect timeout");
            return -1;
        }

        port_ = createCliOpts.serverPort_;
        return 0;
    }

    void MelsecClient::Release()
    {
        if (!isConnected_) return;
        // 断开连接
        tcpClient_.Stop();
        isConnected_ = false;
    }

    // IRegAccess 实现
    namespace {
        // 解析三菱MC软元件ASCII地址（如"D100"、"M0"、"X1A"、"ZR1234"等），返回类型与数值
        inline bool parseAddress(const std::string &addr, MelsecAddressType &outType, uint32_t &outAddr)
        {
            size_t n = addr.size();
            if (n == 0) return false;

            // 首先查找所有支持的前缀，从长到短
            static const std::vector<std::string> prefixList = {
                "ZR", "TN", "TC", "TS", "CN", "CS", "CC", "SM", "SD", // 2字母前缀
                "D", "M", "X", "Y", "L", "F", "V", "B", "R", "S", "Z", "W" // 1字母前缀
            };
            for (const auto& prefix : prefixList) {
                if (addr.size() >= prefix.size() && 
                    strncmp(addr.c_str(), prefix.c_str(), prefix.size()) == 0) 
                {
                    MelsecAddressType type;
                    if (!MelsecAddressTypeFromAscii(prefix, type))
                        continue;

                    std::string tail = addr.substr(prefix.size());
                    if (tail.empty())
                        return false;

                    // 一些软元件序号支持16进制（如X/Y），部分支持十进制
                    uint32_t val = 0;
                    try {
                        if (prefix == "X" || prefix == "Y" || prefix == "B" || prefix == "W") {
                            val = static_cast<uint32_t>(std::stoul(tail, nullptr, 16)); // 16进制
                        } else {
                            val = static_cast<uint32_t>(std::stoul(tail, nullptr, 10));
                        }
                    } catch (...) {
                        return false;
                    }
                    outType = type;
                    outAddr = val;
                    return true;
                }
            }
            return false;
        }
    }

    bool MelsecClient::WriteBit(const std::string &address, bool value)
    {
        // TODO
        return true;
    }

    bool MelsecClient::ReadBit(const std::string &address, bool &value)
    {
        // TODO
        return true;
    }

    bool MelsecClient::WriteUInt16(const std::string &address, std::uint16_t value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        Buffer buffer;
        buffer << value;
        MelsecRequest request(addr, 1);
        request.setData(buffer.data());
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadUInt16(const std::string &address, std::uint16_t &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, 1);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        }

        Buffer buffer(response.getData());
        buffer >> value;
        return true;
    }

    bool MelsecClient::WriteUInt32(const std::string &address, std::uint32_t value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        Buffer buffer;
        buffer << value;
        MelsecRequest request(addr, 2);
        request.setData(buffer.data());
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadUInt32(const std::string &address, std::uint32_t &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, 2);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        } 

        Buffer buffer(response.getData());
        buffer >> value;
        return true;
    }

    bool MelsecClient::WriteFloat(const std::string &address, float value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        Buffer buffer;
        buffer << value;
        MelsecRequest request(addr, 2);
        request.setData(buffer.data());
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadFloat(const std::string &address, float &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, 2);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        } 

        Buffer buffer(response.getData());
        buffer >> value;
        return true;
    }

    bool MelsecClient::WriteRegisters(const std::string &address,
                                      const std::vector<std::uint16_t> &values)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }

        // 打包values到字节流
        std::vector<uint8_t> output;
        output.reserve(values.size() * 2);
        for (auto v : values) {
            output.push_back(static_cast<uint8_t>(v & 0xFF));         // 低字节
            output.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));  // 高字节
        }

        MelsecRequest request(addr, values.size());
        request.setData(output);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadRegisters(const std::string &address,
                                     std::uint16_t regCnt,
                                     std::vector<std::uint16_t> &values)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, regCnt);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        }

        const std::vector<uint8_t> &data = response.getData();
        values.clear();
        size_t bytesCount = regCnt * 2;
        if (data.size() < bytesCount) {
            setError(RegErrorCode::ProtocolError, "ReadRegisters: received data size less than expected.");
            return false;
        }
        values.reserve(regCnt);
        // MELSEC（如 FX3U/FX5U）ASCII/BIN 通常寄存器字（D/X/Y/M等）模式为小端（即低字节在前）
        for (size_t i = 0; i < regCnt; ++i) {
            uint16_t val = static_cast<uint16_t>(data[i * 2]) | (static_cast<uint16_t>(data[i * 2 + 1]) << 8);
            values.push_back(val);
        }

        return true;
    }

    bool MelsecClient::WriteString(const std::string &address,
                                   const std::string &value,
                                   std::uint16_t regNum)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }

        std::vector<uint8_t> output(regNum*2, 0);
        for (int i = 0; i < value.size(); i++)
        {
            output[i] = static_cast<uint8_t>(value[i]);
        }
        MelsecRequest request(addr, regNum);
        request.setData(output);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadString(const std::string &address,
                                  std::uint16_t regNum,
                                  std::string &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, regNum);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        } 

        std::vector<uint8_t> res =  response.getData();
        std::vector<uint8_t> dataValues;
        dataValues.reserve(regNum * 2);

        for (int i = 0; i < res.size(); i++)
        {
            if (i % 2 == 1) {
                dataValues.push_back(res[i]);
                dataValues.push_back(res[i-1]);
            }
        }
        
        std::string str(dataValues.begin(), dataValues.end());
        str.erase(std::remove(str.begin(), str.end(), '\0'), str.end());
        value = str;
        return true;
    }

    bool MelsecClient::WriteBytes(const std::string &address,
                                  const std::vector<std::uint8_t> &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        if (value.size() % 2 != 0) {
            return false;
        }
        uint16_t reqNum = value.size() / 2;
        MelsecRequest request(addr, reqNum);
        request.setData(value);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        return sendAndReceive(request, response);
    }

    bool MelsecClient::ReadBytes(const std::string &address,
                                 std::uint16_t regNum,
                                 std::vector<std::uint8_t> &value)
    {
        MelsecAddressType addressType = MelsecAddressType::D;
        uint32_t addr = 0;
        if (!parseAddress(address, addressType, addr)) {
            setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
            return false;
        }
        MelsecRequest request(addr, regNum);
        request.setAddressType(static_cast<uint8_t>(addressType));
        MelsecResponse response;
        bool bret = sendAndReceive(request, response);
        if (!bret) {
            return false;
        } 

        value =  response.getData();
        return true;
    }

    bool MelsecClient::isConnected()
    {
        return isConnected_;
    }

    void MelsecClient::refreshAliveTime()
    {
        alivetime_ = std::chrono::steady_clock::now();
    }

    uint64_t MelsecClient::getAliveTime()
    {
        std::chrono::nanoseconds res = std::chrono::steady_clock::now() - alivetime_;
        std::chrono::milliseconds millisec = std::chrono::duration_cast<std::chrono::milliseconds>(res);
        return millisec.count();
    }

    int MelsecClient::port()
    {
        return port_;
    }

    void MelsecClient::OnConnection(void *userData, int connfd, int connID, bool isConnected)
    {
        if (isConnected) {
            fprintf(stdout, "Connfd=%d connID=%u, connected.\n", connfd, connID);
            {
                std::lock_guard<std::mutex> lock(mtx_);
                isConnected_ = true;
            }
            cond_.notify_one();  
        } else {
            fprintf(stdout, "Connfd=%d connID=%u, disConnected.\n", connfd, connID);
            isConnected_ = false;
        }

        if (onConnectionCB_) {
            onConnectionCB_(userData, connfd, connID, isConnected);
        }
    }

    void MelsecClient::OnMessage(void *userData, int connfd, int connID, const std::string &payload)
    {
        std::unique_lock<std::mutex> lck(mtx_);
        recvBuf_.assign(reinterpret_cast<const uint8_t*>(payload.data()),
                        reinterpret_cast<const uint8_t*>(payload.data()) + payload.size());
        cond_.notify_one();
    }

    bool MelsecClient::sendAndReceive(const MelsecRequest &request, MelsecResponse &response)
    {
        FairLock::ScopedLock lock(lock_);
        if (!isConnected_) {
            setError(RegErrorCode::NotConnected, "melsec tcp not connected");
            return false;
        }

        std::string msg;
        auto &packed = request.pack();
        msg.append(reinterpret_cast<const char*>(packed.data()), packed.size());
        tcpClient_.AsyncSend(msg);

        // 等待回复
        std::unique_lock<std::mutex> lck(mtx_);
        if (std::cv_status::timeout == cond_.wait_for(lck, std::chrono::microseconds(5000000)))
        {
            setError(RegErrorCode::Timeout, "melsec request timeout");
            return false;
        }

        response.expand(recvBuf_);
        // 判断是否为响应内容
        if (response.getDeputyHead() != 0xD0) {
            setError(RegErrorCode::ProtocolError, "melsec invalid deputy head");
            return false;
        }
        // 判断结束代码是否为正常响应
        if (response.getErrorCode() != 0) {
            setError(RegErrorCode::ProtocolError, "melsec response error code != 0");
            return false;
        }

        setError(RegErrorCode::Ok, "");
        return true;
    }
} // end of namespace mc
} // end of namespace common::regaccess