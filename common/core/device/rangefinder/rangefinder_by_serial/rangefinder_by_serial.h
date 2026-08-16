#ifndef _RANGEFINDER_BY_SERIAL_HH
#define _RANGEFINDER_BY_SERIAL_HH

#include "../rangefinder_base.h"
#include "../rangefinder_global.h"

namespace common
{
namespace device
{

class RANGEFINDER_EXPORT RangefinderBySerial : public RangefinderBase {
public:
    RangefinderBySerial() {
	}
    ~RangefinderBySerial() = default;
public:
    virtual bool Init() override;
    virtual int ChangeSlave(int slaveID) override;
private:
};

}

}

#endif
