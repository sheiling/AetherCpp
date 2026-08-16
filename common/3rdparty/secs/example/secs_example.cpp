#include <iostream>
#include <thread>
#include <chrono>

#include "secs_client.h"
#include "createclientopts.h"

// 简单演示：创建 SECSClient，建立 HSMS 连接并发送一条 Linktest 请求
int main()
{
    SECS::CreateCliOpts opts;
    opts.serverIP_        = "127.0.0.1";
    opts.serverPort_      = 5000;          // 根据实际 HSMS 服务端端口修改
    opts.reconnSwitch_    = false;         // 示例中先关闭自动重连
    opts.reconnMinDelay_  = 3000;
    opts.reconnMaxDelay_  = 9000;
    opts.reconnDelayPolicy_ = 0;

    SECS::SECSClient client;

    // 注册连接状态回调（可选）
    client.RegisterConnectionCB([](void* userData, int connfd, int connID, bool isConnected) {
        std::cout << "[SECS] connection changed, fd=" << connfd
                  << " id=" << connID
                  << " connected=" << std::boolalpha << isConnected << std::endl;
    });

    // 注册数据消息处理回调（可选）
    client.RegisterMessageHandlerCB([](SECS::HSMSMessagePtr msg) {
        auto dataMsg = std::dynamic_pointer_cast<SECS::DataMessage>(msg);
        if (!dataMsg) {
            std::cout << "[SECS] received non-DataMessage." << std::endl;
            return;
        }
        std::cout << "[SECS] received DataMessage, item count = "
                  << dataMsg->getItemCount() << std::endl;
    });

    // 初始化并完成 HSMS Select 流程
    int ret = client.Init(opts);
    if (ret != 0) {
        std::cerr << "[SECS] Init failed, ret = " << ret << std::endl;
        return -1;
    }

    // 示例：发送一次 Linktest 请求，验证链路
    if (!client.linktestRequest()) {
        std::cerr << "[SECS] linktestRequest failed." << std::endl;
    } else {
        std::cout << "[SECS] linktestRequest success." << std::endl;
    }

    // 简单等待一会儿，观察可能的回包/回调输出
    std::this_thread::sleep_for(std::chrono::seconds(2));

    client.Release();
    return 0;
}

