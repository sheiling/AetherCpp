#include "pressurepickup_by_tcp.h"
namespace common
{
namespace device
{

bool PressurePickupByTcp::Init() {
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::TCP;
    modbusMasterOpts.ipOrDevice_ = m_pressurePickupDevInfo_.ip;
    modbusMasterOpts.port_ = m_pressurePickupDevInfo_.port;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    int32_t iret = modbusMaster_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

}
}
