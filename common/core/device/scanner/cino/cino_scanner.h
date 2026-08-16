#ifndef _CINO_SCANNER_HH
#define _CINO_SCANNER_HH

#include "../scanner_base.h"
#include <common/core/comm/serial/serial_master.h>

namespace common
{
namespace device
{

class SCANNER_EXPORT CINOScanner : public ScannerBase {
public:
    CINOScanner() {
	}
    ~CINOScanner() = default;
public:
    virtual bool Init() override;
    virtual bool Request(const std::string &reqData) override;
    virtual bool SyncRequest(const std::string& reqData, std::string& rspData)override;
private:
    common::serial::SerialMaster serial_;
};

}

}

#endif
