#ifndef _MOTION_CONTROLLER_HH
#define _MOTION_CONTROLLER_HH

#include <map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <common/utility/json_tool/reflector.h>
#include "motion_control_global.h"
#include "motion_control_impl/motion_card_base.h"
#include <common/utility/errors/include/error.h>

namespace common
{
namespace device
{

using namespace common::errors;

class MOTION_CONTROL_EXPORT MotionController
{
public:
	MotionController() = default;
	~MotionController() = default;

	ErrorPtr InitDevice();
	void Release();
	// 获取总轴数
	ErrorPtr TotalAxis(uint32_t& axisNum);
	// 轴使能接口
	ErrorPtr AxisEnable(const std::string& axisName, bool enable);
	// 判断轴是否使能接口
	ErrorPtr AxisIsEnable(const std::string& axisName, bool& enable);
	// 轴定长运动控制接口  
	ErrorPtr AxisMove(const std::string& axisName, const common::device::motion::AxisMoveParams& params);
	// 多轴直线插补运动控制接口  
	ErrorPtr AxisLinear(const std::vector<std::string>& axisNames, const common::device::motion::AxisLinearParams& params);
	// 轴连续运动控制接口, 
	ErrorPtr AxisVmove(const std::string& axisName, uint16_t dir, double velRatio = 0.1);
	//等待运动完成
	ErrorPtr AxisWaitMoveDone(const std::string& axisName, int timeout = 30000);
	// 轴停止接口 mode: 0减速停止, 1立刻停止
	ErrorPtr AxisStop(const std::string& axisName, uint8_t mode = 1);
	// 轴急停接口
	ErrorPtr AxisEmgStop();
	// 轴复位(回原点)接口
	ErrorPtr AxisReset(const std::string& axisName, int timeout = 50000);
	// 轴告警清除接口
	ErrorPtr AxisAlarmClear(const std::string& axisName);
	// 轴告警清除接口 清除所有轴告警
	ErrorPtr AxisAlarmClear();
	// 读取轴有关运动的信号状态接口
	ErrorPtr AxisReadStatus(const std::string& axisName, common::device::axis::Axis::AxisSta& axisStatus);
	// 获取编码器位置
	ErrorPtr AxisGetEncoder(const std::string& axisName, double& value);
	// 获取命令位置
	ErrorPtr AxisGetPosition(const std::string& axisName, double& value);
	// 设置编码器和指令位置
	ErrorPtr AxisSetEncoderAndPosition(const std::string& axisName, double value = 0.0);
	// 设置软限位 
	ErrorPtr AxisSetSoftlimit(const std::string& axisName, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit);
	// 锁存功能
	ErrorPtr AxisEnableLtc(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc);
	ErrorPtr AxisGetLtcPos(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos);
	// 单轴定长 + 位置比较（多点队列，工程单位）
	ErrorPtr AxisMoveWithPositionCompare(const std::string& axisName,
		const common::device::motion::AxisMoveWithPositionCompareParams& params);
	// 直线插补 + 位置比较触发（雷赛 smc_compare_* / smc_hcmp_* 队列模式）
	ErrorPtr AxisLinearWithPositionCompare(const common::device::motion::LinearPositionCompareParams& params);
	// 禁用位置比较输出
	ErrorPtr DisablePositionCompare(const common::device::motion::PositionCompareDisableParams& params);
	// IO写操作
	ErrorPtr WriteIO(const std::string& ioName, bool value);
	// IO读操作
	ErrorPtr ReadIO(const std::string& ioName, bool& value);
	// DO 端口批量读（按卡名称路由，vecValue 每个元素对应一个端口）
	ErrorPtr ReadDoPorts(const std::string& cardName, std::vector<uint32_t>& vecValue);
	// DI 端口批量读（按卡名称路由，vecValue 每个元素对应一个端口）
	ErrorPtr ReadDiPorts(const std::string& cardName, std::vector<uint32_t>& vecValue);
	// AD写操作
	ErrorPtr WriteAD(const std::string& adName, double value);
	// AD读操作
	ErrorPtr ReadAD(const std::string& adName, double& value);

