#ifndef _TCP_CLIENT_HH
#define _TCP_CLIENT_HH

#include <memory>
#include <3rdparty/libhv/include/hv/TcpClient.h>
#include "createclientopts.h"
#include "../tcp_comm_global.h"

namespace common::tcp {

class TCP_COMM_EXPORT TcpClient {
public:
    TcpClient() = default;
    ~TcpClient() = default;
public:
    int Start(const CreateCliOpts& createCliOpts);
    int SyncSend(const std::string &payload);
    int AsyncSend(const std::string &payload);
    void Stop();
private:
    void *userdata_ = nullptr;          // 用户私有数据指针(临时存储,不负责资源释放,回调接口会带出给到应用层,方便应用层编码需要)
    OnConnectionFunc onConnectionCB_;   // 连接建立回调接口
    OnMessageFunc onMessageCB_;         // 消息接收回调借口
    OnDeliveredFunc onDeliveredCB_;     // 消息发送完成接口回调(用户空间而非内核空间)
private:
    hv::TcpClient *hvTcpClient_ = nullptr;
    reconn_setting_t *hvReconn_ = nullptr;
};

using TcpClientPtr = std::shared_ptr<TcpClient>;

} // end of namespace common::tcp

#endif
