#ifndef _RANGEFINDER_BY_TCP_HH
#define _RANGEFINDER_BY_TCP_HH

#include "../rangefinder_base.h"
#include "../rangefinder_global.h"

namespace common
{
namespace device
{

class RANGEFINDER_EXPORT RangefinderByTCP : public RangefinderBase {
public:
    RangefinderByTCP() {}
    ~RangefinderByTCP() = default;
public:
    virtual bool Init() override;
private:
};

}

}

#endif
