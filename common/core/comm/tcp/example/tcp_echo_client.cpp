#include <iostream>
#include <thread>
#include <chrono>
#include "../client/tcpclient.h"

using namespace common::tcp;

int main() {
    CreateCliOpts opts;
    opts.serverIP_ = "127.0.0.1";
    opts.serverPort_ = 9000;
    opts.reconnSwitch_ = false;
    opts.userdata_ = nullptr;

    opts.onConnectionCB_ = [](void*, int connfd, uint32_t connID, bool isConnected) {
        std::cout << "[client] connfd=" << connfd
                  << " connID=" << connID
                  << (isConnected ? " connected" : " disconnected")
                  << std::endl;
    };

    opts.onMessageCB_ = [](void*, int /*connfd*/, uint32_t connID, const std::string& payload) {
        std::cout << "[client] recv from " << connID << ": " << payload << std::endl;
    };

    opts.onDeliveredCB_ = []() {
        std::cout << "[client] send complete" << std::endl;
    };

    TcpClient client;

    int ret = client.Start(opts);
    if (ret != 0) {
        std::cerr << "Start TcpClient failed, ret=" << ret << std::endl;
        return ret;
    }

    std::cout << "TCP echo client started, type message and ENTER to send, empty line to quit." << std::endl;

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            break;
        }

        client.AsyncSend(line);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    client.Stop();
    return 0;
}

