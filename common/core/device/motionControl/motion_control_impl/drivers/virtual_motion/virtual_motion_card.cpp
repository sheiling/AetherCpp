#include "virtual_motion_card.h"

#include <chrono>
#include <cmath>
#include <thread>
#include <Logger.h>

namespace common::device::motion {

namespace {
constexpr auto kVmoveSimInterval = std::chrono::milliseconds(200);
}

VirtualMotionCard::~VirtualMotionCard()
{
	stopSimThread();
}

void VirtualMotionCard::startSimThread()
{
	if (m_simRunning.exchange(true))
		return;

	m_simThread = std::thread([this]() {
		while (m_simRunning.load(std::memory_order_relaxed)) {
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto& item : m_axes)
					advanceVmoveUnlocked(item.second);
			}
			std::this_thread::sleep_for(kVmoveSimInterval);
		}
	});
}

void VirtualMotionCard::stopSimThread()
{
	if (!m_simRunning.exchange(false))
		return;
	if (m_simThread.joinable())
		m_simThread.join();
}

VirtualMotionCard::SimAxisState& VirtualMotionCard::axisState(int axisNo)
{
	return m_axes[axisNo];
}

void VirtualMotionCard::advanceVmoveUnlocked(SimAxisState& ax)
{
	if (!ax.vMoving)
		return;

	const auto now = std::chrono::steady_clock::now();
	const double dt = std::chrono::duration<double>(now - ax.vMoveLastUpdate).count();
	ax.vMoveLastUpdate = now;
	if (dt <= 0.0)
		return;

	const double speed = 100.0;
	const double dir   = ax.vMoveDir ? 1.0 : -1.0;
	ax.cmdPos += speed * dt * dir;
	ax.encPos = ax.cmdPos;
}

long VirtualMotionCard::initDevice()
{
	stopSimThread();
	std::lock_guard<std::mutex> lock(m_mutex);
	m_axes.clear();
	m_localIo.clear();
	m_adValues.clear();
	m_crdMoving = false;
	startSimThread();
	LOG_INFO("[VirtualMotionCard] initialized (virtual_debug.virtual_motion_enable)");
	return 0;
}

void VirtualMotionCard::release()
{
	stopSimThread();
	std::lock_guard<std::mutex> lock(m_mutex);
	m_axes.clear();
	m_localIo.clear();
	m_adValues.clear();
	m_crdMoving = false;
}

bool VirtualMotionCard::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	if (rtn == 0)
		return true;
	rtnStr = "virtual_motion rtn=" + std::to_string(rtn);
	LOG_ERROR("[VirtualMotionCard] {} failed: {}", strFun, rtnStr);
	return false;
}

long VirtualMotionCard::initAxis(int axisNo)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo);
	return 0;
}

long VirtualMotionCard::totalAxis(uint32_t& axisNum)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisNum = static_cast<uint32_t>(m_axes.size());
	return 0;
}

long VirtualMotionCard::axisSetVel(int axisNo, const AxisVelocityProfile& profile)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo).profile = profile;
	return 0;
}

long VirtualMotionCard::crdSetVel(int crdNo, const AxisVelocityProfile& profile)
{
	(void)crdNo;
	(void)profile;
	return 0;
}

long VirtualMotionCard::axisSetSMode(int axisNo, int s_mode, double s_para)
{
	(void)axisNo;
	(void)s_mode;
	(void)s_para;
	return 0;
}

long VirtualMotionCard::axisSingleMove(int axisNo, const AxisSingleMoveParams& params)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	advanceVmoveUnlocked(ax);
	ax.vMoving = false;
	ax.profile = params.profile;
	ax.moving = true;
	const double target = params.posiMode == 1 ? params.dist : ax.cmdPos + params.dist;
	ax.cmdPos = target;
	ax.encPos = target;
	ax.moving = false;
	return 0;
}

long VirtualMotionCard::axisVMove(int axisNo, uint16_t dir)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	if (ax.vMoving)
		advanceVmoveUnlocked(ax);
	ax.vMoveLastUpdate = std::chrono::steady_clock::now();
	ax.vMoving  = true;
	ax.moving   = true;
	ax.vMoveDir = dir;
	return 0;
}

long VirtualMotionCard::axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode)
{
	(void)crdNo;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_crdMoving = true;
	for (size_t i = 0; i < axisNos.size() && i < dists.size(); ++i) {
		auto& ax = axisState(axisNos[i]);
		const double target = posiMode == 1 ? dists[i] : ax.cmdPos + dists[i];
		ax.cmdPos = target;
		ax.encPos = target;
	}
	m_crdMoving = false;
	return 0;
}

bool VirtualMotionCard::axisIsMoveDone(int axisNo)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return !axisState(axisNo).moving;
}

bool VirtualMotionCard::crdIsMoveDone(int crdNo)
{
	(void)crdNo;
	std::lock_guard<std::mutex> lock(m_mutex);
	return !m_crdMoving;
}

long VirtualMotionCard::axisStop(int axisNo, uint8_t mode)
{
	(void)mode;
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	advanceVmoveUnlocked(ax);
	ax.vMoving = false;
	ax.moving  = false;
	return 0;
}

