#include "hk_scanner.h"
#include <common/core/comm/tcp/client/tcpclient.h>
#include <Logger.h>

namespace common
{
namespace device
{

bool HKScanner::Init() {
    tcpClient_ = std::make_unique<common::tcp::TcpClient>();
    if (!tcpClient_) {
        return false;
    }
    common::tcp::CreateCliOpts createCliOpts;
    createCliOpts.serverIP_ = m_scannerDevInfo_.ip;
    createCliOpts.serverPort_ = m_scannerDevInfo_.port;
    createCliOpts.reconnSwitch_ = true;
    createCliOpts.reconnMinDelay_ = 3000;
    createCliOpts.reconnMaxDelay_ = 9000;
    createCliOpts.reconnDelayPolicy_ = 1;
    createCliOpts.userdata_ = nullptr;
    createCliOpts.onConnectionCB_ = std::bind(&HKScanner::OnConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    createCliOpts.onMessageCB_ = std::bind(&HKScanner::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    if (0 != tcpClient_->Start(createCliOpts)) {
        SetConnectedState(false);
        return false;
    }
    return true;
}


bool HKScanner::Request(const std::string &reqData) {
    int ret = tcpClient_->AsyncSend(reqData);
    return (ret < 0 ? false : true);
}
bool HKScanner::SyncRequest(const std::string& reqData, std::string& rspData){
    return true;
}

void HKScanner::OnConnection(void *userData, int connfd, uint32_t connID, bool isConnected) {
    if (isConnected) {
        LOG_INFO("[common][device][scanner]--Connfd={} connID={}, connected.", connfd, connID);
    } else {
        LOG_INFO("[common][device][scanner]--Connfd={} connID={}, disConnected.", connfd, connID);
    }
    SetConnectedState(isConnected);
}

void HKScanner::OnMessage(void *userData, int connfd, uint32_t connID, const std::string &payload) {
    if (scannerRspCBFunc_) {
        scannerRspCBFunc_(payload);
    }
}

}

}
