#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CARD_BASE_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CARD_BASE_H

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include <Logger.h>
#include <common/utility/errors/include/error.h>

#include "motion_types.h"
#include "drivers/motion_capabilities.h"
#include "drivers/imotion_device_lifecycle.h"
#include "drivers/iaxis_driver.h"
#include "drivers/iio_driver.h"
#include "drivers/iad_driver.h"

namespace common {
namespace device {
namespace motion {

using namespace common::errors;

class MotionCardBase {
public:
	MotionCardBase() : m_inited(false) {}
	virtual ~MotionCardBase() {}

	ErrorPtr InitDevice();
	void Release();
	ErrorPtr InitAxis();
	ErrorPtr TotalAxis(uint32_t& axisNum);
	ErrorPtr AxisEnable(const std::string& axisName, bool enable);
	ErrorPtr AxisIsEnable(const std::string& axisName, bool& enable);
	ErrorPtr AxisMove(const std::string& axisName, const AxisMoveParams& params);
	ErrorPtr AxisLinear(const std::vector<std::string>& axisNames, const AxisLinearParams& params);
	ErrorPtr AxisVmove(const std::string& axisName, uint16_t dir, double velRatio = 0.1);
	ErrorPtr AxisWaitMoveDone(const std::string& axisName, int timeout = 30000);
	ErrorPtr AxisStop(const std::string& axisName, uint8_t mode = 0);
	ErrorPtr AxisEmgStop();
	ErrorPtr AxisReset(const std::string& axisName, int timeout = 50000);
	ErrorPtr AxisAlarmClear(const std::string& axisName);
	ErrorPtr AxisReadStatus(const std::string& axisName, common::device::axis::Axis::AxisSta& axisStatus);
	ErrorPtr AxisGetEncoder(const std::string& axisName, double& value);
	ErrorPtr AxisSetEncoderAndPosition(const std::string& axisName, double value = 0.0);
	ErrorPtr AxisGetPosition(const std::string& axisName, double& value);
	ErrorPtr AxisSetSoftlimit(const std::string& axisName, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit);
	ErrorPtr AxisEnableLtc(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc);
	ErrorPtr AxisGetLtcPos(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos);
	// 配置位置比较（多点队列），轴名单独传入
	ErrorPtr SetupPositionCompare(const std::string& compareAxisName, const PositionCompareDriverSetup& setup);
	// 禁用位置比较
	ErrorPtr DisablePositionCompare(const PositionCompareDisableParams& params);
	// 单轴定长运动 + 位置比较触发
	ErrorPtr AxisMoveWithPositionCompare(const std::string& axisName, const AxisMoveWithPositionCompareParams& params);
	// 直线插补运动并触发位置比较（起始/终点 + 比较点 + 输出 IO 可配置）
	ErrorPtr AxisLinearWithPositionCompare(const LinearPositionCompareParams& params);
	ErrorPtr WriteIO(const std::string& ioName, bool value);
	ErrorPtr ReadIO(const std::string& ioName, bool& value);
	ErrorPtr ReadDoPorts(std::vector<uint32_t>& vecValue);
	ErrorPtr ReadDiPorts(std::vector<uint32_t>& vecValue);
	ErrorPtr WriteAD(const std::string& adName, double value);
	ErrorPtr ReadAD(const std::string& adName, double& value);

	virtual MotionCapabilities capabilities() const { return {}; }

	virtual IMotionDeviceLifecycle* lifecycleDriver() { return nullptr; }
	virtual IAxisDriver* axisDriver() { return nullptr; }
	virtual IIoDriver* ioDriver() { return nullptr; }
	virtual IAdDriver* adDriver() { return nullptr; }

