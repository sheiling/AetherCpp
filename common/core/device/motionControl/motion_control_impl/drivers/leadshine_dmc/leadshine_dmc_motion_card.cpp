#include "leadshine_dmc_motion_card.h"
#include "../../motion_sdk_err_map.h"
#include <common/3rdparty/device/motion/Leadshine_DMC/include/LTDMC.h>
#include <common/include/macros.h>
#include <Logger.h>

namespace common
{
namespace device
{
namespace motion
{
int LeadshineDmcMotionCard::m_cardNum = 0;

long LeadshineDmcMotionCard::initDevice()
{
	long iret = -1;
	if (m_cardNum <= 0)
	{
		//没有找到卡的时候才进入初始化
		m_cardNum = dmc_board_init();
		if (m_cardNum > 0 && m_cardNo < m_cardNum)
		{
			iret = 0;
		}
	}
	else if (m_cardNo < m_cardNum)//已经找到卡的时候判断当前卡号是否有效
	{
		iret = 0;
	}
	if (iret == 0)
	{
		//有配置文件时  下载配置到卡
		if (!m_deviceConnectOpts_.m_configfile.empty())
		{
			iret = dmc_download_configfile(m_cardNo, m_deviceConnectOpts_.m_configfile.c_str());
			LOG_INFO("[common][device][motion]--dmc_download_configfile rtn={}, m_cardNo={}, configfile={}", iret, m_cardNo, m_deviceConnectOpts_.m_configfile);

		}
	}

	return iret;
}
void LeadshineDmcMotionCard::release()
{
	dmc_board_close();
}
long LeadshineDmcMotionCard::initAxis(int axisNo)
{
	return 0;
}
long LeadshineDmcMotionCard::totalAxis(uint32_t& axisNum)
{
	unsigned long num;
	long iret = dmc_get_total_axes(m_cardNo, &num);
	axisNum = num;
	return iret;
}
long LeadshineDmcMotionCard::axisSetVel(int axisNo, const AxisVelocityProfile& profile)
{
	return dmc_set_profile_unit(m_cardNo, axisNo, profile.startVel, profile.maxVel, profile.tacc, profile.tdec, profile.endVel);
}
long LeadshineDmcMotionCard::crdSetVel(int crdNo, const AxisVelocityProfile& profile)
{
	return dmc_set_vector_profile_unit(m_cardNo, crdNo, profile.startVel, profile.maxVel, profile.tacc, profile.tdec, profile.endVel);
}
long LeadshineDmcMotionCard::axisSetSMode(int axisNo, int s_mode, double s_para)
{
	return dmc_set_s_profile(m_cardNo, axisNo, s_mode, s_para);
}
long LeadshineDmcMotionCard::axisSingleMove(int axisNo, const AxisSingleMoveParams& params)
{
	CHECK_RTN_LONG(axisSetVel(axisNo, params.profile));

	return dmc_pmove_unit(m_cardNo, axisNo, params.dist, params.posiMode);
}
long LeadshineDmcMotionCard::axisVMove(int axisNo, uint16_t dir)
{
	return dmc_vmove(m_cardNo, axisNo, dir);
}
long LeadshineDmcMotionCard::axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode)
{
	const int axisNum = static_cast<int>(axisNos.size());
	WORD* axisList = new WORD[axisNum];
	double* posList = new double[axisNum];
	for (unsigned int i = 0; i < axisNos.size(); i++)
	{
		axisList[i] = axisNos[i];
		posList[i] = dists[i];
	}
	long iret = dmc_line_unit(m_cardNo, crdNo, axisNum, axisList, posList, posiMode);

	delete[] axisList;
	delete[] posList;
	return iret;
}
bool LeadshineDmcMotionCard::axisIsMoveDone(int axisNo)
{
	short doneStatus = dmc_check_done(m_cardNo, axisNo);
	return doneStatus != 0;
}
bool LeadshineDmcMotionCard::crdIsMoveDone(int crdNo)
{
	short doneStatus = dmc_check_done_multicoor(m_cardNo, crdNo);
	return doneStatus != 0;
}
long LeadshineDmcMotionCard::axisStop(int axisNo, uint8_t mode)
{
	return dmc_stop(m_cardNo, axisNo, mode);
}
long LeadshineDmcMotionCard::axisEmgStop()
{
	return dmc_emg_stop(m_cardNo);
}
long LeadshineDmcMotionCard::axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome)
{
	int mode = 4;							//回零模式
	if (axisHome.m_homeMode == axis::HomeModeType::Origin_FindHome)        			mode = 6;				//原点锁存
	else if (axisHome.m_homeMode == axis::HomeModeType::Origin_Add_Z_FindHome)   	mode = 9;		     	//原点锁存加反向 EZ 锁存
	else if (axisHome.m_homeMode == axis::HomeModeType::Z_FindHome)   				mode = 8;		   		//单独记一个 EZ 锁存
	else if (axisHome.m_homeMode == axis::HomeModeType::Limit_FindHome)            	mode = 11;		      	//一次限位回零加回找
	else if (axisHome.m_homeMode == axis::HomeModeType::Limit_Add_Z_FindHome)     	mode = 11;		     	//一次限位回零加回找  没有内置的限位+EZ回零  如有需求再封装

	//设置回原点模式
	CHECK_RTN_LONG(dmc_set_homemode(m_cardNo, axisNo, axisHome.m_homeDir, axisHome.m_homeVelMode, mode, 0));
	//设置回零偏移量及清零模式 0:禁止,1:先清0然后运动到指定位置(相对位置),2:先运动到指定位置(相对位置)再清0
	CHECK_RTN_LONG(dmc_set_home_position_unit(m_cardNo, axisNo, 2, axisHome.m_homeOffset));

	//设置回原点运动速度参数
	CHECK_RTN_LONG(dmc_set_home_profile_unit(m_cardNo, axisNo, axisHome.m_homeLowVel, axisHome.m_homeHighVel, axisHome.m_homeTacc, axisHome.m_homeTacc));

	return 0;
}
long LeadshineDmcMotionCard::axisStartHome(int axisNo)
{
	return dmc_home_move(m_cardNo, axisNo);
}
bool LeadshineDmcMotionCard::axisIsHomeDone(int axisNo)
{
	// 等待轴运动过程结束
	uint16_t doneStatus = dmc_check_done(m_cardNo, axisNo);
	if (doneStatus == 1)
	{
		WORD homeResult = 0;
		dmc_get_home_result(m_cardNo, axisNo, &homeResult);
		if (homeResult != 0)
			return true;
	}

	return false;
}
long LeadshineDmcMotionCard::axisAlarmClear(int axisNo)
{
	return dmc_clear_stop_reason(m_cardNo, axisNo);
}
long LeadshineDmcMotionCard::axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus)
{
	DWORD status = dmc_axis_io_status(m_cardNo, axisNo);

	axisStatus.ALM = ((status >> 0) & 0x01) > 0;
	axisStatus.ELP = ((status >> 1) & 0x01) > 0;
	axisStatus.ELN = ((status >> 2) & 0x01) > 0;
	axisStatus.EMG = ((status >> 3) & 0x01) > 0;
	axisStatus.ORG = ((status >> 4) & 0x01) > 0;
	axisStatus.SLP = ((status >> 6) & 0x01) > 0;
	axisStatus.SLN = ((status >> 7) & 0x01) > 0;
	axisStatus.INP = ((status >> 8) & 0x01) > 0;
	axisStatus.EZ = ((status >> 9) & 0x01) > 0;
	axisStatus.RDY = ((status >> 10) & 0x01) > 0;
	axisStatus.DSTP = ((status >> 11) & 0x01) > 0;

	return 0;
}
long LeadshineDmcMotionCard::axisGetEncoder(int axisNo, double& value)
{
	return dmc_get_encoder_unit(m_cardNo, axisNo, &value);
}
long LeadshineDmcMotionCard::axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value)
{
	(void)axisNo;
	(void)slaveId;
	(void)driverType;
	(void)encoderType;
	(void)value;
	return 0;
}
long LeadshineDmcMotionCard::axisGetPosition(int axisNo, double& value)
{
	return dmc_get_position_unit(m_cardNo, axisNo, &value);
}
long LeadshineDmcMotionCard::axisSetEncoderAndPosition(int axisNo, double value)
{
	CHECK_RTN_LONG(dmc_set_position_unit(m_cardNo, axisNo, value));
	CHECK_RTN_LONG(dmc_set_encoder_unit(m_cardNo, axisNo, value));
	return 0;
}
long LeadshineDmcMotionCard::axisCheckSuccessEncoder(int axisNo, double bandRange)
{
	return dmc_check_success_encoder(m_cardNo, axisNo);
}
long LeadshineDmcMotionCard::axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	return dmc_set_softlimit(m_cardNo, axisNo, axisSoftlimit.m_enable, axisSoftlimit.m_sourceSel, axisSoftlimit.m_slAction, axisSoftlimit.m_limitN, axisSoftlimit.m_limitP);
}
long LeadshineDmcMotionCard::axisSetEquiv(int axisNo, double equiv)
{
	return dmc_set_equiv(m_cardNo, axisNo, equiv);
}
long LeadshineDmcMotionCard::axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic)
{
	return 0;
}
long LeadshineDmcMotionCard::axisSetFactorError(int axisNo, double factorError)
{
	return dmc_set_factor_error(m_cardNo, axisNo, 1, factorError);;
}
long LeadshineDmcMotionCard::axisEnable(int axisNo, bool enable)
{
	if (enable)
	{
		return dmc_write_sevon_pin(m_cardNo, axisNo, 0);
	}
	else
	{
		return dmc_write_sevon_pin(m_cardNo, axisNo, 1);
	}
}
long LeadshineDmcMotionCard::axisIsEnable(int axisNo, bool& enable)
{
	short rtn = dmc_read_sevon_pin(m_cardNo, axisNo);
	enable = rtn == 0 ? true : false;
	return 0;
}
long LeadshineDmcMotionCard::axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	if (!axisLtc.m_enable)
	{
		if (axisLtc.m_softLtc == 1)
		{

		}
		else
		{
			//关闭软件锁存器
			CHECK_RTN_LONG(dmc_softltc_set_mode(m_cardNo, axisLtc.m_ltcNo, false, 0, 0, 0, 0));
		}
		return 0;
	}

	//高速位置锁存  DMC5C10 后四轴无高速锁存功能
	if (axisLtc.m_softLtc == 1)
	{
		//设置LTC 信号,下降沿触发
		CHECK_RTN_LONG(dmc_set_ltc_mode(m_cardNo, axisNo, axisLtc.m_logic, 0, 0));
		//设置轴的锁存方式 0：单次锁存 1：（保留） 2：连续锁存 3：触发延时急停
		CHECK_RTN_LONG(dmc_set_latch_mode(m_cardNo, axisNo, axisLtc.m_mode == 1 ? 2 : 0, axisLtc.m_ltcSrc, 0));
		//复位锁存器标志位
		CHECK_RTN_LONG(dmc_reset_latch_flag(m_cardNo, axisNo));
	}
	else
	{
		//配置软件锁存器
		CHECK_RTN_LONG(dmc_softltc_set_mode(m_cardNo, axisLtc.m_ltcNo, true, axisLtc.m_mode, axisLtc.m_inBit, axisLtc.m_logic, axisLtc.m_filter * 1000));
		//配置锁存源
		CHECK_RTN_LONG(dmc_softltc_set_source(m_cardNo, axisLtc.m_ltcNo, axisNo, axisLtc.m_ltcSrc));
		//复位锁存器
		CHECK_RTN_LONG(dmc_softltc_reset(m_cardNo, axisLtc.m_ltcNo));
	}


	return 0;

}
long LeadshineDmcMotionCard::axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	if (axisLtc.m_softLtc == 1)
	{
		//当选择锁存方式为连续锁存时，该函数第一次执行的时候，读取的是锁存器的第
		//一个锁存值，第二次执行的时候，读取的是锁存器的第二个锁存值，以此类推。
		//单次锁存时，调用 dmc_get_latch_value 不会自动清除已锁存个数，须调用dmc_reset_latch_flag
		short number = dmc_get_latch_flag(m_cardNo, axisNo);
		for (int i = 0; i < number; i++)
		{
			//读取锁存值 axis取值范围：DMC5C10：0~7，DMC5810：0~7，DMC5610：0~5  DMC5410A：0~3
			double value = 0;
			CHECK_RTN_LONG(dmc_get_latch_value_unit(m_cardNo, axisNo, &value));
			vecPos.push_back(value);
		}

	}
	else
	{
		int number;
		CHECK_RTN_LONG(dmc_softltc_get_number(m_cardNo, axisLtc.m_ltcNo, axisNo, &number));
		for (int i = 0; i < number; i++)
		{
			//读取锁存值
			double value = 0;
			CHECK_RTN_LONG(dmc_softltc_get_value_unit(m_cardNo, axisLtc.m_ltcNo, axisNo, &value));
			vecPos.push_back(value);
		}

	}

	return 0;
}
long LeadshineDmcMotionCard::positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup)
{
	if (!setup.highSpeed) {
		CHECK_RTN_LONG(dmc_compare_set_config(m_cardNo, axisNo, true, static_cast<WORD>(setup.compareSource)));
		CHECK_RTN_LONG(dmc_compare_clear_points(m_cardNo, axisNo));
		for (const double pos : setup.positions) {
			CHECK_RTN_LONG(dmc_compare_add_point_unit(
				m_cardNo, axisNo, pos, static_cast<WORD>(setup.lowSpeedDir),
				static_cast<WORD>(getPsoAction(setup.lowSpeedAction)), static_cast<DWORD>(setup.lowSpeedIoBit)));
		}
		return 0;
	}

	const WORD hcmp = static_cast<WORD>(setup.channelNo);
	CHECK_RTN_LONG(dmc_hcmp_set_mode(m_cardNo, hcmp, static_cast<WORD>(setup.mode)));
	CHECK_RTN_LONG(dmc_hcmp_set_config(
		m_cardNo, hcmp, static_cast<WORD>(axisNo), static_cast<WORD>(setup.compareSource),
		static_cast<WORD>(setup.cmpLogic), setup.pulseWidthUs));
	CHECK_RTN_LONG(dmc_hcmp_clear_points(m_cardNo, hcmp));
	for (const double pos : setup.positions) {
		CHECK_RTN_LONG(dmc_hcmp_add_point_unit(m_cardNo, hcmp, pos));
	}
	return 0;
}
long LeadshineDmcMotionCard::positionCompareDisable(int axisNo, int channelNo, bool highSpeed)
{
	if (highSpeed) {
		return dmc_hcmp_set_mode(m_cardNo, static_cast<WORD>(channelNo), 0);
	}
	return dmc_compare_set_config(m_cardNo, axisNo, false, 1);
}
long LeadshineDmcMotionCard::writeIO(int bitNo, bool value)
{
	return dmc_write_outbit(m_cardNo, bitNo, value ? 1 : 0);
}
long LeadshineDmcMotionCard::readIO(int bitNo, bool& value, bool isInputIo)
{
	long iret = 0;
	WORD vue = 0;
	if (isInputIo)
	{
		iret = dmc_read_inbit_ex(m_cardNo, bitNo, &vue);
	}
	else
	{
		iret = dmc_read_outbit_ex(m_cardNo, bitNo, &vue);
	}
	value = (vue != 0);
	return iret;
}
long LeadshineDmcMotionCard::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	WORD CANBaud = 0;		//波特率参数：0, 1, 2, 3, 4, 5对应波特率：1000Kbps，800Kbps, 500Kbps, 250Kbps, 125Kbps, 100Kbps
	WORD CANStatus = 0;		//CAN 通讯状态，0：断开，1：连接，2：异常
	WORD CANNode = nodeNo + 1;

	//设置指定 CAN-IO 扩展模块的某个输出端口的电平  CAN 节点数取值范围：1~8
	CHECK_RTN_LONG(nmc_write_outbit_ex(m_cardNo, CANNode, bitNo, value ? 1 : 0, &CANStatus));
	if (CANStatus != 1)
	{
		//处于未连接状态时  重新连接扩展模块
		CHECK_RTN_LONG(nmc_set_connect_state(m_cardNo, CANNode, 1, CANBaud));
		CHECK_RTN_LONG(nmc_get_connect_state(m_cardNo, &CANNode, &CANStatus));
		if (CANStatus != 1)
			return 608;

		CHECK_RTN_LONG(nmc_write_outbit_ex(m_cardNo, CANNode, bitNo, value ? 1 : 0, &CANStatus));
	}

	return 0;
}
long LeadshineDmcMotionCard::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	WORD CANBaud = 0;		//波特率参数：0, 1, 2, 3, 4, 5对应波特率：1000Kbps，800Kbps, 500Kbps, 250Kbps, 125Kbps, 100Kbps
	WORD CANStatus = 0;		//CAN 通讯状态，0：断开，1：连接，2：异常
	WORD CANNode = nodeNo + 1;
	WORD vue = 0;

	if (isInputIo)
	{
		CHECK_RTN_LONG(nmc_read_inbit_ex(m_cardNo, CANNode, bitNo, &vue, &CANStatus));
	}
	else
	{
		CHECK_RTN_LONG(nmc_read_outbit_ex(m_cardNo, CANNode, bitNo, &vue, &CANStatus));
	}

	if (CANStatus != 1)
	{
		//处于未连接状态时  重新连接扩展模块
		CHECK_RTN_LONG(nmc_set_connect_state(m_cardNo, CANNode, 1, CANBaud));
		CHECK_RTN_LONG(nmc_get_connect_state(m_cardNo, &CANNode, &CANStatus));
		if (CANStatus != 1)
			return 608;

		if (isInputIo)
		{
			CHECK_RTN_LONG(nmc_read_inbit_ex(m_cardNo, CANNode, bitNo, &vue, &CANStatus));
		}
		else
		{
			CHECK_RTN_LONG(nmc_read_outbit_ex(m_cardNo, CANNode, bitNo, &vue, &CANStatus));
		}
	}
	if (CANStatus == 1)
		value = (vue != 0);

	return 0;
}
long LeadshineDmcMotionCard::writeAD(int nodeNo, int channelNo, double value)
{
	WORD CANBaud = 0;		//波特率参数：0, 1, 2, 3, 4, 5对应波特率：1000Kbps，800Kbps, 500Kbps, 250Kbps, 125Kbps, 100Kbps
	WORD CANStatus = 0;		//CAN 通讯状态，0：断开，1：连接，2：异常
	WORD CANNode = nodeNo + 1;

	//设置指定 CAN-ADDA 扩展模块的某个模拟量输出端口的值 
	//CAN节点数取值范围：1~8
	//模拟量输出通道号，范围 0~1
	CHECK_RTN_LONG(nmc_set_da_output_ex(m_cardNo, CANNode, channelNo, value, &CANStatus));
	if (CANStatus != 1)
	{
		//处于未连接状态时  重新连接扩展模块
		CHECK_RTN_LONG(nmc_set_connect_state(m_cardNo, CANNode, 1, CANBaud));
		CHECK_RTN_LONG(nmc_get_connect_state(m_cardNo, &CANNode, &CANStatus));
		if (CANStatus != 1)
			return 608;

		CHECK_RTN_LONG(nmc_set_da_output_ex(m_cardNo, CANNode, channelNo, value, &CANStatus));
	}

	return 0;
}
long LeadshineDmcMotionCard::readAD(int nodeNo, int channelNo, double& value, bool isInputAd)
{
	WORD CANBaud = 0;		//波特率参数：0, 1, 2, 3, 4, 5对应波特率：1000Kbps，800Kbps, 500Kbps, 250Kbps, 125Kbps, 100Kbps
	WORD CANStatus = 0;		//CAN 通讯状态，0：断开，1：连接，2：异常
	WORD CANNode = nodeNo + 1;
	value = 0;

	if (isInputAd)
	{
		CHECK_RTN_LONG(nmc_get_ad_input_ex(m_cardNo, CANNode, channelNo, &value, &CANStatus));
	}
	else
	{
		CHECK_RTN_LONG(nmc_get_da_output_ex(m_cardNo, CANNode, channelNo, &value, &CANStatus));
	}

	if (CANStatus != 1)
	{
		//处于未连接状态时  重新连接扩展模块
		CHECK_RTN_LONG(nmc_set_connect_state(m_cardNo, CANNode, 1, CANBaud));
		CHECK_RTN_LONG(nmc_get_connect_state(m_cardNo, &CANNode, &CANStatus));
		if (CANStatus != 1)
			return 608;

		if (isInputAd)
		{
			CHECK_RTN_LONG(nmc_get_ad_input_ex(m_cardNo, CANNode, channelNo, &value, &CANStatus));
		}
		else
		{
			CHECK_RTN_LONG(nmc_get_da_output_ex(m_cardNo, CANNode, channelNo, &value, &CANStatus));
		}
	}

	return 0;
}
long LeadshineDmcMotionCard::writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}
long LeadshineDmcMotionCard::readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value)
{
	(void)axisNo;
	(void)slaveId;
	(void)axisPDO;
	(void)value;
	return 0;
}
bool LeadshineDmcMotionCard::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	if (rtn == 0)
		return true;

	auto iter = LTSMC_ERR_CODE_MAP.find(rtn);
	if (iter != LTSMC_ERR_CODE_MAP.end()) {
		rtnStr = iter->second;
	}
	else {
		rtnStr = "unknown error";
	}
	LOG_ERROR("[common][device][motion]--{} failed, sdk_ret={}, sdk_errInfo={}", strFun, rtn, rtnStr);

	return false;
}

} // end of namespace motion

} // end of namespace device

}