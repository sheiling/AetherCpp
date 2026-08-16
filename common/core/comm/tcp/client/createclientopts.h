#ifndef _CREATE_CLIENT_OPTS_HH
#define _CREATE_CLIENT_OPTS_HH

#include <string>
#include <functional>

namespace common::tcp {

// 连接建立完成回调接口定义
using OnConnectionFunc = std::function<void(void *userData, int connfd, uint32_t connID, bool isConnected)>;
// 消息接收回调接口定义
using OnMessageFunc = std::function<void(void *userData, int connfd, uint32_t connID, const std::string &payload)>;
// 用户空间(非内核空间)消息发送完成回电接口定义
using OnDeliveredFunc = std::function<void()>;

class CreateCliOpts {
public:
    int connCount_;         // 连接数
    std::string serverIP_;  // 服务端IP地址
    int serverPort_;        // 服务端端口号
    bool reconnSwitch_;     // 重连开关
    int reconnMinDelay_;    // 最小重连时间间隔(配合重连策略使用)：毫秒
    int reconnMaxDelay_;    // 最大重连时间间隔(配合重连策略使用)：毫秒
    /*
     * @reconnDelayPolicy_
     * 0: fixed
     * min_delay=3s => 3,3,3...
     * 1: linear
     * min_delay=3s max_delay=10s => 3,6,9,10,10...
     * other: exponential
     * min_delay=3s max_delay=60s delay_policy=2 => 3,6,12,24,48,60,60...
     */
    int reconnDelayPolicy_; // 重连延迟策略(重连策略只会在服务端异常时才会生效,正常闪断发生时只会按最小时间间隔进行重连尝试)
    
    void *userdata_;                    // 用户私有数据指针(临时存储,不负责资源释放,回调接口会带出给到应用层,方便应用层编码需要)
    OnConnectionFunc onConnectionCB_;   // 连接建立回调接口
    OnMessageFunc onMessageCB_;         // 消息接收回调借口
    OnDeliveredFunc onDeliveredCB_;     // 消息发送完成接口回调(用户空间而非内核空间)

    CreateCliOpts() {
        connCount_ = 1;
        serverIP_ = "127.0.0.1";
        serverPort_ = 0;
        reconnSwitch_ = true;
        reconnMinDelay_ = 3000;
        reconnMaxDelay_ = 9000;
        reconnDelayPolicy_ = 0;

        userdata_ = nullptr;
        onConnectionCB_ = nullptr;
        onMessageCB_ = nullptr;
        onDeliveredCB_ = nullptr;
    }
};

} // end of namespace common::tcp

#endif
