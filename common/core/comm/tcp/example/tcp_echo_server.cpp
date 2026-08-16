#include <iostream>
#include "../server/tcpserver.h"

using namespace common::tcp;

int main() {
    CreateSvrOpts opts;
    opts.serverIP_ = "0.0.0.0";
    opts.serverPort_ = 9000;
    opts.threadCnt_ = 4;
    opts.loadBalance_ = 1; // 随机
    opts.userdata_ = nullptr;

    opts.onConnectionCB_ = [](void*, int connfd, uint32_t connID, bool isConnected) {
        std::cout << "[server] connfd=" << connfd
                  << " connID=" << connID
                  << (isConnected ? " connected" : " disconnected")
                  << std::endl;
    };

    TcpServer server;

    opts.onMessageCB_ = [&server](void*, int /*connfd*/, uint32_t connID, const std::string& payload) {
        std::cout << "[server] recv from " << connID << ": " << payload << std::endl;
        server.AsyncSend((int)connID, payload);
    };

    opts.onDeliveredCB_ = []() {
        std::cout << "[server] send complete" << std::endl;
    };

    int ret = server.Start(opts);
    if (ret != 0) {
        std::cerr << "Start TcpServer failed, ret=" << ret << std::endl;
        return ret;
    }

    std::cout << "TCP echo server started on 0.0.0.0:9000, press ENTER to quit..." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    server.Stop();
    return 0;
}

