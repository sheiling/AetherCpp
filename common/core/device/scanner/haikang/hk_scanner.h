#ifndef _HK_SCANNER_HH
#define _HK_SCANNER_HH

#include "../scanner_base.h"
#include <common/core/comm/tcp/client/tcpclient.h>

namespace common
{
namespace device
{

class SCANNER_EXPORT HKScanner : public ScannerBase {
public:
    HKScanner() {
	}
    ~HKScanner() = default;
public:
    virtual bool Init() override;
    virtual bool Request(const std::string &reqData) override;
    virtual bool SyncRequest(const std::string& reqData, std::string& rspData)override;
private:
    void OnConnection(void *userData, int connfd, uint32_t connID, bool isConnected);
    void OnMessage(void *userData, int connfd, uint32_t connID, const std::string &payload);
private:
    std::unique_ptr<common::tcp::TcpClient> tcpClient_ = nullptr;
};

}

}

#endif
