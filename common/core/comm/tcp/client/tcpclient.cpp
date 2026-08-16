#include "tcpclient.h"

namespace common::tcp {

int TcpClient::Start(const CreateCliOpts& createCliOpts) {
    // 防止重复 Start 导致资源泄露
    if (hvTcpClient_ != nullptr) {
        Stop();
    }

    // 1.构建参数校验
    if (createCliOpts.serverIP_.empty() || createCliOpts.serverPort_ == 0
    || createCliOpts.reconnMinDelay_ == 0 || createCliOpts.reconnMaxDelay_ == 0
    || createCliOpts.onConnectionCB_ == nullptr || createCliOpts.onMessageCB_ == nullptr) {
        return -1;
    }

    if (createCliOpts.reconnMinDelay_ > createCliOpts.reconnMaxDelay_) {
        return -1;
    }

    this->userdata_ = createCliOpts.userdata_;
    this->onConnectionCB_ = createCliOpts.onConnectionCB_;
    this->onMessageCB_ = createCliOpts.onMessageCB_;
    this->onDeliveredCB_ = createCliOpts.onDeliveredCB_;

    // 2.创建hv tcp客户端句柄和连接socket句柄
    hvTcpClient_ = new hv::TcpClient;
	int connfd = hvTcpClient_->createsocket(createCliOpts.serverPort_, createCliOpts.serverIP_.c_str());
    if (connfd < 0) {
        return -1;
    }

    // 3.设置相关回调接口
    hvTcpClient_->onConnection = [this](const hv::SocketChannelPtr& channel) {
        onConnectionCB_(userdata_, channel->fd(), channel->id(), channel->isConnected());
    };

    hvTcpClient_->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        std::string payload((const char*)buf->data(), buf->size());
        onMessageCB_(userdata_, channel->fd(), channel->id(), payload);
    };

    hvTcpClient_->onWriteComplete = [this](const hv::SocketChannelPtr& /*channel*/, hv::Buffer* /*buf*/) {
        if (onDeliveredCB_) {
            onDeliveredCB_();
        }
    };

    // 3.判断是否需要自动重连机制
    if (createCliOpts.reconnSwitch_) {
        hvReconn_ = new reconn_setting_t;
        reconn_setting_init(hvReconn_);
        hvReconn_->min_delay = createCliOpts.reconnMinDelay_;
        hvReconn_->max_delay = createCliOpts.reconnMaxDelay_;
        hvReconn_->delay_policy = createCliOpts.reconnDelayPolicy_;
        hvTcpClient_->setReconnect(hvReconn_);
    }

    // 4.启动 hv tcp客户端
    hvTcpClient_->start();

    return 0;
}

int TcpClient::SyncSend(const std::string &payload) {


    return 0;
}

int TcpClient::AsyncSend(const std::string &payload) {
    // (注意：libhv特性，发送空数据包会导致连接的断开)
    if (payload.length() == 0) {
        return 0;
    }

    if (hvTcpClient_ == nullptr) {
        return -1;
    }

    return (hvTcpClient_->send(payload));
}

void TcpClient::Stop() {
    if (hvTcpClient_) {
        hvTcpClient_->stop();
        delete hvTcpClient_;
        hvTcpClient_ = nullptr;
    }

    if (hvReconn_) {
        delete hvReconn_;
        hvReconn_ = nullptr;
    }
}

} // end of namespace common::tcp
