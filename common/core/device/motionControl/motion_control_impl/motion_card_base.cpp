#include "motion_card_base.h"
#include "../motion_error_codes.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include <utility/string_tool/time_format.hpp>
#include <common/include/macros.h>

namespace common {
namespace device {
namespace motion {


namespace {
inline std::string MotionSdkDetail(long sdkCode, const std::string& sdkMsg) {
	std::ostringstream oss;
	oss << "sdk_code=" << sdkCode;
	if (!sdkMsg.empty()) {
		oss << ", sdk_msg=" << sdkMsg;
	}
	return oss.str();
}

/** JOG 速度比例：外部入参限制在 [0.001, 0.8] */
double clampJogVelRatio(double velRatio)
{
	if (velRatio < 0.001)
		return 0.001;
	if (velRatio > 0.8)
		return 0.8;
	return velRatio;
}
} // namespace

ErrorPtr MotionCardBase::InitDevice() {
    if (m_inited) {
        return nullptr;
    }
    m_cardNo = m_deviceConnectOpts_.m_connectNO;
    CHECK_RTN_INFO(checkError(initDevice(), "initDevice"));
    CHECK_RTN_INFO(InitAxis());
    m_inited = true;
    return nullptr;
}

void MotionCardBase::Release() {
    release();
    m_inited = false;
}

ErrorPtr MotionCardBase::InitAxis() {
    for (auto& axisItem : m_motionAxisMap_) {
        const auto axisData = axisItem.second->getAxisParams();
        CHECK_RTN_INFO(checkError(initAxis(axisData.m_axisNO), "initAxis", axisItem.first));
    }

    for (auto& axisItem : m_motionAxisMap_) {
        const auto axisName = axisItem.first;
        const auto axisData = axisItem.second->getAxisParams();
        const auto moveData = axisItem.second->getMoveParameters();

        CHECK_RTN_INFO(checkError(axisAlarmClear(axisData.m_axisNO), "axisAlarmClear", axisName));
        if (axisData.m_equiv > 0) {
            CHECK_RTN_INFO(checkError(axisSetEquiv(axisData.m_axisNO, axisData.m_equiv), "axisSetEquiv", axisName));
        }
        CHECK_RTN_INFO(checkError(axisSetFactorError(axisData.m_axisNO, axisData.m_errorBandPluse), "axisSetFactorError", axisName));

        common::device::axis::Axis::AxisStaLogic axisStaLogic{};
        axisStaLogic.m_orgLogic = axisData.m_homePinLogic;
        CHECK_RTN_INFO(checkError(axisSetStaLogic(axisData.m_axisNO, axisStaLogic), "axisSetStaLogic", axisName));

        if (axisData.m_positiveLimit > axisData.m_negativeLimit) {
            const bool enableSoftLimit = std::abs(axisData.m_negativeLimit - axisData.m_positiveLimit) >= 10;
            const common::device::axis::Axis::AxisSoftlimit softlimit(
                enableSoftLimit,
                axisData.m_sourceSelLimit,
                axisData.m_softwareLimitAction,
                axisData.m_negativeLimit,
                axisData.m_positiveLimit);
            CHECK_RTN_INFO(checkError(axisSetSoftlimit(axisData.m_axisNO, softlimit), "axisSetSoftlimit", axisName));
        }

        CHECK_RTN_INFO(checkError(axisSetVel(axisData.m_axisNO, {moveData.m_startVel, moveData.m_maxVel, moveData.m_tacc, moveData.m_tdec, moveData.m_stopVel}), "axisSetVel", axisName));
        CHECK_RTN_INFO(checkError(axisSetSMode(axisData.m_axisNO, 0, moveData.m_sPara), "axisSetSMode", axisName));
    }
    return nullptr;
}

ErrorPtr MotionCardBase::TotalAxis(uint32_t& axisNum) {
    CHECK_RTN_INFO(checkError(totalAxis(axisNum), "totalAxis"));
    return nullptr;
}

ErrorPtr MotionCardBase::AxisEnable(const std::string& axisName, bool enable) {
    axis::Axis::AxisData axisData;
    if (!getAxisParams(axisName, axisData)) {
        return common::errors::Newf(common::errors::motion_module::kAxisNotFound, "Axis[%s] not found.", axisName.c_str());
    }
    CHECK_RTN_INFO(checkError(axisEnable(axisData.m_axisNO, enable), "axisEnable", axisName));
    return nullptr;
}
ErrorPtr MotionCardBase::AxisIsEnable(const std::string& axisName, bool& enable)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisIsEnable(axisData.m_axisNO, enable);
		CHECK_RTN_INFO(checkError(iret, "axisIsEnable", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisMove(const std::string& axisName, const AxisMoveParams& params)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	axis::Axis::MoveParameters axisMoveData;
	if (getAxisParams(axisName, axisData) && getMoveParameters(axisName, axisMoveData))
	{
		short iret = 0;

		iret = axisSetSMode(axisData.m_axisNO, 0, axisMoveData.m_sPara);
		CHECK_RTN_INFO(checkError(iret, "axisSetSMode", axisName));

		AxisSingleMoveParams moveParams{
			params.dist * axisData.m_pulseUnit,
			static_cast<int>(params.posiMode),
			{
				axisMoveData.m_startVel,
				params.speed <= 0.0 ? axisMoveData.m_maxVel : params.speed,
				axisMoveData.m_tacc,
				axisMoveData.m_tdec,
				axisMoveData.m_stopVel},
			0.0};
		iret = axisSingleMove(axisData.m_axisNO, moveParams);
		CHECK_RTN_INFO(checkError(iret, "axisSingleMove", axisName));

		axis::Axis::AxisProcessData proc;
		if (getAxisProcessData(axisName, proc)) {
			proc.m_runMode = 1;
			setAxisProcessData(axisName, proc);
		}

		if (params.wait)
		{
			resultInfo = AxisWaitMoveDone(axisName, params.timeout);
		}

	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisLinear(const std::vector<std::string>& axisNames, const AxisLinearParams& params)
{
	ErrorPtr resultInfo = nullptr;
	if (axisNames.size() != params.dists.size() || axisNames.size() < 2)
	{
		return common::errors::New(common::errors::motion_module::kInvalidAxisCount, "Wrong number of axis in linear");
	}

	axis::Axis::AxisData axisData;
	axis::Axis::MoveParameters axisMoveData;
	std::vector<int> vecAxisNo;
	std::vector<double> vecAxisDist;
	for (unsigned i = 0; i < axisNames.size(); i++)
	{
		axis::Axis::AxisData axisData;
		if (getAxisParams(axisNames[i], axisData))
		{
			vecAxisNo.push_back(axisData.m_axisNO);
			vecAxisDist.push_back(params.dists[i] * axisData.m_pulseUnit);
		}
	}

	if (getAxisParams(axisNames[0], axisData) && getMoveParameters(axisNames[0], axisMoveData))
	{
		short iret = crdSetVel(params.crdNo, {axisMoveData.m_startVel, axisMoveData.m_maxVel, axisMoveData.m_tacc, axisMoveData.m_tdec, axisMoveData.m_stopVel});
		CHECK_RTN_INFO(checkError(iret, "crdSetVel", axisNames[0]));

		iret = axisLinear(params.crdNo, vecAxisNo, vecAxisDist, params.posiMode);
		CHECK_RTN_INFO(checkError(iret, "axisLinear", axisNames[0]));

		uint64_t beginTm = common::utility::TimeFormat::GetCurrentMs();
		do
		{
			uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
			if (params.timeout < (currentTm - beginTm))
			{
				return common::errors::New(common::errors::motion_module::kMotionTimeout, "Wait for axis linear motion timeout");
			}
			if (crdIsMoveDone(params.crdNo))
				break;
			std::this_thread::yield();
		} while (true);
	}
	return resultInfo;

}
ErrorPtr MotionCardBase::AxisVmove(const std::string& axisName, uint16_t dir, double velRatio)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	axis::Axis::MoveParameters axisMoveData;
	if (getAxisParams(axisName, axisData) && getMoveParameters(axisName, axisMoveData))
	{
		const double ratio = clampJogVelRatio(velRatio);

		axis::Axis::AxisProcessData proc;
		proc.m_moveDir = (dir != 0);
		proc.m_runMode = 2;
		setAxisProcessData(axisName, proc);

		long iret = 0;
		iret = axisSetVel(axisData.m_axisNO,
		                  {axisMoveData.m_startVel, axisMoveData.m_maxVel * ratio, axisMoveData.m_tacc,
		                   axisMoveData.m_tdec, axisMoveData.m_stopVel});
		CHECK_RTN_INFO(checkError(iret, "axisSetVel", axisName));

		iret = axisVMove(axisData.m_axisNO, dir);
		CHECK_RTN_INFO(checkError(iret, "axisVMove", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisWaitMoveDone(const std::string& axisName, int timeout)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		uint64_t beginTm = common::utility::TimeFormat::GetCurrentMs();
		bool enterBondRange = false;
		double encoderValue = 0.0;
		double positionValue = 0.0;
		do
		{
			uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
			if (timeout < (currentTm - beginTm))
			{
				resultInfo = common::errors::Newf(common::errors::motion_module::kMotionTimeout, "Wait for axis motion timeout:%s", axisName.c_str());
				if (enterBondRange) {
					LOG_WARN("[common][device][motion]---encoderValue({}) != positionValue({}), 轴名[{}],轴号[{}]", encoderValue, positionValue, axisName, axisData.m_axisNO);
				}
				return resultInfo;
			}
			// 判断轴是否停止
			if (axisIsMoveDone(axisData.m_axisNO))
			{
				// 判断是否在误差范围以内
				if (axisData.m_errorBandPluse > 0)
				{
					enterBondRange = true;
					encoderValue = 0.0;
					positionValue = 0.0;
					long iret = axisGetEncoder(axisData.m_axisNO, encoderValue);
					CHECK_RTN_INFO(checkError(iret, "axisGetEncoder", axisName));
					iret = axisGetPosition(axisData.m_axisNO, positionValue);
					CHECK_RTN_INFO(checkError(iret, "axisGetPosition", axisName));
					if (abs(encoderValue - positionValue) >= axisData.m_errorBandPluse)
					{
						continue;
					}
				}
				break;
			}
			std::this_thread::yield();
		} while (true);

		axis::Axis::AxisProcessData proc;
		if (getAxisProcessData(axisName, proc)) {
			proc.m_runMode = 0;
			setAxisProcessData(axisName, proc);
		}
	}

	return resultInfo;
}
ErrorPtr MotionCardBase::AxisStop(const std::string& axisName, uint8_t mode)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		if (axisIsMoveDone(axisData.m_axisNO))
			return resultInfo;

		long iret = axisStop(axisData.m_axisNO, mode);
		CHECK_RTN_INFO(checkError(iret, "axisStop", axisName));

		axis::Axis::AxisProcessData proc;
		if (getAxisProcessData(axisName, proc)) {
			proc.m_runMode = 0;
			setAxisProcessData(axisName, proc);
		}

		resultInfo = AxisWaitMoveDone(axisName);
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisEmgStop()
{
	ErrorPtr resultInfo = nullptr;
	long iret = axisEmgStop();
	CHECK_RTN_INFO(checkError(iret, "axisEmgStop"));

	return resultInfo;
}
ErrorPtr MotionCardBase::AxisReset(const std::string& axisName, int timeout)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (!getAxisParams(axisName, axisData)) {
		CHECK_RTN_INFO(checkError(1, "获取轴参数失败", axisName));
	}

	common::device::axis::Axis::AxisSta axisStatus;
	long iret = axisReadStatus(axisData.m_axisNO, axisStatus);
	CHECK_RTN_INFO(checkError(iret, "axisReadStatus", axisName));
	//判断轴是否报警
	if (axisStatus.ALM)
	{
		iret = axisAlarmClear(axisData.m_axisNO);
		CHECK_RTN_INFO(checkError(iret, "axisAlarmClear", axisName));

		iret = axisReadStatus(axisData.m_axisNO, axisStatus);
		CHECK_RTN_INFO(checkError(iret, "axisReadStatus", axisName));
		if (axisStatus.ALM)
		{
			return common::errors::Newf(common::errors::motion_module::kAxisAlarm, "Axis alarm:%s", axisName.c_str());
		}
	}
	//停止轴运动
	iret = axisStop(axisData.m_axisNO);
	CHECK_RTN_INFO(checkError(iret, "axisStop", axisName));
	//回零之前使能轴
	iret = axisEnable(axisData.m_axisNO, true);
	CHECK_RTN_INFO(checkError(iret, "axisEnable", axisName));
	//指令位置和编码器位置清零
	iret = axisSetEncoderAndPosition(axisData.m_axisNO, 0);
	CHECK_RTN_INFO(checkError(iret, "axisSetEncoderAndPosition", axisName));

	//使能即回零
	if (axisData.m_homeMode == axis::HomeModeType::Enable)
	{
		if (axisData.m_encoderType == 1)
		{
			//绝对式编码器
			double currentPosition = 0.0;
			iret = axisGetAbsEncoder(axisData.m_axisNO, axisData.m_slaveID, axisData.m_driverType, axisData.m_encoderType, currentPosition);
			CHECK_RTN_INFO(checkError(iret, "axisGetAbsEncoder", axisName));

			//设置到当前的指令位置和编码器位置
	     	iret = axisSetEncoderAndPosition(axisData.m_axisNO, currentPosition);
	     	CHECK_RTN_INFO(checkError(iret, "axisSetEncoderAndPosition", axisName));

			//去到零点位置
			CHECK_RTN_INFO(AxisMove(axisName, {0.0, 1u}));
		}
		else
		{
			//增量式编码器
		}
	}
	else 
	{
		//设置回零参数
		common::device::axis::Axis::AxisHome axisHome;
		axisHome.m_homeDir = axisData.m_homeDir;
		axisHome.m_homeEdgeType = axisData.m_homeEdgeType;
		axisHome.m_homeHighVel = axisData.m_homeHighVel;
		axisHome.m_homeLowVel = axisData.m_homeLowVel;
		axisHome.m_homeMode = axisData.m_homeMode;
		axisHome.m_homeOffset = 0;//axisData.m_homeOffset;
		axisHome.m_homeSourceSel = axisData.m_homeSourceSel;
		axisHome.m_homeTacc = axisData.m_homeTacc;
		axisHome.m_homeVelMode = axisData.m_homeVelMode;

		iret = axisSetHomeParam(axisData.m_axisNO, axisHome);
		CHECK_RTN_INFO(checkError(iret, "axisSetHomeParam", axisName));
		//启动回零运动
		iret = axisStartHome(axisData.m_axisNO);
		CHECK_RTN_INFO(checkError(iret, "axisStartHome", axisName));

		bool homeDone = false;
		if (timeout <= 10000)
			timeout = 10000;
		uint64_t beginTm = common::utility::TimeFormat::GetCurrentMs();
		do {
			uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
			if (timeout < (currentTm - beginTm)){
				return common::errors::Newf(common::errors::motion_module::kAxisResetTimeout, "%s axis reset timeout: %dms", axisName.c_str(), timeout);
			}

			common::device::axis::Axis::AxisSta status;
			axisReadStatus(axisData.m_axisNO, status);
			if (status.ALM) {
				return common::errors::Newf(common::errors::motion_module::kAxisAlarm, "%s axis alarm during reset", axisName.c_str());
			}

			bool enable = false;
			axisIsEnable(axisData.m_axisNO, enable);
			if (!enable) {
				return common::errors::Newf(common::errors::motion_module::kAxisDisabled, "%s axis disabled during reset", axisName.c_str());
			}

			homeDone = axisIsHomeDone(axisData.m_axisNO);
			std::this_thread::yield();
		} while (!homeDone);

		if (homeDone)
		{
			//回零完成以后 指令位置和编码器位置清零
			Sleep(200);
			iret = axisSetEncoderAndPosition(axisData.m_axisNO, 0);
			CHECK_RTN_INFO(checkError(iret, "axisSetEncoderAndPosition", axisName));

			if (axisData.m_homeOffset != 0)
			{
				LOG_WARN("[common][device][motion]---回零后偏移为:{}", axisData.m_homeOffset);
				CHECK_RTN_INFO(AxisMove(axisName, { axisData.m_homeOffset, 1u }));
			}
		}
	}

	//停止轴运动状态
	axisStop(axisData.m_axisNO);

	return resultInfo;
}
ErrorPtr MotionCardBase::AxisAlarmClear(const std::string& axisName)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisAlarmClear(axisData.m_axisNO);
		CHECK_RTN_INFO(checkError(iret, "axisAlarmClear", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisReadStatus(const std::string& axisName, common::device::axis::Axis::AxisSta& axisStatus)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisReadStatus(axisData.m_axisNO, axisStatus);
		CHECK_RTN_INFO(checkError(iret, "axisReadStatus", axisName));

		axis::Axis::AxisProcessData proc;
		if (getAxisProcessData(axisName, proc)) {
			if (axisIsMoveDone(axisData.m_axisNO))
				proc.m_runMode = 0;
			setAxisProcessData(axisName, proc);
			axisStatus.MOVE_DIR = proc.m_moveDir;
			axisStatus.RUN_MODE = proc.m_runMode;
		}
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisGetEncoder(const std::string& axisName, double& value)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisGetEncoder(axisData.m_axisNO, value);
		CHECK_RTN_INFO(checkError(iret, "axisGetEncoder", axisName));
		if (axisData.m_pulseUnit > 0)
			value /= axisData.m_pulseUnit;
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisGetPosition(const std::string& axisName, double& value)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisGetPosition(axisData.m_axisNO, value);
		CHECK_RTN_INFO(checkError(iret, "axisGetPosition", axisName));
		if (axisData.m_pulseUnit > 0)
			value /= axisData.m_pulseUnit;
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisSetEncoderAndPosition(const std::string& axisName, double value)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisSetEncoderAndPosition(axisData.m_axisNO, value * axisData.m_pulseUnit);
		CHECK_RTN_INFO(checkError(iret, "axisSetEncoderAndPosition", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisSetSoftlimit(const std::string& axisName, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisSetSoftlimit(axisData.m_axisNO, axisSoftlimit);
		CHECK_RTN_INFO(checkError(iret, "axisSetSoftlimit", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisEnableLtc(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisEnableLtc(axisData.m_axisNO, axisLtc);
		CHECK_RTN_INFO(checkError(iret, "axisEnableLtc", axisName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::AxisGetLtcPos(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	ErrorPtr resultInfo = nullptr;
	axis::Axis::AxisData axisData;
	if (getAxisParams(axisName, axisData))
	{
		long iret = axisGetLtcPos(axisData.m_axisNO, axisLtc, vecPos);
		CHECK_RTN_INFO(checkError(iret, "axisGetLtcPos", axisName));
	}
	return resultInfo;
}

namespace {
int PositionCompareChannelIndex(PositionCompareChannel channel)
{
	switch (channel) {
	case PositionCompareChannel::HighSpeed0: return 0;
	case PositionCompareChannel::HighSpeed1: return 1;
	default: return 0;
	}
}

bool BuildPositionCompareDriverSetupFromPath(
	double pathStart,
	double pathEnd,
	const std::vector<double>& comparePositionsEng,
	const axis::Axis::AxisData& compareAxisData,
	PositionCompareChannel channel,
	PositionCompareSource compareSource,
	const PositionCompareOutputConfig& output,
	const deviceIO::DeviceIO::DeviceIoData& ioData,
	PositionCompareDriverSetup& setup,
	std::string& errorMsg)
{
	const bool increasing = pathEnd >= pathStart;

	setup.highSpeed = channel != PositionCompareChannel::LowSpeed;
	setup.channelNo = PositionCompareChannelIndex(channel);
	setup.mode = 4;
	setup.compareSource = static_cast<int>(compareSource);
	setup.cmpLogic = output.cmpLogic;
	setup.pulseWidthUs = static_cast<long>(output.pulseWidthMs) * 1000L;
	setup.lowSpeedDir = increasing ? 1 : 0;
	setup.lowSpeedAction = output.action;
	setup.lowSpeedIoBit = ioData.m_port;
	setup.positions.clear();

	const double pathMin = std::min(pathStart, pathEnd);
	const double pathMax = std::max(pathStart, pathEnd);
	for (double pos : comparePositionsEng) {
		if (pos < pathMin || pos > pathMax) {
			errorMsg = "compare position out of motion path range";
			return false;
		}
		setup.positions.push_back(pos * compareAxisData.m_pulseUnit);
	}
	if (setup.positions.empty()) {
		errorMsg = "compare positions empty";
		return false;
	}
	std::sort(setup.positions.begin(), setup.positions.end());
	if (!increasing) {
		std::reverse(setup.positions.begin(), setup.positions.end());
	}
	return true;
}

bool BuildPositionCompareDriverSetup(
	const LinearPositionCompareParams& params,
	const axis::Axis::AxisData& compareAxisData,
	const deviceIO::DeviceIO::DeviceIoData& ioData,
	PositionCompareDriverSetup& setup,
	std::string& errorMsg)
{
	const double compareStart = params.compareRefAxis == PositionCompareRefAxis::X ? params.startX : params.startY;
	const double compareEnd = params.compareRefAxis == PositionCompareRefAxis::X ? params.endX : params.endY;
	return BuildPositionCompareDriverSetupFromPath(
		compareStart, compareEnd, params.comparePositions, compareAxisData,
		params.channel, params.compareSource, params.output, ioData, setup, errorMsg);
}
} // namespace

ErrorPtr MotionCardBase::SetupPositionCompare(const std::string& compareAxisName, const PositionCompareDriverSetup& setup)
{
	axis::Axis::AxisData axisData;
	if (!getAxisParams(compareAxisName, axisData)) {
		return common::errors::Newf(common::errors::motion_module::kAxisNotFound, "Axis[%s] not found.", compareAxisName.c_str());
	}
	if (setup.positions.empty()) {
		return common::errors::New(common::errors::motion_module::kInvalidComparePoints, "compare positions empty");
	}
	long iret = positionCompareSetup(axisData.m_axisNO, setup);
	CHECK_RTN_INFO(checkError(iret, "positionCompareSetup", compareAxisName));
	return nullptr;
}

ErrorPtr MotionCardBase::DisablePositionCompare(const PositionCompareDisableParams& params)
{
	axis::Axis::AxisData axisData;
	if (!getAxisParams(params.compareAxisName, axisData)) {
		return common::errors::Newf(common::errors::motion_module::kAxisNotFound, "Axis[%s] not found.", params.compareAxisName.c_str());
	}
	const bool highSpeed = params.channel != PositionCompareChannel::LowSpeed;
	const int channelNo = PositionCompareChannelIndex(params.channel);
	long iret = positionCompareDisable(axisData.m_axisNO, channelNo, highSpeed);
	CHECK_RTN_INFO(checkError(iret, "positionCompareDisable", params.compareAxisName));
	return nullptr;
}

ErrorPtr MotionCardBase::AxisLinearWithPositionCompare(const LinearPositionCompareParams& params)
{
	if (params.comparePositions.empty()) {
		return common::errors::New(common::errors::motion_module::kInvalidComparePoints, "compare positions empty");
	}

	axis::Axis::AxisData xAxisData;
	axis::Axis::AxisData yAxisData;
	if (!getAxisParams(params.axisXName, xAxisData) || !getAxisParams(params.axisYName, yAxisData)) {
		return common::errors::New(common::errors::motion_module::kAxisNotFound, "linear axis not found");
	}

	const std::string compareAxisName =
		params.compareRefAxis == PositionCompareRefAxis::X ? params.axisXName : params.axisYName;
	axis::Axis::AxisData compareAxisData;
	if (!getAxisParams(compareAxisName, compareAxisData)) {
		return common::errors::Newf(common::errors::motion_module::kAxisNotFound, "Axis[%s] not found.", compareAxisName.c_str());
	}

	deviceIO::DeviceIO::DeviceIoData ioData;
	if (params.channel == PositionCompareChannel::LowSpeed) {
		if (!getIoParams(params.output.ioName, ioData)) {
			return common::errors::Newf(common::errors::motion_module::kIoCardNotMapped, "IO[%s] not found.", params.output.ioName.c_str());
		}
	}

	PositionCompareDriverSetup driverSetup;
	std::string buildError;
	if (!BuildPositionCompareDriverSetup(params, compareAxisData, ioData, driverSetup, buildError)) {
		return common::errors::New(common::errors::motion_module::kInvalidComparePoints, buildError);
	}

	ErrorPtr setupErr = SetupPositionCompare(compareAxisName, driverSetup);
	if (setupErr) {
		return setupErr;
	}

	const std::vector<std::string> axisNames = {params.axisXName, params.axisYName};

	if (params.moveToStartFirst) {
		AxisLinearParams toStart;
		toStart.dists = {params.startX, params.startY};
		toStart.posiMode = 1;
		toStart.crdNo = params.crdNo;
		toStart.timeout = params.timeout;
		CHECK_RTN_INFO(AxisLinear(axisNames, toStart));
	}

	AxisLinearParams toEnd;
	toEnd.dists = {params.endX, params.endY};
	toEnd.posiMode = 1;
	toEnd.crdNo = params.crdNo;
	toEnd.timeout = params.timeout;
	return AxisLinear(axisNames, toEnd);
}

ErrorPtr MotionCardBase::AxisMoveWithPositionCompare(const std::string& axisName, const AxisMoveWithPositionCompareParams& params)
{
	if (params.comparePositions.empty()) {
		return common::errors::New(common::errors::motion_module::kInvalidComparePoints, "compare positions empty");
	}

	axis::Axis::AxisData axisData;
	if (!getAxisParams(axisName, axisData)) {
		return common::errors::Newf(common::errors::motion_module::kAxisNotFound, "Axis[%s] not found.", axisName.c_str());
	}

	deviceIO::DeviceIO::DeviceIoData ioData;
	if (params.channel == PositionCompareChannel::LowSpeed) {
		if (!getIoParams(params.output.ioName, ioData)) {
			return common::errors::Newf(common::errors::motion_module::kIoCardNotMapped, "IO[%s] not found.", params.output.ioName.c_str());
		}
	}

	PositionCompareDriverSetup driverSetup;
	std::string buildError;
	if (!BuildPositionCompareDriverSetupFromPath(
		params.startPos, params.endPos, params.comparePositions, axisData,
		params.channel, params.compareSource, params.output, ioData, driverSetup, buildError)) {
		return common::errors::New(common::errors::motion_module::kInvalidComparePoints, buildError);
	}

	CHECK_RTN_INFO(SetupPositionCompare(axisName, driverSetup));

	if (params.moveToStartFirst) {
		AxisMoveParams toStart;
		toStart.dist = params.startPos;
		toStart.posiMode = 1;
		toStart.speed = params.speed;
		toStart.timeout = params.timeout;
		CHECK_RTN_INFO(AxisMove(axisName, toStart));
	}

	AxisMoveParams toEnd;
	toEnd.dist = params.endPos;
	toEnd.posiMode = 1;
	toEnd.speed = params.speed;
	toEnd.timeout = params.timeout;
	return AxisMove(axisName, toEnd);
}
ErrorPtr MotionCardBase::WriteIO(const std::string& ioName, bool value)
{
	ErrorPtr resultInfo = nullptr;
	deviceIO::DeviceIO::DeviceIoData ioData;
	if (getIoParams(ioName, ioData))
	{
		long iret = 0;
		//判断有效电平
		if (ioData.m_logic == 0)
			value = !value;

		//区分是本地IO还是扩展IO
		if (ioData.m_extend == 0)
		{
			iret = writeIO(ioData.m_port, value);
			CHECK_RTN_INFO(checkError(iret, "writeIO", ioName));
		}
		else
		{
			iret = writeExtendIO(ioData.m_node, ioData.m_port, value);
			CHECK_RTN_INFO(checkError(iret, "writeExtendIO", ioName));
		}
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::ReadIO(const std::string& ioName, bool& value)
{
	ErrorPtr resultInfo = nullptr;
	deviceIO::DeviceIO::DeviceIoData ioData;
	if (getIoParams(ioName, ioData))
	{
		long iret = 0;

		//区分是本地IO还是扩展IO
		if (ioData.m_extend == 0)
		{
			iret = readIO(ioData.m_port, value, ioData.m_type == 0);
			CHECK_RTN_INFO(checkError(iret, "readIO", ioName));
		}
		else
		{
			iret = readExtendIO(ioData.m_node, ioData.m_port, value, ioData.m_type == 0);
			CHECK_RTN_INFO(checkError(iret, "readExtendIO", ioName));
		}
		if (ioData.m_logic == 0)
			value = !value;
	}
	return resultInfo;
}

ErrorPtr MotionCardBase::ReadDoPorts(std::vector<uint32_t>& vecValue)
{
	const long iret = readDoPorts(vecValue);
	CHECK_RTN_INFO(checkError(iret, "readDoPorts"));
	return nullptr;
}

ErrorPtr MotionCardBase::ReadDiPorts(std::vector<uint32_t>& vecValue)
{
	const long iret = readDiPorts(vecValue);
	CHECK_RTN_INFO(checkError(iret, "readDiPorts"));
	return nullptr;
}

ErrorPtr MotionCardBase::WriteAD(const std::string& adName, double value)
{
	ErrorPtr resultInfo = nullptr;
	deviceAD::DeviceAD::DeviceAdData adData;
	if (getAdParams(adName, adData))
	{
		long iret = writeAD(adData.m_node, adData.m_channel, value);
		CHECK_RTN_INFO(checkError(iret, "writeAD", adName));
	}
	return resultInfo;
}
ErrorPtr MotionCardBase::ReadAD(const std::string& adName, double& value)
{
	ErrorPtr resultInfo = nullptr;
	deviceAD::DeviceAD::DeviceAdData adData;
	if (getAdParams(adName, adData))
	{
		long iret = readAD(adData.m_node, adData.m_channel, value, adData.m_type == 0);
		CHECK_RTN_INFO(checkError(iret, "readAD", adName));
	}
	return resultInfo;
}

/**
 * @brief 获取驱动器支持的标准回零模式（35种模式）
 * 
 * 本函数根据回零类型、回零方向和边沿类型，将用户配置转换为标准回零模式号
 * 
 * @param homeType 回零类型：
 *   0 = 原点回零（使用原点传感器/Home开关，速度快但精度较低）
 *   1 = Z相回零（使用编码器Z相信号，精度高但需要编码器支持）
 *   2 = 限位回零（使用限位开关，精度最低但最简单）
 *   3 = 原点+Z相回零（先找原点传感器，再找Z相，精度最高，适合高精度应用）
 *   4 = 限位+Z相回零（先找限位开关，再找Z相，精度较高）
 * 
 * @param homeDir 回零方向：
 *   0 = 负向回零（向负方向运动找零点，即向减小位置值的方向运动）
 *   1 = 正向回零（向正方向运动找零点，即向增大位置值的方向运动）
 * 
 * @param homeEdgeType 边沿类型（仅在原点回零homeType=0和原点+Z相回零homeType=3时有效）：
 *   0 = 下降沿触发（无限位保护，原点信号从高电平变为低电平时触发，最常用，适用于NPN型传感器）
 *   1 = 上升沿触发（无限位保护，原点信号从低电平变为高电平时触发，适用于PNP型传感器）
 *   2 = 有限位保护 + 下降沿（先向回零方向运动，如碰到限位开关则反转，然后找原点下降沿）
 *   3 = 有限位保护 + 上升沿（先向回零方向运动，如碰到限位开关则反转，然后找原点上升沿）
 *   4 = 有限位保护 + 双沿检测（先找上升沿，再找下降沿，最终以上升沿为准，精度最高但耗时较长）
 *   5 = 有限位保护 + 双沿检测（先找下降沿，再找上升沿，最终以下降沿为准，精度最高但耗时较长）
 * 
 * 
 * 【模式1-2：限位+Z相回零（6098h=1-2）】
 *   模式1（6098h=1）：反向超程开关+Z相回零
 *        机械原点：电机Z信号；减速点：反向超程开关（N-OT）
 *        动作流程：
 *          - 回零启动时N-OT=0：以反向高速开始回零 → 遇到N-OT上升沿后减速反向 → 正向低速运行 → 遇到N-OT下降沿后的第一个Z信号停机
 *          - 回零启动时N-OT=1：直接正向低速开始回零 → 遇到N-OT下降沿后的第一个Z信号停机
 *        适用：有反向超程开关（负限位）和编码器Z相，需要较高精度
 *   模式2（6098h=2）：正向超程开关+Z相回零
 *        机械原点：电机Z信号；减速点：正向超程开关（P-OT）
 *        动作流程：
 *          - 回零启动时P-OT=0：以正向高速开始回零 → 遇到P-OT上升沿后减速反向 → 反向低速运行 → 遇到P-OT下降沿后的第一个Z信号停机
 *          - 回零启动时P-OT=1：直接反向低速开始回零 → 遇到P-OT下降沿后的第一个Z信号停机
 *        适用：有正向超程开关（正限位）和编码器Z相，需要较高精度
 * 
 * 【模式3-6：原点+Z相回零（基本模式，无限位保护，6098h=3-6）】
 *   模式3（6098h=3）：原点正向找Z相回零（下降沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0：以正向高速开始回零 → 遇到HW上升沿后减速反向 → 反向低速运行 → 遇到HW下降沿后继续运行 → 之后遇到第一个Z停机
 *          - 回零启动时HW=1：直接反向低速开始回零 → 遇到HW下降沿后的第一个Z停机
 *        适用：有原点传感器和编码器Z相，需要高精度，初始位置在原点传感器范围内
 *   模式4（6098h=4）：原点正向找Z相回零（上升沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0：直接正向低速开始回零 → 遇到HW上升沿后第一个Z停机
 *          - 回零启动时HW=1：以反向高速开始回零 → 遇到HW下降沿后减速反向 → 正向低速运行 → 遇到HW上升沿后的第一个Z停机
 *        适用：有原点传感器和编码器Z相，需要高精度，初始位置在原点传感器范围内
 *   模式5（6098h=5）：原点负向找Z相回零（下降沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0：以反向高速开始回零 → 遇到HW上升沿后减速反向 → 正向低速运行 → 遇到HW下降沿后的第一个Z停机
 *          - 回零启动时HW=1：直接正向低速开始回零 → 遇到HW下降沿后的第一个Z停机
 *        适用：有原点传感器和编码器Z相，需要高精度，初始位置在原点传感器范围内
 *   模式6（6098h=6）：原点负向找Z相回零（上升沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0：直接反向低速开始回零 → 遇到HW上升沿后第一个Z停机
 *          - 回零启动时HW=1：以正向高速开始回零 → 遇到HW下降沿后减速反向 → 反向低速运行 → 遇到HW上升沿后的第一个Z停机
 *        适用：有原点传感器和编码器Z相，需要高精度，初始位置在原点传感器范围内
 * 
 * 【模式7-10：原点+Z相回零（带限位保护，正向，6098h=7-10）】
 *   模式7（6098h=7）：原点正向找Z相回零（限位保护+下降沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0，未遇到正向限位：以正向高速开始回零 → 遇到HW上升沿后减速反向 → 反向低速运行 → 遇到HW下降沿后的第一个Z停机
 *          - 回零启动时HW=0，遇到正向限位：以正向高速开始回零 → 若遇到限位开关自动反向 → 反向高速运行 → 遇到HW上升沿后减速继续反向低速运行 → 遇到HW下降沿后的第一个Z停机
 *          - 回零启动时HW=1：直接反向低速开始回零 → 遇到HW下降沿后的第一个Z停机
 *        适用：有正限位、原点传感器和编码器Z相，不确定初始位置
 *   模式8（6098h=8）：原点正向找Z相回零（限位保护+双沿检测，上升沿优先）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0，未遇到正向限位：以正向高速开始回零 → 遇到HW上升沿后减速反向 → 反向低速运行 → 遇到HW下降沿后反向 → 正向低速运行 → 遇到HW上升沿后的第一个Z停机
 *          - 回零启动时HW=0，遇到正向限位：以正向高速开始回零 → 若遇到限位开关自动反向 → 反向高速运行 → 遇到HW上升沿后减速反向低速运行 → 遇到HW下降沿后反向正向低速 → 遇到HW上升沿后的第一个Z停机
 *          - 回零启动时HW=1：直接反向低速开始回零 → 遇到HW下降沿后反向正向低速 → 遇到HW上升沿后的第一个Z停机
 *        适用：有正限位、原点传感器和编码器Z相，需要最高精度
 *   模式9（6098h=9）：原点正向找Z相回零（限位保护+双沿检测，上升沿优先）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0，未遇到正向限位：以正向高速开始回零 → 遇到HW上升沿后减速正向低速运行 → 遇到HW下降沿后反向反向低速运行 → 遇到HW上升沿后的第一个Z停机
 *          - 回零启动时HW=0，遇到正向限位：以正向高速开始回零 → 若遇到限位开关自动反向 → 反向高速运行 → 遇到HW上升沿后减速反向即恢复正向运行 → 正向低速遇到HW下降沿后反向反向低速运行 → 遇到HW上升沿后的第一个Z停机
 *          - 回零启动时HW=1：直接正向低速开始回零 → 遇到HW下降沿后反向反向低速运行 → 遇到HW上升沿后的第一个Z停机
 *        适用：有正限位、原点传感器和编码器Z相，需要最高精度
 *   模式10（6098h=10）：原点正向找Z相回零（限位保护+下降沿+Z相）
 *        机械原点：电机Z信号；减速点：原点开关（HW）
 *        动作流程：
 *          - 回零启动时HW=0，未遇到正向限位：以正向高速开始回零 → 遇到HW上升沿后减速正向低速运行 → 遇到HW下降沿后继续正向低速运行 → 之后遇到的第一个Z停机
 *          - 回零启动时HW=0，遇到正向限位：以正向高速开始回零 → 若遇到限位开关自动反向 → 反向高速运行 → 遇到HW上升沿后减速反向即恢复正向运行 → 正向低速遇到HW下降沿后的第一个Z停机
 *          - 回零启动时HW=1：直接正向低速开始回零 → 遇到HW下降沿后的第一个Z停机
 *        适用：有正限位、原点传感器和编码器Z相，不确定初始位置
 * 
 * 【模式11-14：原点+Z相回零（带限位保护，负向，6098h=11-14）】
 *   模式11-14与模式7-10运动曲线相似，仅初始运行方向相反（负向）
 *   模式11（6098h=11）：对应模式7，但初始方向为负向
 *   模式12（6098h=12）：对应模式8，但初始方向为负向
 *   模式13（6098h=13）：对应模式9，但初始方向为负向
 *   模式14（6098h=14）：对应模式10，但初始方向为负向
 *        适用：有负限位、原点传感器和编码器Z相，不确定初始位置或需要最高精度
 * 
 * 【模式17-30：原点回零（直接用原点传感器作为零点，不找Z相，6098h=17-30）】
 *   模式17-30与模式1-14运动曲线相同，仅最后一步找Z信号的步骤省去，遇到原点信号立即停机
 *   模式17（6098h=17）：对应模式1，但遇到限位下降沿后立即停机，不找Z相
 *   模式18（6098h=18）：对应模式2，但遇到限位下降沿后立即停机，不找Z相
 *   模式19（6098h=19）：对应模式3，但遇到原点下降沿后立即停机，不找Z相
 *   模式20（6098h=20）：对应模式4，但遇到原点上升沿后立即停机，不找Z相
 *   模式21（6098h=21）：对应模式5，但遇到原点下降沿后立即停机，不找Z相
 *   模式22（6098h=22）：对应模式6，但遇到原点上升沿后立即停机，不找Z相
 *   模式23（6098h=23）：对应模式7，但遇到原点下降沿后立即停机，不找Z相
 *   模式24（6098h=24）：对应模式8，但遇到原点上升沿后立即停机，不找Z相
 *   模式25（6098h=25）：对应模式9，但遇到原点上升沿后立即停机，不找Z相
 *   模式26（6098h=26）：对应模式10，但遇到原点下降沿后立即停机，不找Z相
 *   模式27（6098h=27）：对应模式11，但遇到原点下降沿后立即停机，不找Z相
 *   模式28（6098h=28）：对应模式12，但遇到原点上升沿后立即停机，不找Z相
 *   模式29（6098h=29）：对应模式13，但遇到原点上升沿后立即停机，不找Z相
 *   模式30（6098h=30）：对应模式14，但遇到原点下降沿后立即停机，不找Z相
 *        适用：有原点传感器或限位开关，速度要求快，精度要求不高（±传感器精度，通常±0.1mm）
 * 
 * 【模式31-32：扩展模式（6098h=31-32）】
 *   标准402协议中未定义此模式，可用于扩展
 * 
 * 【模式33-34：Z相回零（直接用编码器Z相作为零点，6098h=33-34）】
 *   机械原点：电机Z信号；减速点：无
 *   模式33（6098h=33）：反向低速运行，遇到的第一个Z信号停机
 *        动作：向负方向低速运动 → 找到第一个Z相 → 停止并设为零点
 *        适用：只有编码器Z相，精度高（±编码器分辨率），但需要确保初始位置不在Z相上
 *   模式34（6098h=34）：正向低速运行，遇到的第一个Z信号停机
 *        动作：向正方向低速运动 → 找到第一个Z相 → 停止并设为零点
 *        适用：只有编码器Z相，精度高（±编码器分辨率），但需要确保初始位置不在Z相上
 * 
 * 【模式35：当前位置回零（6098h=35）】
 *   模式35：以当前位置为机械原点
 *        动作：不进行任何运动，触发原点回零后（6040控制字：0x0F-0x1F），用户当前位置6064=607C（当前位置=目标位置）
 *        适用：已知当前位置准确，或手动定位后需要清零
 */
int MotionCardBase::getHomeMode(axis::HomeModeType homeType, int homeDir, int homeEdgeType)
{
	int mode = 0;

	// 以当前位置为零点，不进行运动
	if (homeType == axis::HomeModeType::Current_Position)
		mode = 35;	

	// ========== 情况1：原点回零（homeType == 0）==========
	// 使用原点传感器作为零点，不找Z相，速度快但精度较低（±传感器精度，通常±0.1mm）
	// homeEdgeType说明：
	//   0/1：无限位保护，单沿检测（0=下降沿，1=上升沿）
	//   2/3：有限位保护，单沿检测（2=下降沿，3=上升沿），如碰到限位开关则自动反转
	//   4/5：有限位保护，双沿检测（4=先上升沿后下降沿，最终以上升沿为准；5=先下降沿后上升沿，最终以下降沿为准），精度最高但耗时较长
	
	if (homeType == axis::HomeModeType::Origin_FindHome && homeDir == 0) {
		// 负向回零（向负方向运动找原点）
		if (homeEdgeType == 0) mode = 21;      // 模式21：原点负向回零，无限位保护，原点下降沿为零点（最常用，适用于NPN型传感器）
		else if (homeEdgeType == 1) mode = 22;  // 模式22：原点负向回零，无限位保护，原点上升沿为零点（适用于PNP型传感器）
		else if (homeEdgeType == 2) mode = 27;  // 模式27：原点负向回零，有限位保护，遇到负限位正转找原点信号，原点下降沿为零点
		else if (homeEdgeType == 3) mode = 28;  // 模式28：原点负向回零，有限位保护，遇到负限位正转找原点信号，原点上升沿为零点
		else if (homeEdgeType == 4) mode = 29;  // 模式29：原点负向回零，有限位保护+双沿检测，遇到负限位正转找原点，先找上升沿再找下降沿，最终以上升沿为原点（高精度）
		else if (homeEdgeType == 5) mode = 30;  // 模式30：原点负向回零，有限位保护+双沿检测，遇到负限位正转找原点，先找下降沿再找上升沿，最终以下降沿为原点（高精度）
	}
	else if (homeType == axis::HomeModeType::Origin_FindHome && homeDir == 1) {
		// 正向回零（向正方向运动找原点）
		if (homeEdgeType == 0) mode = 19;      // 模式19：原点正向回零，无限位保护，原点下降沿为零点（最常用，适用于NPN型传感器）
		else if (homeEdgeType == 1) mode = 20;  // 模式20：原点正向回零，无限位保护，原点上升沿为零点（适用于PNP型传感器）
		else if (homeEdgeType == 2) mode = 23;  // 模式23：原点正向回零，有限位保护，遇到正限位反转找原点信号，原点下降沿为零点
		else if (homeEdgeType == 3) mode = 24;  // 模式24：原点正向回零，有限位保护，遇到正限位反转找原点信号，原点上升沿为零点
		else if (homeEdgeType == 4) mode = 25;  // 模式25：原点正向回零，有限位保护+双沿检测，遇到正限位反转找原点，先找上升沿再找下降沿，最终以上升沿为原点（高精度）
		else if (homeEdgeType == 5) mode = 26;  // 模式26：原点正向回零，有限位保护+双沿检测，遇到正限位反转找原点，先找下降沿再找上升沿，最终以下降沿为原点（高精度）
	}
	
	// ========== 情况2：Z相回零（homeType == 1）==========
	// 使用编码器Z相信号作为零点，精度高（±编码器分辨率）但需要编码器支持，且需要确保初始位置不在Z相上
	else if (homeType == axis::HomeModeType::Z_FindHome && homeDir == 0)
		mode = 33;  // 模式33：负向找Z相回零（向负方向运动，找到第一个Z相就停止并设为零点）
	else if (homeType == axis::HomeModeType::Z_FindHome && homeDir == 1)
		mode = 34;  // 模式34：正向找Z相回零（向正方向运动，找到第一个Z相就停止并设为零点）
	
	// ========== 情况3：限位回零（homeType == 2）==========
	// 使用限位开关作为零点，精度最低（±限位开关精度，通常±1mm）但最简单，适用于精度要求不高的场合
	else if (homeType == axis::HomeModeType::Limit_FindHome && homeDir == 0)
		mode = 17;  // 模式17：负限位负向回零，负限位下降沿为零点（碰到负限位开关就停止并设为零点）
	else if (homeType == axis::HomeModeType::Limit_FindHome && homeDir == 1)
		mode = 18;  // 模式18：正限位正向回零，正限位下降沿为零点（碰到正限位开关就停止并设为零点）
	
	// ========== 情况4：原点+Z相回零（homeType == 3）==========
	// 先找原点传感器，再找Z相，精度最高（±编码器分辨率），适合高精度应用（如CNC、精密定位等）
	// 动作流程：找原点传感器 → 减速反向运动离开原点 → 找第一个Z相 → 停止并设为零点
	// homeEdgeType说明：
	//   0/1：无限位保护，单沿检测（0=下降沿，1=上升沿）
	//   2/3：有限位保护，单沿检测（2=下降沿，3=上升沿），如碰到限位开关则自动反转
	//   4/5：有限位保护，双沿检测（4=先上升沿后下降沿，最终以上升沿为准；5=先下降沿后上升沿，最终以下降沿为准），精度最高但耗时较长
	else if (homeType == axis::HomeModeType::Origin_Add_Z_FindHome && homeDir == 0) {
		// 负向回零（向负方向运动）
		if (homeEdgeType == 0) mode = 5;   // 模式5：原点负向找Z相，无限位保护，遇到原点上升沿时反向、减速正向运动，以离开原点下降沿的第一个Z相为零点
		else if (homeEdgeType == 1) mode = 6;   // 模式6：原点负向找Z相，无限位保护，遇到原点上升沿后第一个Z相为零点（不反向，直接找Z相）
		else if (homeEdgeType == 2) mode = 11;  // 模式11：原点负向找Z相，有限位保护，遇到负限位开关反转找原点，离开原点下降沿的第一个Z相为零点
		else if (homeEdgeType == 3) mode = 12;  // 模式12：原点负向找Z相，有限位保护，遇到负限位开关反转找原点，离开原点上升沿的第一个Z相为零点
		else if (homeEdgeType == 4) mode = 13;  // 模式13：原点负向找Z相，有限位保护+双沿检测，遇到负限位开关反转找原点，先找上升沿再找下降沿，最终以上升沿的第一个Z相为原点（最高精度）
		else if (homeEdgeType == 5) mode = 14;  // 模式14：原点负向找Z相，有限位保护+双沿检测，遇到负限位开关反转找原点，先找下降沿再找上升沿，最终以下降沿的第一个Z相为原点（最高精度）
	}
	else if (homeType == axis::HomeModeType::Origin_Add_Z_FindHome && homeDir == 1) {
		// 正向回零（向正方向运动）
		if (homeEdgeType == 0) mode = 3;   // 模式3：原点正向找Z相，无限位保护，遇到原点上升沿时反向、减速负向运动，以离开原点下降沿的第一个Z相为零点
		else if (homeEdgeType == 1) mode = 4;   // 模式4：原点正向找Z相，无限位保护，遇到原点上升沿的第一个Z相为零点（不反向，直接找Z相）
		else if (homeEdgeType == 2) mode = 7;   // 模式7：原点正向找Z相，有限位保护，遇到正限位开关反转找原点，遇到原点上升沿时减速继续运动，离开原点下降沿的第一个Z相为零点
		else if (homeEdgeType == 3) mode = 8;   // 模式8：原点正向找Z相，有限位保护，遇到正限位开关反转找原点，遇到原点上升沿时减速继续运动，遇到下降沿时反向、减速正向运动，以离开原点上升沿的第一个Z相为零点
		else if (homeEdgeType == 4) mode = 9;   // 模式9：原点正向找Z相，有限位保护+双沿检测，遇到正限位开关反转找原点，先找上升沿再找下降沿，最终以上升沿的第一个Z相为原点（最高精度）
		else if (homeEdgeType == 5) mode = 10;  // 模式10：原点正向找Z相，有限位保护+双沿检测，遇到正限位开关反转找原点，先找下降沿再找上升沿，最终以下降沿的第一个Z相为原点（最高精度）
	}
	
	// ========== 情况5：限位+Z相回零（homeType == 4）==========
	// 先找限位开关，再找Z相，精度较高（±编码器分辨率），适用于有限位开关和编码器的场合
	else if (homeType == axis::HomeModeType::Limit_Add_Z_FindHome && homeDir == 0)
		mode = 1;  // 模式1：负限位负向找Z相，遇到负限位开关反转找Z相，离开负限位的第一个Z相为零点
	else if (homeType == axis::HomeModeType::Limit_Add_Z_FindHome && homeDir == 1)
		mode = 2;  // 模式2：正限位正向找Z相，遇到正限位开关反转找Z相，离开正限位的第一个Z相为零点

	return mode;
		
}
int MotionCardBase::getPsoAction(common::device::axis::Axis::AxisPsoAction cmpAction)
{
	int action = 0;
	if (m_typeName_ == MotionCardDevType::LeiShineDmc || m_typeName_ == MotionCardDevType::LeiShineNmc)
	{
		switch (cmpAction)
		{
			case common::device::axis::Axis::AxisPsoAction::WriteIO_High:action = 1; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_Low:action = 2; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_Reverse:action = 3; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_500us:action = 5; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_1ms:action = 6; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_10ms:action = 7; break;
			case common::device::axis::Axis::AxisPsoAction::WriteIO_100ms:action = 8; break;
			case common::device::axis::Axis::AxisPsoAction::StopAxis:action = 13; break;
			case common::device::axis::Axis::AxisPsoAction::ChangeSpeed:action = 15; break;
			default:
			break;
		}
	}
	return action;
}
ErrorPtr inline MotionCardBase::checkError(long rtn, const std::string& strFun, const std::string& axisName)
{
	std::string rtnStr;
	std::string errStr = strFun;
	if (!axisName.empty())
		errStr = "[" + axisName + "]:" + errStr;

	if (makeError(rtn, errStr, rtnStr))
		return nullptr;
	return common::errors::Newf(common::errors::motion_module::kSdkError, "%s error: %s", strFun, MotionSdkDetail(rtn, rtnStr));
}


void MotionCardBase::reflectFromJson(const json& j)
{
	getFromJson(j, "type", m_typeName_);
	fromJson(j, "connOpts", m_deviceConnectOpts_);
}
void MotionCardBase::reflectToJson(json& j)const
{
	j["type"] = m_typeName_;
	toJson(j["connOpts"], m_deviceConnectOpts_);
}
void MotionCardBase::saveDataToJson(json& j)const
{
	toJson(j["Axis"], m_motionAxisMap_);
	toJson(j["DeviceIo"], m_motionIOMap_);
	toJson(j["DeviceAd"], m_motionADMap_);
}
bool MotionCardBase::addAxis(const std::string& moduleName, const std::string& key, std::shared_ptr<axis::Axis> value)
{
	bool rtn = false;
	if (moduleName == "Axis")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionAxisMutex_);
		if (m_motionAxisMap_.find(key) == m_motionAxisMap_.end()) {
			m_motionAxisMap_[key] = std::move(value);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is already existed", moduleName, key);
		}
	}
	return rtn;
}
bool MotionCardBase::addDeviceIO(const std::string& moduleName, const std::string& key, std::shared_ptr<deviceIO::DeviceIO> value)
{
	bool rtn = false;
	if (moduleName == "DeviceIO")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionIOMutex_);
		if (m_motionIOMap_.find(key) == m_motionIOMap_.end()) {
			m_motionIOMap_[key] = std::move(value);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is already existed", moduleName, key);
		}
	}
	return rtn;
}
bool MotionCardBase::addDeviceAD(const std::string& moduleName, const std::string& key, std::shared_ptr<deviceAD::DeviceAD> value)
{
	bool rtn = false;
	if (moduleName == "DeviceAD")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionADMutex_);
		if (m_motionADMap_.find(key) == m_motionADMap_.end()) {
			m_motionADMap_[key] = std::move(value);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is already existed", moduleName, key);
		}
	}
	return rtn;
}
bool MotionCardBase::remove(const std::string& moduleName, const std::string& key)
{
	bool rtn = false;
	if (moduleName == "Axis")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionAxisMutex_);
		auto it = m_motionAxisMap_.find(key);
		if (it != m_motionAxisMap_.end())
		{
			m_motionAxisMap_.erase(it);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is inexistence", moduleName, key);
		}
	}
	else if (moduleName == "DeviceIO")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionIOMutex_);
		auto it = m_motionIOMap_.find(key);
		if (it != m_motionIOMap_.end())
		{
			m_motionIOMap_.erase(it);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is inexistence", moduleName, key);
		}
	}
	else if (moduleName == "DeviceAD")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionADMutex_);
		auto it = m_motionADMap_.find(key);
		if (it != m_motionADMap_.end())
		{
			m_motionADMap_.erase(it);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]---{} failed, name={} is inexistence", moduleName, key);
		}
	}
	return rtn;
}
bool MotionCardBase::getAxisParams(const std::string& axisName, axis::Axis::AxisData& axisData)
{
	std::shared_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		axisData = it->second.get()->getAxisParams();
		return true;
	}
	return false;
}
bool MotionCardBase::getMoveParameters(const std::string& axisName, axis::Axis::MoveParameters& axisMoveData)
{
	std::shared_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		axisMoveData = it->second.get()->getMoveParameters();
		return true;
	}
	return false;
}
bool MotionCardBase::setAxisParams(const std::string& axisName, const axis::Axis::AxisData& axisData)
{
	std::unique_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		it->second.get()->setAxisParams(axisData);
		return true;
	}
	return false;
}
bool MotionCardBase::setMoveParameters(const std::string& axisName, const axis::Axis::MoveParameters& axisMoveData)
{
	std::unique_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		it->second.get()->setMoveParameters(axisMoveData);
		return true;
	}
	return false;
}
bool MotionCardBase::getAxisProcessData(const std::string& axisName, axis::Axis::AxisProcessData& processData)
{
	std::shared_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		processData = it->second.get()->getAxisProcessData();
		return true;
	}
	return false;
}
bool MotionCardBase::setAxisProcessData(const std::string& axisName, const axis::Axis::AxisProcessData& processData)
{
	std::unique_lock<std::shared_mutex> lck(m_motionAxisMutex_);
	auto it = m_motionAxisMap_.find(axisName);
	if (it != m_motionAxisMap_.end()) {
		it->second.get()->setAxisProcessData(processData);
		return true;
	}
	return false;
}
bool MotionCardBase::getIoParams(const std::string& ioName, deviceIO::DeviceIO::DeviceIoData& ioData)
{
	std::shared_lock<std::shared_mutex> lck(m_motionIOMutex_);
	auto it = m_motionIOMap_.find(ioName);
	if (it != m_motionIOMap_.end()) {
		ioData = it->second.get()->getDeviceIoData();
		return true;
	}
	return false;
}
bool MotionCardBase::setIoParams(const std::string& ioName, const deviceIO::DeviceIO::DeviceIoData& ioData)
{
	std::unique_lock<std::shared_mutex> lck(m_motionIOMutex_);
	auto it = m_motionIOMap_.find(ioName);
	if (it != m_motionIOMap_.end()) {
		it->second.get()->setDeviceIoData(ioData);
		return true;
	}
	return false;
}
bool MotionCardBase::getAdParams(const std::string& adName, deviceAD::DeviceAD::DeviceAdData& adData)
{
	std::shared_lock<std::shared_mutex> lck(m_motionADMutex_);
	auto it = m_motionADMap_.find(adName);
	if (it != m_motionADMap_.end()) {
		adData = it->second.get()->getDeviceAdData();
		return true;
	}
	return false;
}
bool MotionCardBase::setAdParams(const std::string& adName, const deviceAD::DeviceAD::DeviceAdData& adData)
{
	std::unique_lock<std::shared_mutex> lck(m_motionADMutex_);
	auto it = m_motionADMap_.find(adName);
	if (it != m_motionADMap_.end()) {
		it->second.get()->setDeviceAdData(adData);
		return true;
	}
	return false;
}
}; // end of namespace motion

} // end of namespace device

}
