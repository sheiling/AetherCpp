#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_GAOCHUAN_IO_MOTION_CARD_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_GAOCHUAN_IO_MOTION_CARD_H

#include "../../motion_card_base.h"

namespace common::device::motion {

/** 高川 EtherCAT IO（ecat_io_dll）。仅 IO，无运动轴。 */
class GaoChuanIoMotionCard : public MotionCardBase,
	public IMotionDeviceLifecycle,
	public IIoDriver {
public:
	GaoChuanIoMotionCard() = default;
	~GaoChuanIoMotionCard() override = default;

	IMotionDeviceLifecycle* lifecycleDriver() override { return this; }
	IIoDriver* ioDriver() override { return this; }
	MotionCapabilities capabilities() const override { return MotionCapabilities::ioOnly(); }

	long initDevice() override;
	void release() override;
	bool makeError(long rtn, const std::string& strFun, std::string& rtnStr) override;
	long writeIO(int bitNo, bool value) override;
	long readIO(int bitNo, bool& value, bool isInputIo = false) override;
	long readDoPorts(std::vector<uint32_t>& vecValue) override;
	long readDiPorts(std::vector<uint32_t>& vecValue) override;
	long writeExtendIO(int nodeNo, int bitNo, bool value) override;
	long readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo = false) override;

private:
	const std::string& nioDeviceId() const { return m_deviceConnectOpts_.m_connectString; }
};

} // namespace common::device::motion

#endif
