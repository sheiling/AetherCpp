#include "rangefinder_by_tcp.h"
#include <common/core/comm/regaccess/modbus/master/modbus_master.h>
#include <common/core/comm/regaccess/modbus/master/modbus_master_options.h>

namespace common
{
namespace device
{

bool RangefinderByTCP::Init() {
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::TCP;
    modbusMasterOpts.ipOrDevice_ = m_rangefinderDevInfo_.ip;
    modbusMasterOpts.port_ = m_rangefinderDevInfo_.port;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    int32_t iret = modbusMastes_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

}

}