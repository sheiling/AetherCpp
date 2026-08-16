#include "leadshine_nmc_motion_card.h"
#include "../../motion_sdk_err_map.h"
#include <common/3rdparty/device/motion/Leadshine_NMC/include/LTSMC.h>
#include <common/include/macros.h>

extern std::map<uint16_t, std::string> LTSMC_BUS_ERR_CODE_MAP;

namespace common
{
namespace device
{
namespace motion
{

long LeadshineNmcMotionCard::initDevice()
{
	long rtn = smc_board_init(m_deviceConnectOpts_.m_connectNO, m_deviceConnectOpts_.m_connectType,
		const_cast<char*>(m_deviceConnectOpts_.m_connectString.c_str()), m_deviceConnectOpts_.m_baud);
	CHECK_RTN_LONG(rtn);

	m_frimType = FrimType::Bac632e;
	DWORD FirmID = 0;
	DWORD SubFirmID = 0;
	rtn = smc_get_card_soft_version(m_deviceConnectOpts_.m_connectNO, &FirmID, &SubFirmID);
	if (FirmID == 772)
		m_frimType = FrimType::Smc304;
	LOG_INFO("[common][device][motion]---FirmID={:x},SubFirmID={}", FirmID, SubFirmID);

	//有配置文件时  下载配置到卡
	if (rtn == 0 && !m_deviceConnectOpts_.m_configfile.empty())
	{
		rtn = smc_download_parafile(m_cardNo, const_cast<char*>(m_deviceConnectOpts_.m_configfile.c_str()));
		LOG_INFO("[common][device][motion]---smc_download_parafile rtn={}, m_cardNo={}, configfile={}", rtn, m_cardNo, m_deviceConnectOpts_.m_configfile);
	}

	return rtn;
}
void LeadshineNmcMotionCard::release()
{
	smc_board_close(m_cardNo);
}
long LeadshineNmcMotionCard::initAxis(int axisNo)
{
	long iret = 0;
	if (m_frimType == FrimType::Smc304)
	{
	}
	else
	{

	}

	return 0;
}
long LeadshineNmcMotionCard::totalAxis(uint32_t& axisNum)
{
	unsigned long valueTotalAxis = 0;
	long iret = nmcs_get_total_axes(m_deviceConnectOpts_.m_connectNO, &valueTotalAxis);
	axisNum = valueTotalAxis;
	return iret;
}
long LeadshineNmcMotionCard::axisSetVel(int axisNo, const AxisVelocityProfile& profile)
{
	return smc_set_profile_unit(m_cardNo, axisNo, profile.startVel, profile.maxVel, profile.tacc, profile.tdec, profile.endVel);
}
long LeadshineNmcMotionCard::crdSetVel(int crdNo, const AxisVelocityProfile& profile)
{
	return smc_set_vector_profile_unit(m_cardNo, crdNo, profile.startVel, profile.maxVel, profile.tacc, profile.tdec, profile.endVel);
}
long LeadshineNmcMotionCard::axisSetSMode(int axisNo, int s_mode, double s_para)
{
	return smc_set_s_profile(m_cardNo, axisNo, s_mode, s_para);
}
long LeadshineNmcMotionCard::axisSingleMove(int axisNo, const AxisSingleMoveParams& params)
{
	CHECK_RTN_LONG(axisSetVel(axisNo, params.profile));

	return smc_pmove_unit(m_cardNo, axisNo, params.dist, params.posiMode);
}
long LeadshineNmcMotionCard::axisVMove(int axisNo, uint16_t dir)
{
	return smc_vmove(m_cardNo, axisNo, dir);
}
long LeadshineNmcMotionCard::axisLinear(int crdNo, const std::vector<int>& axisNos, const std::vector<double>& dists, uint32_t posiMode)
{
	const int axisNum = static_cast<int>(axisNos.size());
	WORD* axisList = new WORD[axisNum];
	double* posList = new double[axisNum];
	for (unsigned int i = 0; i < axisNos.size(); i++)
	{
		axisList[i] = axisNos[i];
		posList[i] = dists[i];
	}
	long iret = smc_line_unit(m_cardNo, crdNo, axisNum, axisList, posList, posiMode);

	delete[] axisList;
	delete[] posList;
	return iret;
}
bool LeadshineNmcMotionCard::axisIsMoveDone(int axisNo)
{
	short doneStatus = smc_check_done(m_cardNo, axisNo);
	return doneStatus != 0;
}
bool LeadshineNmcMotionCard::crdIsMoveDone(int crdNo)
{
	short doneStatus = smc_check_done_multicoor(m_cardNo, crdNo);
	return doneStatus != 0;
}
long LeadshineNmcMotionCard::axisStop(int axisNo, uint8_t mode)
{
	return smc_stop(m_cardNo, axisNo, mode);
}
long LeadshineNmcMotionCard::axisEmgStop()
{
	return smc_emg_stop(m_cardNo);
}
long LeadshineNmcMotionCard::axisSetHomeParam(int axisNo, const common::device::axis::Axis::AxisHome& axisHome)
{
	int mode = 0;
	if (m_frimType == FrimType::Smc304)
	{
		if (axisHome.m_homeMode == axis::HomeModeType::Origin_FindHome)          		mode = 1;		//一次回零加反找
		else if (axisHome.m_homeMode == axis::HomeModeType::Origin_Add_Z_FindHome)  	mode = 5;		//一次回零加反找EZ
		else if (axisHome.m_homeMode == axis::HomeModeType::Z_FindHome)      			mode = 4;		//EZ 回零
		else if (axisHome.m_homeMode == axis::HomeModeType::Limit_FindHome)    			mode = 1;		//一次限位回零加反找
		else if (axisHome.m_homeMode == axis::HomeModeType::Limit_Add_Z_FindHome)  		mode = 5;		//一次限位回零加反找  没有内置的限位+EZ回零  如有需求再封装
	}
	else
	{
		mode = getHomeMode(axisHome.m_homeMode, axisHome.m_homeDir, axisHome.m_homeEdgeType);	// 获取总线定义标准回零模式
	}

	//设置回零偏移量及清零模式 0:禁止,1:先清0然后运动到指定位置(相对位置),2:先运动到指定位置(相对位置)再清0
	CHECK_RTN_LONG(smc_set_home_position_unit(m_cardNo, axisNo, 2, axisHome.m_homeOffset));
		LOG_INFO("[common][device][motion]---smc_set_home_position_unit(card={}, axisNo={}, 2, homeOffset={}) , sdk_ret={}", m_cardNo, axisNo, axisHome.m_homeOffset, 0);

	//设置回原点运动速度参数
	short ret = (smc_set_home_profile_unit(m_cardNo, axisNo, axisHome.m_homeLowVel, axisHome.m_homeHighVel, axisHome.m_homeTacc, axisHome.m_homeTacc));
		LOG_INFO("[common][device][motion]---smc_set_home_profile_unit(card={}, axisNo={}, homeLowVel={}, homeHighVel={}, homeAcc={}, homeDec={}) , sdk_ret={}", m_cardNo, axisNo, axisHome.m_homeLowVel, axisHome.m_homeHighVel, axisHome.m_homeTacc, axisHome.m_homeTacc, ret);

	//设置回原点模式
	ret = (smc_set_homemode(m_cardNo, axisNo, axisHome.m_homeDir, axisHome.m_homeVelMode, mode, axisHome.m_homeSourceSel));
		LOG_INFO("[common][device][motion]---smc_set_homemode(card={}, axisNo={}, homeDir={}, homeVelMode={}, homeMode={}, source={}, homeEdgeType={}) , sdk_ret={}", m_cardNo, axisNo, axisHome.m_homeDir, axisHome.m_homeVelMode, mode, axisHome.m_homeSourceSel, axisHome.m_homeEdgeType, ret);

	return 0;
}
long LeadshineNmcMotionCard::axisStartHome(int axisNo)
{
	return smc_home_move(m_cardNo, axisNo);
}
bool LeadshineNmcMotionCard::axisIsHomeDone(int axisNo)
{
	// 等待轴运动过程结束
	short doneStatus = smc_check_done(m_cardNo, axisNo);
	if (doneStatus == 1)
	{
		WORD homeResult = 0;
		smc_get_home_result(m_cardNo, axisNo, &homeResult);
		if (homeResult != 0)
			return true;
	}

	return false;
}
long LeadshineNmcMotionCard::axisAlarmClear(int axisNo)
{
	//清除报警信号
	nmcs_set_alarm_clear(m_cardNo, 2, 0);
	//清除总线错误 PortNum:总线端口号， 0、1 为 CANopen 端口, 2 为 EtherCAT 端口
	nmcs_clear_errcode(m_cardNo, 2);
	//清除指定链接号总线错误代码
	nmcs_clear_card_errcode(m_cardNo);
	//清除指定轴号总线错误代码
	return nmcs_clear_axis_errcode(m_cardNo, axisNo);
}
long LeadshineNmcMotionCard::axisReadStatus(int axisNo, common::device::axis::Axis::AxisSta& axisStatus)
{
	DWORD status = smc_axis_io_status(m_cardNo, axisNo);

	axisStatus.ELP = ((status >> 1) & 0x01) > 0;
	axisStatus.ELN = ((status >> 2) & 0x01) > 0;
	axisStatus.EMG = ((status >> 3) & 0x01) > 0;
	axisStatus.ORG = ((status >> 4) & 0x01) > 0;
	axisStatus.SLP = ((status >> 6) & 0x01) > 0;
	axisStatus.SLN = ((status >> 7) & 0x01) > 0;

	if (m_frimType == FrimType::Smc304)
	{
		axisStatus.ALM = ((status >> 0) & 0x01) > 0;
		axisStatus.INP = ((status >> 8) & 0x01) > 0;
		axisStatus.EZ = ((status >> 9) & 0x01) > 0;
		axisStatus.RDY = ((status >> 10) & 0x01) > 0;
		axisStatus.DSTP = ((status >> 11) & 0x01) > 0;
	}
	if (m_frimType == FrimType::Bac632e)
	{
		unsigned long errcode = 0;
		CHECK_RTN_LONG(nmcs_get_errcode(m_cardNo, 2, &errcode)); //获取总线状态
		axisStatus.ALM = (errcode != 0);		//总线报警 

		if (errcode != 0)
		{
			std::string errCodeStr = makeBusError(errcode);
			LOG_ERROR("[common][device][motion]---bus failed, code={}, errStr={}", errcode, errCodeStr);
		}
	}

	return 0;
}
long LeadshineNmcMotionCard::axisGetEncoder(int axisNo, double& value)
{
	return smc_get_encoder_unit(m_cardNo, axisNo, &value);
}
long LeadshineNmcMotionCard::axisGetAbsEncoder(int axisNo, int slaveId, int driverType, int encoderType, double& value)
{
	value = 0;
	// 松下驱动器
	if (driverType == 1)
	{
		common::device::axis::Axis::AxisPDO pdoData;
		pdoData.m_isFloat = false;
		pdoData.m_isUnsigned = false;
		pdoData.m_length = 32;
		pdoData.m_port = 2;
		pdoData.m_index = 0x6064;
		pdoData.m_subIndex = 0;
		return readPDO(axisNo, slaveId, pdoData, value);
	}
	return 0;
}
long LeadshineNmcMotionCard::axisGetPosition(int axisNo, double& value)
{
	return smc_get_position_unit(m_cardNo, axisNo, &value);
}
long LeadshineNmcMotionCard::axisSetEncoderAndPosition(int axisNo, double value)
{
	CHECK_RTN_LONG(smc_set_position_unit(m_cardNo, axisNo, value));
	CHECK_RTN_LONG(smc_set_encoder_unit(m_cardNo, axisNo, value));
	return 0;
}
long LeadshineNmcMotionCard::axisCheckSuccessEncoder(int axisNo, double bandRange)
{
	return 0;
}
long LeadshineNmcMotionCard::axisSetSoftlimit(int axisNo, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	return smc_set_softlimit_unit(m_cardNo, axisNo, axisSoftlimit.m_enable, axisSoftlimit.m_sourceSel, axisSoftlimit.m_slAction, axisSoftlimit.m_limitN, axisSoftlimit.m_limitP);
}
long LeadshineNmcMotionCard::axisSetEquiv(int axisNo, double equiv)
{
	return smc_set_equiv(m_cardNo, axisNo, equiv);
}
long LeadshineNmcMotionCard::axisSetStaLogic(int axisNo, common::device::axis::Axis::AxisStaLogic axisStaLogic)
{
	short ret = 0;

	if (m_frimType == FrimType::Smc304)
	{
		if (axisStaLogic.m_orgLogic != -1) 
		{
			WORD logic = 0;
			double filter = 0;
			CHECK_RTN_LONG(smc_get_home_pin_logic(m_cardNo, axisNo, &logic, &filter));
			if (axisStaLogic.m_orgLogic != logic) {
				CHECK_RTN_LONG(smc_set_home_pin_logic(m_cardNo, axisNo, (axisStaLogic.m_orgLogic == 0 ? 0 : 1), filter));
			}
		}
	}

	return 0;
}
long LeadshineNmcMotionCard::axisSetFactorError(int axisNo, double factorError)
{
	return 0;
}
long LeadshineNmcMotionCard::axisEnable(int axisNo, bool enable)
{
	if (m_frimType == FrimType::Smc304)
	{
		return smc_write_sevon_pin(m_cardNo, axisNo, enable ? 0 : 1);
	}
	else
	{
		if (enable)
		{
			CHECK_RTN_LONG(nmcs_set_axis_enable(m_cardNo, axisNo));

			uint16_t statemachine = 0; // 轴状态机
			time_t beginTm = time(NULL);
			while (statemachine != 4) {
				time_t currTm = time(NULL);
				time_t costTm = currTm - beginTm;
				if (costTm > 3) {
					LOG_ERROR("[common][device][motion]---NmcsSetAxisEnable timeout(3s), axisNO={}", axisNo);
					return false;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10)); //延时一段时间
				//等待轴状态机切换为使能完成状态
				CHECK_RTN_LONG(nmcs_get_axis_state_machine(m_cardNo, axisNo, &statemachine));
			}

			return 0;
		}
		else
		{
			return nmcs_set_axis_disable(m_cardNo, axisNo);
		}
	}

}
long LeadshineNmcMotionCard::axisIsEnable(int axisNo, bool& enable)
{
	short rtn = smc_read_sevon_pin(m_cardNo, axisNo);
	enable = rtn == 0 ? true : false;
	return 0;
}
long LeadshineNmcMotionCard::axisEnableLtc(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	if (!axisLtc.m_enable)
	{
		if (axisLtc.m_softLtc == 1)
		{

		}
		else
		{
			//关闭软件锁存器
			CHECK_RTN_LONG(smc_softltc_set_mode(m_cardNo, axisLtc.m_ltcNo, false, 0, 0, 0, 0));
		}
		return 0;
	}

	//高速位置锁存 
	if (axisLtc.m_softLtc == 1)
	{
		//设置LTC 信号,下降沿触发
		CHECK_RTN_LONG(smc_set_ltc_mode(m_cardNo, axisNo, axisLtc.m_logic, 0, 0));
		//设置轴的锁存方式 0：单次锁存 1：（保留） 2：连续锁存 3：触发延时急停
		CHECK_RTN_LONG(smc_set_latch_mode(m_cardNo, axisNo, axisLtc.m_mode == 1 ? 2 : 0, axisLtc.m_ltcSrc, 0));
		//复位锁存器标志位
		CHECK_RTN_LONG(smc_reset_latch_flag(m_cardNo, axisNo));
	}
	else
	{
		//配置软件锁存器
		CHECK_RTN_LONG(smc_softltc_set_mode(m_cardNo, axisLtc.m_ltcNo, true, axisLtc.m_mode, axisLtc.m_inBit, axisLtc.m_logic, axisLtc.m_filter * 1000));
		//配置锁存源
		CHECK_RTN_LONG(smc_softltc_set_source(m_cardNo, axisLtc.m_ltcNo, axisNo, axisLtc.m_ltcSrc));
		//复位锁存器
		CHECK_RTN_LONG(smc_softltc_reset(m_cardNo, axisLtc.m_ltcNo));
	}

	return 0;
}
long LeadshineNmcMotionCard::axisGetLtcPos(int axisNo, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	if (axisLtc.m_softLtc == 1)
	{
		short number = smc_get_latch_flag(m_cardNo, axisNo);
		for (int i = 0; i < number; i++)
		{
			double value = 0;
			CHECK_RTN_LONG(smc_get_latch_value_unit(m_cardNo, axisNo, &value));
			vecPos.push_back(value);
		}

	}
	else
	{
		int number;
		CHECK_RTN_LONG(smc_softltc_get_number(m_cardNo, axisLtc.m_ltcNo, axisNo, &number));
		for (int i = 0; i < number; i++)
		{
			//读取锁存值
			double value = 0;
			CHECK_RTN_LONG(smc_softltc_get_value_unit(m_cardNo, axisLtc.m_ltcNo, axisNo, &value));
			vecPos.push_back(value);
		}

	}

	return 0;
}
long LeadshineNmcMotionCard::positionCompareSetup(int axisNo, const PositionCompareDriverSetup& setup)
{
	if (!setup.highSpeed) {
		CHECK_RTN_LONG(smc_compare_set_config(m_cardNo, axisNo, true, static_cast<WORD>(setup.compareSource)));
		CHECK_RTN_LONG(smc_compare_clear_points(m_cardNo, axisNo));
		for (const double pos : setup.positions) {
			CHECK_RTN_LONG(smc_compare_add_point_unit(
				m_cardNo, axisNo, pos, static_cast<WORD>(setup.lowSpeedDir),
				static_cast<WORD>(getPsoAction(setup.lowSpeedAction)), static_cast<DWORD>(setup.lowSpeedIoBit)));
		}
		return 0;
	}

	const WORD hcmp = static_cast<WORD>(setup.channelNo);
	CHECK_RTN_LONG(smc_hcmp_set_mode(m_cardNo, hcmp, static_cast<WORD>(setup.mode)));
	CHECK_RTN_LONG(smc_hcmp_set_config(
		m_cardNo, hcmp, static_cast<WORD>(axisNo), static_cast<WORD>(setup.compareSource),
		static_cast<WORD>(setup.cmpLogic), setup.pulseWidthUs));
	CHECK_RTN_LONG(smc_hcmp_clear_points(m_cardNo, hcmp));
	for (const double pos : setup.positions) {
		CHECK_RTN_LONG(smc_hcmp_add_point_unit(m_cardNo, hcmp, pos));
	}
	return 0;
}
long LeadshineNmcMotionCard::positionCompareDisable(int axisNo, int channelNo, bool highSpeed)
{
	if (highSpeed) {
		return smc_hcmp_set_mode(m_cardNo, static_cast<WORD>(channelNo), 0);
	}
	return smc_compare_set_config(m_cardNo, axisNo, false, 1);
}
long LeadshineNmcMotionCard::writeIO(int bitNo, bool value)
{
	return smc_write_outbit(m_cardNo, bitNo, value ? 1 : 0);
}
long LeadshineNmcMotionCard::readIO(int bitNo, bool& value, bool isInputIo)
{
	long iret = 0;
	WORD vue = 0;
	if (isInputIo)
	{
		iret = smc_read_inbit_ex(m_cardNo, bitNo, &vue);
	}
	else
	{
		iret = smc_read_outbit_ex(m_cardNo, bitNo, &vue);
	}
	value = (vue != 0);
	return iret;
}
long LeadshineNmcMotionCard::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	WORD ProtNo = bitNo / 32;   //IO组号最小值为0，每32点IO一组；IO数满32，组号加1
	WORD BitNo = bitNo % 32;	//位号
	CHECK_RTN_LONG(nmcs_write_outbit_extern(m_cardNo, ProtNo, nodeNo, BitNo, value ? 1 : 0));

