#include "motion_card_base.h"

namespace common::device::motion {

long MotionCardBase::initDevice()
{
	IMotionDeviceLifecycle* lc = lifecycleDriver();
	return lc ? lc->initDevice() : kSdkNotSupported;
}

void MotionCardBase::release()
{
	IMotionDeviceLifecycle* lc = lifecycleDriver();
	if (lc) {
		lc->release();
	}
}

bool MotionCardBase::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	IMotionDeviceLifecycle* lc = lifecycleDriver();
	return lc ? lc->makeError(rtn, strFun, rtnStr) : false;
}

long MotionCardBase::initAxis(int axisNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->initAxis(axisNo) : kSdkNotSupported;
}

long MotionCardBase::totalAxis(uint32_t& axisNum)
{
	IAxisDriver* d = axisDriver();
	return d ? d->totalAxis(axisNum) : kSdkNotSupported;
}

long MotionCardBase::axisSetVel(int axisNo, const AxisVelocityProfile& profile)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetVel(axisNo, profile) : kSdkNotSupported;
}

long MotionCardBase::crdSetVel(int crdNo, const AxisVelocityProfile& profile)
{
	IAxisDriver* d = axisDriver();
	return d ? d->crdSetVel(crdNo, profile) : kSdkNotSupported;
}

long MotionCardBase::axisSetSMode(int axisNo, int s_mode, double s_para)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetSMode(axisNo, s_mode, s_para) : kSdkNotSupported;
}

long MotionCardBase::axisSingleMove(int axisNo, const AxisSingleMoveParams& params)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSingleMove(axisNo, params) : kSdkNotSupported;
}

long MotionCardBase::axisVMove(int axisNo, uint16_t dir)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisVMove(axisNo, dir) : kSdkNotSupported;
}

long MotionCardBase::axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisLinear(crdNo, axisNos, dists, posiMode) : kSdkNotSupported;
}

bool MotionCardBase::axisIsMoveDone(int axisNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisIsMoveDone(axisNo) : true;
}

bool MotionCardBase::crdIsMoveDone(int crdNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->crdIsMoveDone(crdNo) : true;
}

long MotionCardBase::axisStop(int axisNo, uint8_t mode)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisStop(axisNo, mode) : kSdkNotSupported;
}

long MotionCardBase::axisEmgStop()
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisEmgStop() : kSdkNotSupported;
}

long MotionCardBase::axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetHomeParam(axisNo, axisHome) : kSdkNotSupported;
}

long MotionCardBase::axisStartHome(int axisNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisStartHome(axisNo) : kSdkNotSupported;
}

bool MotionCardBase::axisIsHomeDone(int axisNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisIsHomeDone(axisNo) : true;
}

long MotionCardBase::axisAlarmClear(int axisNo)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisAlarmClear(axisNo) : kSdkNotSupported;
}

long MotionCardBase::axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisReadStatus(axisNo, axisStatus) : kSdkNotSupported;
}

long MotionCardBase::axisGetEncoder(int axisNo, double& value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisGetEncoder(axisNo, value) : kSdkNotSupported;
}

long MotionCardBase::axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisGetAbsEncoder(axisNo, slaveId, driverType, encoderType, value) : kSdkNotSupported;
}

long MotionCardBase::axisGetPosition(int axisNo, double& value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisGetPosition(axisNo, value) : kSdkNotSupported;
}

long MotionCardBase::axisSetEncoderAndPosition(int axisNo, double value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetEncoderAndPosition(axisNo, value) : kSdkNotSupported;
}

long MotionCardBase::axisCheckSuccessEncoder(int axisNo, double bandRange)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisCheckSuccessEncoder(axisNo, bandRange) : kSdkNotSupported;
}

long MotionCardBase::axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetSoftlimit(axisNo, axisSoftlimit) : kSdkNotSupported;
}

long MotionCardBase::axisSetEquiv(int axisNo, double equiv)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetEquiv(axisNo, equiv) : kSdkNotSupported;
}

long MotionCardBase::axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetStaLogic(axisNo, axisStaLogic) : kSdkNotSupported;
}

long MotionCardBase::axisSetFactorError(int axisNo, double factorError)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisSetFactorError(axisNo, factorError) : kSdkNotSupported;
}

long MotionCardBase::axisEnable(int axisNo, bool enable)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisEnable(axisNo, enable) : kSdkNotSupported;
}

long MotionCardBase::axisIsEnable(int axisNo, bool& enable)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisIsEnable(axisNo, enable) : kSdkNotSupported;
}

long MotionCardBase::axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisEnableLtc(axisNo, axisLtc) : kSdkNotSupported;
}

long MotionCardBase::axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	IAxisDriver* d = axisDriver();
	return d ? d->axisGetLtcPos(axisNo, axisLtc, vecPos) : kSdkNotSupported;
}

long MotionCardBase::positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup)
{
	IAxisDriver* d = axisDriver();
	return d ? d->positionCompareSetup(axisNo, setup) : kSdkNotSupported;
}

long MotionCardBase::positionCompareDisable(int axisNo, int channelNo, bool highSpeed)
{
	IAxisDriver* d = axisDriver();
	return d ? d->positionCompareDisable(axisNo, channelNo, highSpeed) : kSdkNotSupported;
}

long MotionCardBase::writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->writePDO(axisNo, slaveId, axisPDO, value) : kSdkNotSupported;
}

long MotionCardBase::readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value)
{
	IAxisDriver* d = axisDriver();
	return d ? d->readPDO(axisNo, slaveId, axisPDO, value) : kSdkNotSupported;
}

long MotionCardBase::writeIO(int bitNo, bool value)
{
	IIoDriver* d = ioDriver();
	return d ? d->writeIO(bitNo, value) : kSdkNotSupported;
}

long MotionCardBase::readIO(int bitNo, bool& value, bool isInputIo)
{
	IIoDriver* d = ioDriver();
	return d ? d->readIO(bitNo, value, isInputIo) : kSdkNotSupported;
}

long MotionCardBase::readDoPorts(std::vector<uint32_t>& vecValue)
{
	IIoDriver* d = ioDriver();
	return d ? d->readDoPorts(vecValue) : kSdkNotSupported;
}

long MotionCardBase::readDiPorts(std::vector<uint32_t>& vecValue)
{
	IIoDriver* d = ioDriver();
	return d ? d->readDiPorts(vecValue) : kSdkNotSupported;
}

long MotionCardBase::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	IIoDriver* d = ioDriver();
	return d ? d->writeExtendIO(nodeNo, bitNo, value) : kSdkNotSupported;
}

long MotionCardBase::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	IIoDriver* d = ioDriver();
	return d ? d->readExtendIO(nodeNo, bitNo, value, isInputIo) : kSdkNotSupported;
}

long MotionCardBase::writeAD(int nodeNo, int channelNo, double value)
{
	IAdDriver* d = adDriver();
	return d ? d->writeAD(nodeNo, channelNo, value) : kSdkNotSupported;
}

long MotionCardBase::readAD(int nodeNo, int channelNo, double& value, bool isInputAd)
{
	IAdDriver* d = adDriver();
	return d ? d->readAD(nodeNo, channelNo, value, isInputAd) : kSdkNotSupported;
}

} // namespace common::device::motion
