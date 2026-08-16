#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_IIO_DRIVER_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_IIO_DRIVER_H

#include <cstdint>
#include <vector>

#include "../motion_types.h"

namespace common::device::motion {

class IIoDriver {
public:
	virtual ~IIoDriver() = default;

	virtual long writeIO(int bitNo, bool value) = 0;
	virtual long readIO(int bitNo, bool& value, bool isInputIo = false) = 0;
	virtual long readDoPorts(std::vector<uint32_t>& vecValue) { (void)vecValue; return kSdkNotSupported; }
	virtual long readDiPorts(std::vector<uint32_t>& vecValue) { (void)vecValue; return kSdkNotSupported; }
	virtual long writeExtendIO(int nodeNo, int bitNo, bool value) = 0;
	virtual long readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo = false) = 0;
};

} // namespace common::device::motion

#endif
