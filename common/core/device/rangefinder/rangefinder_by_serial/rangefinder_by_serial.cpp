#include "rangefinder_by_serial.h"
#include <common/core/comm/regaccess/modbus/master/modbus_master.h>
#include <common/core/comm/regaccess/modbus/master/modbus_master_options.h>

namespace common
{
namespace device
{

bool RangefinderBySerial::Init() {
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::RTU;
    modbusMasterOpts.ipOrDevice_ = "\\\\.\\" + m_rangefinderDevInfo_.serialInfo.com_;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    modbusMasterOpts.baud_ = m_rangefinderDevInfo_.serialInfo.baud_;
    if (m_rangefinderDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Even) {
        modbusMasterOpts.parity_ = 'E';
    } else if (m_rangefinderDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Odd) {
        modbusMasterOpts.parity_ = 'O';
    } else {
        modbusMasterOpts.parity_ = 'N';
    }
    modbusMasterOpts.dataBit_ = m_rangefinderDevInfo_.serialInfo.dataBit_;
    modbusMasterOpts.stopBit_ = m_rangefinderDevInfo_.serialInfo.stopBit_;
    int32_t iret = modbusMastes_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

int RangefinderBySerial::ChangeSlave(int slaveID) {
    return (modbusMastes_.ChangeSlave(slaveID));
}

}

}
