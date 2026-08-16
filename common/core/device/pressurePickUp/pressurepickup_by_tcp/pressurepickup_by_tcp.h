#ifndef _PRESSUREPICKUP_BY_TCP_HH
#define _PRESSUREPICKUP_BY_TCP_HH

#include "../pressurepickup_base.h"

namespace common
{
namespace device
{

class PRESSUREPICKUP_EXPORT PressurePickupByTcp : public PressurePickupBase {
public:
    PressurePickupByTcp() {}
    ~PressurePickupByTcp() = default;
public:
    virtual bool Init() override;
private:
};

}

}

#endif
