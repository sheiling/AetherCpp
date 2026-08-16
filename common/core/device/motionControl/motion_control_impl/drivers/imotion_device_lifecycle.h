#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_IMOTION_DEVICE_LIFECYCLE_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_IMOTION_DEVICE_LIFECYCLE_H

#include <string>

namespace common::device::motion {

class IMotionDeviceLifecycle {
public:
	virtual ~IMotionDeviceLifecycle() = default;

	virtual long initDevice() = 0;
	virtual void release() = 0;
	virtual bool makeError(long rtn, const std::string& strFun, std::string& rtnStr) = 0;
};

} // namespace common::device::motion

#endif
