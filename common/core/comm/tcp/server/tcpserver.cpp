#include "tcpserver.h"

namespace common::tcp {

int TcpServer::Start(const CreateSvrOpts& createSvrOpts) {
    // 防止重复 Start 导致资源泄露
    if (hvTcpServer_ != nullptr) {
        Stop();
    }

    // 1.构建参数校验
    if (createSvrOpts.serverIP_.empty() || createSvrOpts.serverPort_ == 0
    || createSvrOpts.onConnectionCB_ == nullptr || createSvrOpts.onMessageCB_ == nullptr) {
        return -1;
    }

    this->userdata_ = createSvrOpts.userdata_;
    this->onConnectionCB_ = createSvrOpts.onConnectionCB_;
    this->onMessageCB_ = createSvrOpts.onMessageCB_;
    this->onDeliveredCB_ = createSvrOpts.onDeliveredCB_;

    // 2.创建hv tcp服务端句柄和监听socket句柄
    hvTcpServer_ = new hv::TcpServer;
    int sockfd = hvTcpServer_->createsocket(createSvrOpts.serverPort_, createSvrOpts.serverIP_.c_str());
    if (sockfd < 0) {
        return -1;
    }

    // 3.设置相关回调接口
    hvTcpServer_->onConnection = [this](const hv::SocketChannelPtr& channel) {
        onConnectionCB_(userdata_, channel->fd(), channel->id(), channel->isConnected());
    };
    
    hvTcpServer_->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        std::string payload((const char*)buf->data(), buf->size());
		onMessageCB_(userdata_, channel->fd(), channel->id(), payload);
    };

    hvTcpServer_->onWriteComplete = [this](const hv::SocketChannelPtr& /*channel*/, hv::Buffer* /*buf*/) {
        if (onDeliveredCB_) {
            onDeliveredCB_();
        }
    };

    // 设置启动IO线程数
    hvTcpServer_->setThreadNum(createSvrOpts.threadCnt_);

    // 根据配置设置负载均衡策略，非法值回退为随机
    load_balance_e lb = LB_Random;
    switch (createSvrOpts.loadBalance_) {
    case 0: lb = LB_RoundRobin; break;
    case 1: lb = LB_Random; break;
    case 2: lb = LB_LeastConnections; break;
    case 3: lb = LB_IpHash; break;
    case 4: lb = LB_UrlHash; break;
    default: lb = LB_Random; break;
    }
    hvTcpServer_->setLoadBalance(lb);

    // 启动 hv tcp服务端
    hvTcpServer_->start();

    return 0;
}

int TcpServer::SyncSend(int connID, const std::string &payload) {

    if (payload.length() == 0) {
        return 0;
    }

    if (hvTcpServer_ == nullptr) {
        return -1;
    }

    auto channel = hvTcpServer_->getChannelById(connID);
    if (!channel) {
        return -1;
    }

    // libhv 的 write 本身是同步的，这里仅做语义上的区分
    return channel->write(payload.data(), (int)payload.size());
}

int TcpServer::AsyncSend(int connID, const std::string &payload) {
    if (payload.length() == 0) {
		return 0;
    }

    if (hvTcpServer_ == nullptr) {
        return -1;
    }

    auto channel = hvTcpServer_->getChannelById(connID);
    if (!channel) {
        return -1;
    }

	return channel->write(payload.data(), (int)payload.size());
}

int TcpServer::Broadcast(const std::string &payload) {
	if (payload.length() == 0) {
		return 0;
    }

    return (hvTcpServer_->broadcast(payload));
}

void TcpServer::CloseConn(int connID)
{
    if (!hvTcpServer_) {
        return;
    }

    auto channel = hvTcpServer_->getChannelById(connID);
    if (channel) {
        channel->close();
    }
}

void TcpServer::Stop() {
    if (hvTcpServer_) {
        hvTcpServer_->stop();
        delete hvTcpServer_;
        hvTcpServer_ = nullptr;
    }
}

} // end of namespace common::tcp