	// 获取指定运动控制卡连接状态
	bool GetCardConnectedState(const std::string& cardName);
	// 获取所有卡名称列表
	std::vector<std::string> getAllCardName();
	// 获取所有轴名列表
	std::vector<std::string> getAllAxisName();
	// 获取所有IO名称列表
	std::vector<std::string> getAllIoName();
	// 获取所有AD名称列表
	std::vector<std::string> getAllAdName();
	// 获取指定轴参数
	common::device::axis::Axis::AxisData getAxisParams(const std::string& axisName);
	common::device::axis::Axis::MoveParameters getAxisMoveParams(const std::string& axisName);
	// 设置指定轴参数
	void setAxisParams(const std::string& axisName, common::device::axis::Axis::AxisData& axisData);
	void setAxisMoveParams(const std::string& axisName, common::device::axis::Axis::MoveParameters& moveParameters);
	// 获取指定IO参数
	common::device::deviceIO::DeviceIO::DeviceIoData getDeviceIoData(const std::string& ioName);
	// 设置指定IO参数
	void setDeviceIoData(const std::string& ioName, common::device::deviceIO::DeviceIO::DeviceIoData& deviceIoData);
	// 获取指定AD参数
	common::device::deviceAD::DeviceAD::DeviceAdData getDeviceAdData(const std::string& adName);
	// 设置指定AD参数
	void setDeviceAdData(const std::string& adName, common::device::deviceAD::DeviceAD::DeviceAdData& deviceAdData);

	void reflectFromJson(const json& j);
	void reflectToJson(json& j)const;
	bool addDevice(const std::string& moduleName, const std::string& key, common::device::motion::MotionCardDevType type, const std::string& cardName = "");
	bool removeDevice(const std::string& moduleName, const std::string& key);

private:
	// 更新运动控制卡连接状态，状态变化时通过 MessageCenter 通知
	void SetCardConnectedState(const std::string& cardName, bool connected);
	std::unique_ptr<common::device::motion::MotionCardBase> createInstance(const std::string& key, common::device::motion::MotionCardDevType type);
	const std::shared_ptr<common::device::motion::MotionCardBase> getMotionCardHandler(const std::string& key) const;
	const std::shared_ptr<common::device::motion::MotionCardBase> getMotionCardHandlerByAxis(const std::string& key)const;
	const std::shared_ptr<common::device::motion::MotionCardBase> getMotionCardHandlerByIO(const std::string& key)const;
	const std::shared_ptr<common::device::motion::MotionCardBase> getMotionCardHandlerByAD(const std::string& key)const;
	bool setAxisToCard(std::unordered_map<std::string, std::shared_ptr<common::device::axis::Axis>>& axisDataMap);
	bool setIoToCard(std::unordered_map<std::string, std::shared_ptr<deviceIO::DeviceIO>>& ioDataMap);
	bool setAdToCard(std::unordered_map<std::string, std::shared_ptr<deviceAD::DeviceAD>>& adDataMap);

	std::unordered_map<std::string, std::shared_ptr<common::device::motion::MotionCardBase>> m_motionCardPool_;  // 控制卡连接池
	std::unordered_map<std::string, std::string> axisToCardMap_;		//每个轴对应的卡名称
	std::unordered_map<std::string, std::string> ioToCardMap_;			//每个IO对应的卡名称
	std::unordered_map<std::string, std::string> adToCardMap_;			//每个AD对应的卡名称
	mutable std::shared_mutex m_motionCardMutex_;
	mutable std::shared_mutex m_axisToCardMutex_;
	mutable std::shared_mutex m_ioToCardMutex_;
	mutable std::shared_mutex m_adToCardMutex_;

	std::unordered_map<std::string, bool> cardConnectedState_;
}; 

}// end of namespace device

}
#endif //_MOTION_CONTROLLER_HH