	void reflectFromJson(const json& j);
	void reflectToJson(json& j) const;
	void saveDataToJson(json& j) const;
	bool addAxis(const std::string& moduleName, const std::string& key, std::shared_ptr<axis::Axis> value);
	bool addDeviceIO(const std::string& moduleName, const std::string& key, std::shared_ptr<deviceIO::DeviceIO> value);
	bool addDeviceAD(const std::string& moduleName, const std::string& key, std::shared_ptr<deviceAD::DeviceAD> value);
	bool remove(const std::string& moduleName, const std::string& key);
	bool getAxisParams(const std::string& axisName, axis::Axis::AxisData& axisData);
	bool getMoveParameters(const std::string& axisName, axis::Axis::MoveParameters& axisMoveData);
	bool getAxisProcessData(const std::string& axisName, axis::Axis::AxisProcessData& processData);
	bool setAxisParams(const std::string& axisName, const axis::Axis::AxisData& axisData);
	bool setMoveParameters(const std::string& axisName, const axis::Axis::MoveParameters& axisMoveData);
	bool setAxisProcessData(const std::string& axisName, const axis::Axis::AxisProcessData& processData);
	bool getIoParams(const std::string& ioName, deviceIO::DeviceIO::DeviceIoData& ioData);
	bool setIoParams(const std::string& ioName, const deviceIO::DeviceIO::DeviceIoData& ioData);
	bool getAdParams(const std::string& adName, deviceAD::DeviceAD::DeviceAdData& adData);
	bool setAdParams(const std::string& adName, const deviceAD::DeviceAD::DeviceAdData& adData);

protected:
	int getHomeMode(axis::HomeModeType homeType, int homeDir, int homeEdgeType = 3);
	int getPsoAction(common::device::axis::Axis::AxisPsoAction cmpAction);
	ErrorPtr inline checkError(long rtn, const std::string& strFun, const std::string& axisName = "");

	long initDevice();
	void release();
	long initAxis(int axisNo);
	long totalAxis(uint32_t& axisNum);
	long axisSetVel(int axisNo, const AxisVelocityProfile& profile);
	long crdSetVel(int crdNo, const AxisVelocityProfile& profile);
	long axisSetSMode(int axisNo, int s_mode, double s_para);
	long axisSingleMove(int axisNo, const AxisSingleMoveParams& params);
	long axisVMove(int axisNo, uint16_t dir);
	long axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode);
	bool axisIsMoveDone(int axisNo);
	bool crdIsMoveDone(int crdNo);
	long axisStop(int axisNo, uint8_t mode = 1);
	long axisEmgStop();
	long axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome);
	long axisStartHome(int axisNo);
	bool axisIsHomeDone(int axisNo);
	long axisAlarmClear(int axisNo);
	long axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus);
	long axisGetEncoder(int axisNo, double& value);
	long axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value);
	long axisGetPosition(int axisNo, double& value);
	long axisSetEncoderAndPosition(int axisNo, double value);
	long axisCheckSuccessEncoder(int axisNo, double bandRange = 1.0);
	long axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit);
	long axisSetEquiv(int axisNo, double equiv);
	long axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic);
	long axisSetFactorError(int axisNo, double factorError);
	long axisEnable(int axisNo, bool enable);
	long axisIsEnable(int axisNo, bool& enable);
	long axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc);
	long axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos);
	long positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup);
	long positionCompareDisable(int axisNo, int channelNo, bool highSpeed);
	long writeIO(int bitNo, bool value);
	long readIO(int bitNo, bool& value, bool isInputIo = false);
	long readDoPorts(std::vector<uint32_t>& vecValue);
	long readDiPorts(std::vector<uint32_t>& vecValue);
	long writeExtendIO(int nodeNo, int bitNo, bool value);
	long readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo = false);
	long writeAD(int nodeNo, int channelNo, double value);
	long readAD(int nodeNo, int channelNo, double& value, bool isInputAd = false);
	long writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value);
	long readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value);
	bool makeError(long rtn, const std::string& strFun, std::string& rtnStr);

	bool m_inited;
	int m_cardNo;
	DeviceConnectOpts m_deviceConnectOpts_;
	MotionCardDevType m_typeName_;
	std::unordered_map<std::string, std::shared_ptr<axis::Axis>> m_motionAxisMap_;
	std::unordered_map<std::string, std::shared_ptr<deviceIO::DeviceIO>> m_motionIOMap_;
	std::unordered_map<std::string, std::shared_ptr<deviceAD::DeviceAD>> m_motionADMap_;
	mutable std::shared_mutex m_motionAxisMutex_;
	mutable std::shared_mutex m_motionIOMutex_;
	mutable std::shared_mutex m_motionADMutex_;
};

} // namespace motion
} // namespace device
} // namespace common

#endif
