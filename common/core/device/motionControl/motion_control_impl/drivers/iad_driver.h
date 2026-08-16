#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_IAD_DRIVER_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_IAD_DRIVER_H

namespace common::device::motion {

class IAdDriver {
public:
	virtual ~IAdDriver() = default;

	virtual long writeAD(int nodeNo, int channelNo, double value) = 0;
	virtual long readAD(int nodeNo, int channelNo, double& value, bool isInputAd = false) = 0;
};

} // namespace common::device::motion

#endif
