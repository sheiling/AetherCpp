#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_LEADSHINE_NMC_MOTION_CARD_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_LEADSHINE_NMC_MOTION_CARD_H

#include "../../motion_card_base.h"

namespace common::device::motion {

enum class FrimType {
	Bac632e,
	Smc304,
};

class LeadshineNmcMotionCard : public MotionCardBase,
	public IMotionDeviceLifecycle,
	public IAxisDriver,
	public IIoDriver,
	public IAdDriver {
public:
	LeadshineNmcMotionCard() = default;
	~LeadshineNmcMotionCard() override = default;

	IMotionDeviceLifecycle* lifecycleDriver() override { return this; }
	IAxisDriver* axisDriver() override { return this; }
	IIoDriver* ioDriver() override { return this; }
	IAdDriver* adDriver() override { return this; }
	MotionCapabilities capabilities() const override { return MotionCapabilities::fullMotion(); }

	long initDevice() override;
	void release() override;
	long initAxis(int axisNo) override;
	long totalAxis(uint32_t& axisNum) override;
	long axisSetVel(int axisNo, const AxisVelocityProfile& profile) override;
	long crdSetVel(int crdNo, const AxisVelocityProfile& profile) override;
	long axisSetSMode(int axisNo, int s_mode, double s_para) override;
	long axisSingleMove(int axisNo, const AxisSingleMoveParams& params) override;
	long axisVMove(int axisNo, uint16_t dir) override;
	long axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode) override;
	bool axisIsMoveDone(int axisNo) override;
	bool crdIsMoveDone(int crdNo) override;
	long axisStop(int axisNo, uint8_t mode = 1) override;
	long axisEmgStop() override;
	long axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome) override;
	long axisStartHome(int axisNo) override;
	bool axisIsHomeDone(int axisNo) override;
	long axisAlarmClear(int axisNo) override;
	long axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus) override;
	long axisGetEncoder(int axisNo, double& value) override;
	long axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value) override;
	long axisGetPosition(int axisNo, double& value) override;
	long axisSetEncoderAndPosition(int axisNo, double value) override;
	long axisCheckSuccessEncoder(int axisNo, double bandRange) override;
	long axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit) override;
	long axisSetEquiv(int axisNo, double equiv) override;
	long axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic) override;
	long axisSetFactorError(int axisNo, double factorError) override;
	long axisEnable(int axisNo, bool enable) override;
	long axisIsEnable(int axisNo, bool& enable) override;
	long axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc) override;
	long axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos) override;
	long positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup) override;
	long positionCompareDisable(int axisNo, int channelNo, bool highSpeed) override;
	long writeIO(int bitNo, bool value) override;
	long readIO(int bitNo, bool& value, bool isInputIo = false) override;
	long writeExtendIO(int nodeNo, int bitNo, bool value) override;
	long readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo = false) override;
	long writeAD(int nodeNo, int channelNo, double value) override;
	long readAD(int nodeNo, int channelNo, double& value, bool isInputAd = false) override;
	long writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value) override;
	long readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value) override;
	bool makeError(long rtn, const std::string& strFun, std::string& rtnStr) override;

private:
	std::string makeBusError(long rtn);
	FrimType m_frimType = FrimType::Bac632e;
};

} // namespace common::device::motion

#endif
