#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CAPABILITIES_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CAPABILITIES_H

namespace common::device::motion {

struct MotionCapabilities {
	bool axis = false;
	bool crd = false;
	bool io = false;
	bool ad = false;

	static MotionCapabilities fullMotion() { return {true, true, true, true}; }
	static MotionCapabilities ioOnly() { return {false, false, true, false}; }
};

} // namespace common::device::motion

#endif