long VirtualMotionCard::axisEmgStop()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto& item : m_axes) {
		advanceVmoveUnlocked(item.second);
		item.second.vMoving = false;
		item.second.moving  = false;
	}
	m_crdMoving = false;
	return 0;
}

long VirtualMotionCard::axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome)
{
	(void)axisNo;
	(void)axisHome;
	return 0;
}

long VirtualMotionCard::axisStartHome(int axisNo)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo).homing = true;
	axisState(axisNo).cmdPos = 0.0;
	axisState(axisNo).encPos = 0.0;
	axisState(axisNo).homing = false;
	return 0;
}

bool VirtualMotionCard::axisIsHomeDone(int axisNo)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return !axisState(axisNo).homing;
}

long VirtualMotionCard::axisAlarmClear(int axisNo)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo).alarm = false;
	return 0;
}

long VirtualMotionCard::axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	advanceVmoveUnlocked(ax);
	axisStatus = {};
	axisStatus.ALM = ax.alarm;
	axisStatus.RDY = ax.enabled;
	axisStatus.INP = !ax.moving;
	axisStatus.RUN_MODE = ax.vMoving ? 2 : 0;
	return 0;
}

long VirtualMotionCard::axisGetEncoder(int axisNo, double& value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	advanceVmoveUnlocked(ax);
	value = ax.encPos;
	return 0;
}

long VirtualMotionCard::axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value)
{
	(void)slaveId;
	(void)driverType;
	(void)encoderType;
	return axisGetEncoder(axisNo, value);
}

long VirtualMotionCard::axisGetPosition(int axisNo, double& value)
{
	return axisGetEncoder(axisNo, value);
}

long VirtualMotionCard::axisSetEncoderAndPosition(int axisNo, double value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	ax.cmdPos = value;
	ax.encPos = value;
	return 0;
}

long VirtualMotionCard::axisCheckSuccessEncoder(int axisNo, double bandRange)
{
	(void)bandRange;
	std::lock_guard<std::mutex> lock(m_mutex);
	const auto& ax = axisState(axisNo);
	return std::abs(ax.encPos - ax.cmdPos) < bandRange ? 0 : 1;
}

long VirtualMotionCard::axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	(void)axisNo;
	(void)axisSoftlimit;
	return 0;
}

long VirtualMotionCard::axisSetEquiv(int axisNo, double equiv)
{
	(void)axisNo;
	(void)equiv;
	return 0;
}

long VirtualMotionCard::axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic)
{
	(void)axisNo;
	(void)axisStaLogic;
	return 0;
}

long VirtualMotionCard::axisSetFactorError(int axisNo, double factorError)
{
	(void)axisNo;
	(void)factorError;
	return 0;
}

long VirtualMotionCard::axisEnable(int axisNo, bool enable)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo).enabled = enable;
	return 0;
}

long VirtualMotionCard::axisIsEnable(int axisNo, bool& enable)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	enable = axisState(axisNo).enabled;
	return 0;
}

long VirtualMotionCard::axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	(void)axisNo;
	(void)axisLtc;
	return 0;
}

long VirtualMotionCard::axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	(void)axisLtc;
	vecPos.clear();
	double pos = 0.0;
	const long rtn = axisGetEncoder(axisNo, pos);
	if (rtn == 0)
		vecPos.push_back(pos);
	return rtn;
}

long VirtualMotionCard::positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& ax = axisState(axisNo);
	ax.comparePoints = setup.positions;
	ax.compareHighSpeed = setup.highSpeed;
	LOG_INFO("[VirtualMotionCard] positionCompareSetup axis={} points={} highSpeed={}",
		axisNo, setup.positions.size(), setup.highSpeed);
	return 0;
}

long VirtualMotionCard::positionCompareDisable(int axisNo, int channelNo, bool highSpeed)
{
	(void)channelNo;
	(void)highSpeed;
	std::lock_guard<std::mutex> lock(m_mutex);
	axisState(axisNo).comparePoints.clear();
	return 0;
}

long VirtualMotionCard::writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}

long VirtualMotionCard::readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}

long VirtualMotionCard::writeIO(int bitNo, bool value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_localIo[bitNo] = value;
	return 0;
}

long VirtualMotionCard::readIO(int bitNo, bool& value, bool isInputIo)
{
	(void)isInputIo;
	std::lock_guard<std::mutex> lock(m_mutex);
	value = m_localIo.count(bitNo) ? m_localIo[bitNo] : false;
	return 0;
}

long VirtualMotionCard::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	return writeIO(bitNo, value);
}

long VirtualMotionCard::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	(void)nodeNo;
	return readIO(bitNo, value, isInputIo);
}

long VirtualMotionCard::writeAD(int nodeNo, int channelNo, double value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	const uint64_t key = (static_cast<uint64_t>(nodeNo) << 32) | static_cast<uint32_t>(channelNo);
	m_adValues[key] = value;
	return 0;
}

long VirtualMotionCard::readAD(int nodeNo, int channelNo, double& value, bool isInputAd)
{
	(void)isInputAd;
	std::lock_guard<std::mutex> lock(m_mutex);
	const uint64_t key = (static_cast<uint64_t>(nodeNo) << 32) | static_cast<uint32_t>(channelNo);
	value = m_adValues.count(key) ? m_adValues[key] : 0.0;
	return 0;
}

} // namespace common::device::motion
