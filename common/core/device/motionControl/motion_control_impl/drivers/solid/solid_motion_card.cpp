#include "solid_motion_card.h"
#include "../../motion_sdk_err_map.h"
#include <common/3rdparty/device/motion/Solid/include/sldm.h>
#include <common/include/macros.h>
#include <Logger.h>

extern std::map<uint16_t, std::string> SOLID_ERR_CODE_MAP;

namespace common
{
namespace device
{
namespace motion
{

long SolidMotionCard::initDevice()
{
	short mc;
	long rtn = SLDM_Open(m_deviceConnectOpts_.m_connectString.c_str(), &mc);
	CHECK_RTN_LONG(rtn);

	if (mc <= 0)
		return -4;

	m_cardNo = mc;

	return rtn;
}
void SolidMotionCard::release()
{
	SLDM_Close(m_cardNo);
}
long SolidMotionCard::initAxis(int axisNo)
{
	return 0;
}
long SolidMotionCard::totalAxis(uint32_t& axisNum)
{
	uint32_t period;
	int16_t ch_max;
	int16_t axis_nums;
	short iret = SLDM_GetMcProperty(m_cardNo, &period, &axis_nums, &ch_max);
	axisNum = axis_nums;

	return iret;
}
long SolidMotionCard::axisSetVel(int axisNo, const AxisVelocityProfile& profile)
{
	CHECK_RTN_LONG(SLDM_SetVelS(m_cardNo, axisNo, profile.startVel));
	CHECK_RTN_LONG(SLDM_SetMaxVel(m_cardNo, axisNo, profile.maxVel));
	CHECK_RTN_LONG(SLDM_SetVelE(m_cardNo, axisNo, profile.endVel));
	CHECK_RTN_LONG(SLDM_SetMaxAcc(m_cardNo, axisNo, profile.tacc));
	CHECK_RTN_LONG(SLDM_SetMaxDec(m_cardNo, axisNo, profile.tdec));

	return 0;
}
long SolidMotionCard::crdSetVel(int crdNo, const AxisVelocityProfile& profile)
{
	(void)crdNo;
	(void)profile;
	return 0;
}
long SolidMotionCard::axisSetSMode(int axisNo, int s_mode, double s_para)
{
	return 0;
}
long SolidMotionCard::axisSingleMove(int axisNo, const AxisSingleMoveParams& params)
{
	const auto& p = params.profile;
	if (params.posiMode == 1)
		return SLDM_MoveA(m_cardNo, axisNo, p.startVel, p.maxVel, p.tacc, p.tdec, p.endVel, params.smth);
	return SLDM_MoveR(m_cardNo, axisNo, p.startVel, p.maxVel, p.tacc, p.tdec, p.endVel, params.smth);
}
long SolidMotionCard::axisVMove(int axisNo, uint16_t dir)
{
	short iret = 0;
	if (dir == 1)
		iret = SLDM_JogPlus(m_cardNo, axisNo);
	else
		iret = SLDM_JogMinus(m_cardNo, axisNo);

	return iret;
}
long SolidMotionCard::axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode)
{
	(void)crdNo;
	(void)axisNos;
	(void)dists;
	(void)posiMode;
	return 0;
}
bool SolidMotionCard::axisIsMoveDone(int axisNo)
{
	unsigned int runing = 0;
	short iret = SLDM_GetRunning(m_cardNo, &runing);
	if (iret != 0)
		return false;
	if ((((runing >> axisNo) & 0x01) > 0))
		return false;

	unsigned int reached = 0;
	iret = SLDM_GetReached(m_cardNo, &reached);
	if (iret != 0)
		return false;
	if ((((reached >> axisNo) & 0x01) <= 0))
		return false;

	return true;
}
bool SolidMotionCard::crdIsMoveDone(int crdNo)
{
	return 0;
}
long SolidMotionCard::axisStop(int axisNo, uint8_t mode)
{
	short iret = 0;
	if (mode == 0)
		iret = SLDM_Stop(m_cardNo, axisNo);
	else
		iret = SLDM_EStop(m_cardNo, axisNo);

	return iret;
}
long SolidMotionCard::axisEmgStop()
{
	short iret = 0;
	for (int i = 0; i < 64; i++)
		iret |= SLDM_EStop(m_cardNo, i);

	return iret;
}
long SolidMotionCard::axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome)
{
	int mode = 0;

	//* 0 - 无效
	//* 1 - HOME，零点开关一次回零
	//* 2 - LMT，限位开关一次回零
	//* 3 - Z，编码器Z相信号一次回零
	//* 4 - HOME + Z，原点与编码器Z相信号联合回零
	//* 5 - LMT + Z，限位开关与编码器Z相信号联合回零
	//* 6 - LMT + HOME，限位开关与零点开关联合回零
	//* 7 - LMT + HOME + Z，限位开关、零点开关与编码器Z相信号联合回零
	//* 8 - 原地，仅仅将轴的位置清零
	if (axisHome.m_homeMode == common::device::axis::HomeModeType::Origin_FindHome)mode = 1;
	else if (axisHome.m_homeMode == common::device::axis::HomeModeType::Z_FindHome)mode = 3;
	else if (axisHome.m_homeMode == common::device::axis::HomeModeType::Limit_FindHome)mode = 2;
	else if (axisHome.m_homeMode == common::device::axis::HomeModeType::Origin_Add_Z_FindHome)mode = 4;
	else if (axisHome.m_homeMode == common::device::axis::HomeModeType::Limit_Add_Z_FindHome)mode = 5;

	short iret = SLDM_SetHomeParam(m_cardNo, axisNo, axisHome.m_homeDir, mode, axisHome.m_homeLowVel, axisHome.m_homeHighVel, axisHome.m_homeTacc, axisHome.m_homeOffset);

	return iret;
}
long SolidMotionCard::axisStartHome(int axisNo)
{
	return SLDM_Home(m_cardNo, axisNo);
}
bool SolidMotionCard::axisIsHomeDone(int axisNo)
{
	unsigned int homing = 0;
	short iret = SLDM_GetHoming(m_cardNo, &homing);
	if (iret != 0)
		return false;
	if ((((homing >> axisNo) & 0x01) > 0))
		return false;

	unsigned int homed = 0;
	iret = SLDM_GetHomed(m_cardNo, &homed);
	if (iret != 0)
		return false;
	if ((((homed >> axisNo) & 0x01) <= 0))
		return false;

	return true;
}
long SolidMotionCard::axisAlarmClear(int axisNo)
{
	short iret = SLDM_ResetAlarm(m_cardNo, axisNo, 1);	//清除报警 
	Sleep(50);
	iret |= SLDM_ResetAlarm(m_cardNo, axisNo, 0);		//清除报警完成

	return iret;
}
long SolidMotionCard::axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus)
{
	unsigned int status = 0;
	CHECK_RTN_LONG(SLDM_GetAlarm(m_cardNo, &status));
	axisStatus.ALM = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetPOT(m_cardNo, &status));
	axisStatus.ELP = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetNOT(m_cardNo, &status));
	axisStatus.ELN = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetHome(m_cardNo, &status));
	axisStatus.ORG = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetReached(m_cardNo, &status));
	axisStatus.INP = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetPSLFlag(m_cardNo, &status));
	axisStatus.SLP = ((status >> axisNo) & 0x01) > 0;

	status = 0;
	CHECK_RTN_LONG(SLDM_GetNSLFlag(m_cardNo, &status));
	axisStatus.SLN = ((status >> axisNo) & 0x01) > 0;


	return 0;
}
long SolidMotionCard::axisGetEncoder(int axisNo, double& value)
{
	int pos = 0;
	short iret = SLDM_GetPosEncoder(m_cardNo, axisNo, &pos);
	value = pos;

	return iret;
}
long SolidMotionCard::axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value)
{
	(void)axisNo;
	(void)slaveId;
	(void)driverType;
	(void)encoderType;
	(void)value;
	return 0;
}
long SolidMotionCard::axisGetPosition(int axisNo, double& value)
{
	int pos = 0;
	short iret = SLDM_GetPosPulse(m_cardNo, axisNo, &pos);
	value = pos;

	return iret;
}
long SolidMotionCard::axisSetEncoderAndPosition(int axisNo, double value)
{
	CHECK_RTN_LONG(SLDM_SetPosEncoder(m_cardNo, axisNo, value));
	CHECK_RTN_LONG(SLDM_SetPosPulse(m_cardNo, axisNo, value));
	return 0;
}
long SolidMotionCard::axisCheckSuccessEncoder(int axisNo, double bandRange)
{
	return 0;
}
long SolidMotionCard::axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	CHECK_RTN_LONG(SLDM_SetPSL(m_cardNo, axisNo, axisSoftlimit.m_limitP));
	CHECK_RTN_LONG(SLDM_SetNSL(m_cardNo, axisNo, axisSoftlimit.m_limitN));

	return 0;
}
long SolidMotionCard::axisSetEquiv(int axisNo, double equiv)
{
	return 0;
}
long SolidMotionCard::axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic)
{
	return 0;
}
long SolidMotionCard::axisSetFactorError(int axisNo, double factorError)
{
	return 0;
}
long SolidMotionCard::axisEnable(int axisNo, bool enable)
{
	return SLDM_SetServoOn(m_cardNo, axisNo, (int16_t)enable);
}
long SolidMotionCard::axisIsEnable(int axisNo, bool& enable)
{
	unsigned int status;
	short iret = SLDM_GetServoStatus(m_cardNo, &status);

	enable = (iret == 0) ? (((status >> axisNo) & 0x01) > 0) : false;

	return iret;
}
long SolidMotionCard::axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	return 0;
}
long SolidMotionCard::axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{

	return 0;
}
long SolidMotionCard::positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup)
{
	(void)axisNo;
	(void)setup;
	return kSdkNotSupported;
}
long SolidMotionCard::positionCompareDisable(int axisNo, int channelNo, bool highSpeed)
{
	(void)axisNo;
	(void)channelNo;
	(void)highSpeed;
	return kSdkNotSupported;
}
long SolidMotionCard::writeIO(int bitNo, bool value)
{
	return writeExtendIO(0, bitNo, value);
}
long SolidMotionCard::readIO(int bitNo, bool& value, bool isInputIo)
{
	return readExtendIO(0, bitNo, value, isInputIo);
}
long SolidMotionCard::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	return SLDM_SetOutBit(m_cardNo, nodeNo, bitNo, value ? 1 : 0);
}
long SolidMotionCard::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	short iret = 0;
	unsigned short vue = 0;
	if (isInputIo)
	{
		iret = SLDM_GetInBit(m_cardNo, nodeNo, bitNo, &vue);
	}
	else
	{
		iret = SLDM_GetOutBit(m_cardNo, nodeNo, bitNo, &vue);
	}
	value = (vue != 0);
	return iret;
}
long SolidMotionCard::writeAD(int nodeNo, int channelNo, double value)
{
	return 0;
}
long SolidMotionCard::readAD(int nodeNo, int channelNo, double& value, bool isInputAd)
{
	return 0;
}
long SolidMotionCard::writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}
long SolidMotionCard::readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}
bool SolidMotionCard::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	if (rtn == 0)
		return true;

	auto iter = SOLID_ERR_CODE_MAP.find(rtn);
	if (iter != SOLID_ERR_CODE_MAP.end()) {
		rtnStr = iter->second;
	}
	else {
		rtnStr = "unknown error";
	}
	LOG_ERROR("[common][device][motion]---{} failed, sdk_ret={}, sdk_errInfo={}", strFun, rtn, rtnStr);

	return false;
}

} // end of namespace motion

} // end of namespace device

}

