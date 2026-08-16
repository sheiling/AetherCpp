#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_IAXIS_DRIVER_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_IAXIS_DRIVER_H

#include <cstdint>
#include <vector>

#include "../motion_types.h"

namespace common::device::motion {

class IAxisDriver {
public:
	virtual ~IAxisDriver() = default;

	virtual long initAxis(int axisNo) = 0;
	virtual long totalAxis(uint32_t& axisNum) = 0;
	virtual long axisSetVel(int axisNo, const AxisVelocityProfile& profile) = 0;
	virtual long crdSetVel(int crdNo, const AxisVelocityProfile& profile) = 0;
	virtual long axisSetSMode(int axisNo, int s_mode, double s_para) = 0;
	virtual long axisSingleMove(int axisNo, const AxisSingleMoveParams& params) = 0;
	virtual long axisVMove(int axisNo, uint16_t dir) = 0;
	virtual long axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode) = 0;
	virtual bool axisIsMoveDone(int axisNo) = 0;
	virtual bool crdIsMoveDone(int crdNo) = 0;
	virtual long axisStop(int axisNo, uint8_t mode) = 0;
	virtual long axisEmgStop() = 0;
	virtual long axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome) = 0;
	virtual long axisStartHome(int axisNo) = 0;
	virtual bool axisIsHomeDone(int axisNo) = 0;
	virtual long axisAlarmClear(int axisNo) = 0;
	virtual long axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus) = 0;
	virtual long axisGetEncoder(int axisNo, double& value) = 0;
	virtual long axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value) = 0;
	virtual long axisGetPosition(int axisNo, double& value) = 0;
	virtual long axisSetEncoderAndPosition(int axisNo, double value) = 0;
	virtual long axisCheckSuccessEncoder(int axisNo, double bandRange) = 0;
	virtual long axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit) = 0;
	virtual long axisSetEquiv(int axisNo, double equiv) = 0;
	virtual long axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic) = 0;
	virtual long axisSetFactorError(int axisNo, double factorError) = 0;
	virtual long axisEnable(int axisNo, bool enable) = 0;
	virtual long axisIsEnable(int axisNo, bool& enable) = 0;
	virtual long axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc) = 0;
	virtual long axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos) = 0;
	virtual long positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup) = 0;
	virtual long positionCompareDisable(int axisNo, int channelNo, bool highSpeed) = 0;
	virtual long writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value) = 0;
	virtual long readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value) = 0;
};

} // namespace common::device::motion

#endif
