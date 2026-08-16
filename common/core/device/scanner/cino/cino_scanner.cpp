#include "cino_scanner.h"
#include <common/core/comm/serial/serial_master_opts.h>

namespace common
{
namespace device
{

bool CINOScanner::Init() {
    common::serial::SerialInfo mserialOpts;
    mserialOpts.rspTimeout_ = m_scannerDevInfo_.serialInfo.rspTimeout_;
    mserialOpts.com_ = m_scannerDevInfo_.serialInfo.com_;
    mserialOpts.baud_ = m_scannerDevInfo_.serialInfo.baud_;
    mserialOpts.parity_ = (common::serial::ParityMode)m_scannerDevInfo_.serialInfo.parity_;
    mserialOpts.dataBit_ = m_scannerDevInfo_.serialInfo.dataBit_;
    mserialOpts.stopBit_ = m_scannerDevInfo_.serialInfo.stopBit_;

    int ret = serial_.init(mserialOpts);
    if (0 != ret) {
        SetConnectedState(false);
        return false;
    }

    SetConnectedState(true);
    return true;
}


bool CINOScanner::Request(const std::string &reqData) {
    return true;
}
bool CINOScanner::SyncRequest(const std::string& reqData, std::string& rspData) {
    int ret = 0;
    if (reqData == "start")
    {
        serial_.write("7E 80 02 00 04 00 01 01 86 7E");

        std::string rspFrame;
        ret = serial_.read(rspFrame, "\r\n", 2000);
        if (ret <= 0) {
            return false;
        }
        rspData = rspFrame;
        return true;
    }
    return false;
}

}
}
