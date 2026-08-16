#ifndef _TCP_SERVER_HH
#define _TCP_SERVER_HH

#include <memory>
#include <3rdparty/libhv/include/hv/TcpServer.h>
#include "createserveropts.h"
#include "../tcp_comm_global.h"

namespace common::tcp {

class TCP_COMM_EXPORT TcpServer {
public:
    TcpServer() = default;
    ~TcpServer() = default;
public:
    int Start(const CreateSvrOpts& createSvrOpts);
    int SyncSend(int connID, const std::string &payload);
    int AsyncSend(int connID, const std::string &payload);
    int Broadcast(const std::string &payload);
    void CloseConn(int connID);
    void Stop();
private:
    void *userdata_ = nullptr;          // 用户私有数据指针(临时存储,不负责资源释放,回调接口会带出给到应用层,方便应用层编码需要)
    OnConnectionFunc onConnectionCB_;   // 连接建立回调接口
    OnMessageFunc onMessageCB_;         // 消息接收回调借口
    OnDeliveredFunc onDeliveredCB_;     // 消息发送完成接口回调(用户空间而非内核空间)
private:
    hv::TcpServer *hvTcpServer_ = nullptr;
};

using TcpServerPtr = std::shared_ptr<TcpServer>;

} // end of namespace common::tcp

#endif
