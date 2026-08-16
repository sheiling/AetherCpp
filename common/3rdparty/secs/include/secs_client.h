#ifndef SECS_CLIENT_HH
#define SECS_CLIENT_HH

#include <functional>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <cstdint>

#include "tcpclient.h"
#include "reader.h"
#include "ast/hsms.h"
#include "ast/data_message.h"
#include "concurrent_map.h"
#include "sequence.h"
#include "secs_global.h"

namespace SECS
{

using OnConnectionCB = std::function<void(void*, int, int, bool)>;
using MessageHandlerCB = std::function<void(HSMSMessagePtr)>;
using MessageHandlerFunc = std::function<bool(const Message &)>;

class SECS_EXPORT SECSClient
{
public:
    SECSClient() = default;
    ~SECSClient() = default;
    
    int Init(CreateCliOpts &createCliOpts);
    void Release();
    bool sendMessage(HSMSMessagePtr request);
    bool sendRequest(const std::vector<uint8_t> &request, std::vector<uint8_t> &response);
    bool sendRequest(HSMSMessagePtr request, HSMSMessagePtr response);
    bool selectRequest();
    bool deselectRequest();
    bool linktestRequest();
    void RegisterMessageHandlerCB(MessageHandlerCB messageHandlerCB) {
        messageHandlerCB_ = messageHandlerCB;
    }
    void RegisterConnectionCB(OnConnectionCB onConnectionCB) {
        onConnectionCB_ = onConnectionCB;
    }
private:
    void OnConnection(void *userData, int connfd, int connID, bool isConnected);
    void OnMessage(void *userData, int connfd, int connID, const std::string &payload);
    bool sendMessage(HSMSMessagePtr request, Message &response, bool replyExpected = false);
    bool msgHandler();
    bool dataMessageHandler(const Message &msg);
    bool selectReply(const Message &msg);
    bool deselectReply(const Message &msg);
    bool linktestReply(const Message &msg);
    bool separateRequest();
    MessageHandlerFunc getMessageHandlerFunc(SType sType);
    bool reject(uint16_t sessionID, uint32_t systemBytes, uint8_t pType, uint8_t sType, uint8_t reasonCode);
    
private:
    OnConnectionCB onConnectionCB_ = nullptr;
    MessageHandlerCB messageHandlerCB_ = nullptr;
    TcpClient tcpClient_;
    int connected_ = 0; // 0-NOT CONNECTED, 1-CONNECTED(NOT SELECTED), 2-CONNECTED(SELECTED)
    Reader reader_;
    ConcurrentMap responseMap_;
    uint64_t T6_ = 5000;
    uint16_t sessionID_ = 65535;
    std::condition_variable cond_;
    std::mutex mtx_;
    std::map<SType, MessageHandlerFunc> msgHandlerFuncMap_;  
};

}

#endif