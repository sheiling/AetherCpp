#ifndef _PRESSUREPICKUP_BY_SERIAL_HH
#define _PRESSUREPICKUP_BY_SERIAL_HH

#include "../pressurepickup_base.h"

namespace common
{
namespace device
{

class PRESSUREPICKUP_EXPORT PressurePickupBySerial : public PressurePickupBase {
public:
    PressurePickupBySerial() {}
    ~PressurePickupBySerial() = default;
public:
    virtual bool Init() override;
    virtual int ChangeSlave(int slaveID) override;
private:
};

}

}

#endif
