#ifndef _CREATE_SERVER_OPTS_HH
#define _CREATE_SERVER_OPTS_HH

#include <string>
#include <functional>

namespace common::tcp {

// 连接建立完成回调接口定义
using OnConnectionFunc = std::function<void(void *userData, int connfd, uint32_t connID, bool isConnected)>;
// 消息接收回调接口定义
using OnMessageFunc = std::function<void(void *userData, int connfd, uint32_t connID, const std::string &payload)>;
// 用户空间(非内核空间)消息发送完成回电接口定义
using OnDeliveredFunc = std::function<void()>;

class CreateSvrOpts {
public:
    std::string serverIP_;  // 服务端IP地址
    int serverPort_;        // 服务端端口号
    int threadCnt_;         // IO线程数

    /*
     * @loadBalance_
     * 0: LB_RoundRobin
     * 1: LB_Random
     * 2: LB_LeastConnections
     * 3: LB_IpHash
     * 4: LB_UrlHash
     * other: LB_Random
     */
    int loadBalance_;       // 负载均衡

    void *userdata_;                    // 用户私有数据指针(临时存储,不负责资源释放,回调接口会带出给到应用层,方便应用层编码需要)
    OnConnectionFunc onConnectionCB_;   // 连接建立回调接口
    OnMessageFunc onMessageCB_;         // 消息接收回调借口
    OnDeliveredFunc onDeliveredCB_;     // 消息发送完成接口回调(用户空间而非内核空间)

    CreateSvrOpts() {
        serverIP_ = "127.0.0.1";
        serverPort_ = 0;
        threadCnt_ = 4;
        loadBalance_ = 1;
        userdata_ = nullptr;
        onConnectionCB_ = nullptr;
        onMessageCB_ = nullptr;
        onDeliveredCB_ = nullptr;
    }
};

} // end of namespace common::tcp

#endif