	return 0;
}
long LeadshineNmcMotionCard::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	long iret = 0;
	WORD ProtNo = bitNo / 32;   //IO组号最小值为0，每32点IO一组；IO数满32，组号加1
	WORD BitNo = bitNo % 32;	//位号
	WORD vue = 0;
	if (isInputIo)
	{
		iret = nmcs_read_inbit_extern(m_cardNo, ProtNo, nodeNo, BitNo, &vue);
	}
	else
	{
		iret = nmcs_read_outbit_extern(m_cardNo, ProtNo, nodeNo, BitNo, &vue);
	}
	value = (vue != 0);

	return iret;
}
long LeadshineNmcMotionCard::writeAD(int nodeNo, int channelNo, double value)
{
	return smc_set_da_output(m_cardNo, channelNo, value);
}
long LeadshineNmcMotionCard::readAD(int nodeNo, int channelNo, double& value, bool isInputAd)
{
	if (isInputAd)
	{
		value = smc_get_ain(m_cardNo, channelNo);
	}
	else
	{
		CHECK_RTN_LONG(smc_get_da_output(m_cardNo, channelNo, &value));
	}
	return 0;
}
long LeadshineNmcMotionCard::writePDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double value)
{
	const int nodeId = slaveId > 0 ? slaveId : (1000 + axisNo);

	if (!axisPDO.m_isFloat)
	{
		DWORD dword = value;
		CHECK_RTN_LONG(nmcs_set_node_od(m_cardNo, axisPDO.m_port, nodeId, axisPDO.m_index, axisPDO.m_subIndex, axisPDO.m_length, dword));
	}
	else
	{
		CHECK_RTN_LONG(nmcs_set_node_od_float(m_cardNo, axisPDO.m_port, nodeId, axisPDO.m_index, axisPDO.m_subIndex, axisPDO.m_length, (float)value));
		
	}

	return 0;
}
long LeadshineNmcMotionCard::readPDO(int axisNo, int slaveId, common::device::axis::Axis::AxisPDO& axisPDO, double& value)
{
	const int nodeId = slaveId > 0 ? slaveId : (1000 + axisNo);
	
	value = 0;
	if (!axisPDO.m_isFloat)
	{
		DWORD dword = 0;
		CHECK_RTN_LONG(nmcs_get_node_od(m_cardNo, axisPDO.m_port, nodeId, axisPDO.m_index, axisPDO.m_subIndex, axisPDO.m_length, &dword));
		
		if (axisPDO.m_isUnsigned)
			value = dword;
		else
			value = static_cast<int>(dword);
	}
	else
	{
		float vue = 0;
		CHECK_RTN_LONG(nmcs_get_node_od_float(m_cardNo, axisPDO.m_port, nodeId, axisPDO.m_index, axisPDO.m_subIndex, axisPDO.m_length, &vue));
		value = vue;
	}

	return 0;
}
bool LeadshineNmcMotionCard::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
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
	LOG_ERROR("[common][device][motion]---{} failed, sdk_ret={}, sdk_errInfo={}", strFun, rtn, rtnStr);

	return false;
}

std::string LeadshineNmcMotionCard::makeBusError(long rtn)
{
	std::string errCodeStr = "";

	auto iter = LTSMC_BUS_ERR_CODE_MAP.find(rtn);
	if (iter != LTSMC_BUS_ERR_CODE_MAP.end()) {
		errCodeStr = iter->second;
	}
	else {
		errCodeStr = "unknown error";
	}

	return errCodeStr;
}

} // end of namespace motion

} // end of namespace device

}

