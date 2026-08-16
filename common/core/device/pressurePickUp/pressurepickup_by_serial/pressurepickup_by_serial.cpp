#include "pressurepickup_by_serial.h"
namespace common
{
namespace device
{

bool PressurePickupBySerial::Init() {
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::RTU;
    modbusMasterOpts.ipOrDevice_ = "\\.\\" + m_pressurePickupDevInfo_.serialInfo.com_;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    modbusMasterOpts.baud_ = m_pressurePickupDevInfo_.serialInfo.baud_;
    if (m_pressurePickupDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Even) {
        modbusMasterOpts.parity_ = 'E';
    } else if (m_pressurePickupDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Odd) {
        modbusMasterOpts.parity_ = 'O';
    } else {
        modbusMasterOpts.parity_ = 'N';
    }
    modbusMasterOpts.dataBit_ = m_pressurePickupDevInfo_.serialInfo.dataBit_;
    modbusMasterOpts.stopBit_ = m_pressurePickupDevInfo_.serialInfo.stopBit_;
    int32_t iret = modbusMaster_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

int PressurePickupBySerial::ChangeSlave(int slaveID) {
    return (modbusMaster_.ChangeSlave(slaveID));
}

}

}
