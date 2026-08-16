#ifndef _DISPLAY_CONTROL_BASE_HH
#define _DISPLAY_CONTROL_BASE_HH

#include "display_control_data.h"


namespace common
{
namespace device
{

// 显示控制设备型号定义
enum DisplayControlDevType {
	DCU_MARS = 0,            	// MARS
	DCU_COEX = 1,           	// COEX
};
NLOHMANN_JSON_SERIALIZE_ENUM(DisplayControlDevType, {
{ DisplayControlDevType::DCU_MARS,       "DCU_MARS" },
{ DisplayControlDevType::DCU_COEX,       "DCU_COEX"}
	});

// 显示控制设备信息
struct DisplayControlDevInfo {
	std::string ip = "";                            // IP地址
	uint16_t port = 0;                              // 端口
	void reflectFromJson(const json& j)
	{
		getFromJson(j, "ip", ip);
		getFromJson(j, "port", port);
	}
	void reflectToJson(json& j)const
	{
		j["ip"] = ip;
		j["port"] = port;
	}
};


class DisplayControlBase {
public:
	DisplayControlBase() :
		m_currentCorrectType(enum_ScreenCorrectType::COB) {
	}
	virtual ~DisplayControlBase() = default;
	void reflectFromJson(const json& j)
	{
		fromJson(j, "connOpts", m_displayControlDevInfo_);
		getFromJson(j, "type", m_typeName);
	}
	void reflectToJson(json& j)const
	{
		toJson(j["connOpts"], m_displayControlDevInfo_);
		j["type"] = m_typeName;
	}
public:
	virtual bool init() = 0;
	virtual bool release() = 0;

	// 打屏
	virtual common::utility::ResultInfo controlDisplay(const ST_HWDisplayParameterBase& hwDisplayParameter) = 0;
	
	// 擦除系数
	virtual common::utility::ResultInfo eraseCoefficients() = 0;
	// 关闭校正效果
	virtual common::utility::ResultInfo correctionEffect(bool isClose) = 0;
	
	// 固化系数到接收卡
	virtual common::utility::ResultInfo writeFlash() = 0;
	// 切换校正开关，用于开启或关闭校正功能
	virtual common::utility::ResultInfo toggleCorrectionSwitch(const ST_ToggleCorrectionSwitch& toggleCorrectionSwitch) = 0;
	// 获取屏体信息--单屏
	virtual common::utility::ResultInfo displayInfoForIndex(int index, ST_DisplayScreenInfo& displayScreenInfo) = 0;
	// 获取屏体信息--组合屏
	virtual common::utility::ResultInfo displayInfoForName(std::string screenName, ST_DisplayCombScreenInfo& displayCombScreenInfo) = 0;
	// 获取配置文件
	virtual common::utility::ResultInfo getFile() = 0;
	// 发送配置文件
	virtual common::utility::ResultInfo sendFile() = 0;
	// 检查 DVI 信源
	virtual common::utility::ResultInfo checkDVISignal(const common::utility::Rect& rect, bool& isExistDVISignal) = 0;
	// 获取伽马列
	virtual common::utility::ResultInfo getGammaList(std::map<enum_LedColorType, std::vector<unsigned int>>& gammaList) = 0;


	// 切换系数上传模式(稳定/快速)
	virtual common::utility::ResultInfo setUploadMode(enum_UploadCoefModeType mode, int timeout = 2000) = 0;
	// 上传系数
	virtual common::utility::ResultInfo uploadCoefficients(const ST_UploadCoefficientsParam& uploadCoefficientsParam) = 0;
	// 上传多灰阶系数
	virtual common::utility::ResultInfo uploadMultiLayerCoefficients() = 0;


	// 回读画质引擎
	virtual common::utility::ResultInfo readImageEngine(bool& isExistState, bool& isOpenState) = 0;
	// 固化画质引擎
	virtual common::utility::ResultInfo writeImageEngine(enum_ImageEngineMode mode) = 0;
	// 固化配置文件
	virtual common::utility::ResultInfo writeConfigFile() = 0;
	// flash检测
	virtual common::utility::ResultInfo checkFlashStatus() = 0;


	// 用于处判断是否支持写入模组 ID 的操作
	virtual common::utility::ResultInfo supportWriteModuleID(bool& isSupportWriteModuleID) = 0;
	// 写入模组ID
	virtual common::utility::ResultInfo writeModuleID(ST_LocationInfo cabLocationinfo, int moduleWidth, int moduleHeight, const std::vector<std::string>& idList) = 0;
	// 读取模组ID
	virtual common::utility::ResultInfo readModuleID() = 0;
	// 写入箱体ID
	virtual common::utility::ResultInfo writeCabinetID(const std::map<std::string, std::string> cabinetIDMap) = 0;
	// 读取箱体ID
	virtual common::utility::ResultInfo readCabinetID(std::vector<std::string>& cabIdList) = 0;
	// 用于判断是否支持硬件打屏
	virtual common::utility::ResultInfo supportHWDisplayScreen(bool& isSupportHWDisplay) = 0;
	// 获取屏体模式
	virtual common::utility::ResultInfo getCurrentScreenMode(ST_ScreenOperationMode& screenOperationMode) = 0;
	// 获取屏亚像素信息
	virtual common::utility::ResultInfo getCurrentSubPixInfo(ST_SubPixInfo& subPixInfo) = 0;
	// 获取屏体列表
	virtual common::utility::ResultInfo getScreenList(enum_ScreenOperationMode screenMode, ST_DisplayScreenList& displayScreenList) = 0;
	// 获取操作通信口
	virtual common::utility::ResultInfo getComPort(int index, std::string& comPort) = 0;

	enum_ScreenCorrectType getCurrentCorrectType() const;
	void setCurrentCorrectType(const enum_ScreenCorrectType& val);

	//检查返回值是否报警
	common::utility::ResultInfo checkError(long rtn, const std::string& strFun = "");
protected:
	//生成错误信息
	virtual bool makeError(long rtn, const std::string& strFun, std::string& rtnStr) = 0;



protected:
	DisplayControlDevType m_typeName = DCU_MARS;				// 设备型号
	DisplayControlDevInfo m_displayControlDevInfo_;             // 设备信息  

	enum_ScreenCorrectType m_currentCorrectType;    			// 当前校正类型
};


}

}

#endif
