#ifndef _LCT_DISPLAY_CONTROL_HH
#define _LCT_DISPLAY_CONTROL_HH

#include "../display_control_base.h"
#include "proxy/lct_data_codec.h"
#include <common/utility/ipc_tool/tcp/client/tcp_client.h>
#include <atomic>

namespace common
{
namespace device
{

class DisplayControlLct : public DisplayControlBase {
public:
	DisplayControlLct();
	~DisplayControlLct() = default;
public:
	virtual bool init() override;
	virtual bool release() override;

	// 打屏
	virtual common::utility::ResultInfo controlDisplay(const ST_HWDisplayParameterBase& hwDisplayParameter) override;
	
	// 擦除系数
	virtual common::utility::ResultInfo eraseCoefficients() override;
	// 校正效果
	virtual common::utility::ResultInfo correctionEffect(bool isClose) override;
	// 上传多灰阶系数
	virtual common::utility::ResultInfo uploadMultiLayerCoefficients() override;
	// 固化系数到接收卡
	virtual common::utility::ResultInfo writeFlash() override;
	// 切换校正开关，用于开启或关闭校正功能
	virtual common::utility::ResultInfo toggleCorrectionSwitch(const ST_ToggleCorrectionSwitch& toggleCorrectionSwitch) override;
	// 获取屏体信息--单屏
	virtual common::utility::ResultInfo displayInfoForIndex(int index, ST_DisplayScreenInfo& displayScreenInfo) override;
	// 获取屏体信息--组合屏
	virtual common::utility::ResultInfo displayInfoForName(std::string screenName, ST_DisplayCombScreenInfo& displayCombScreenInfo) override;
	// 获取配置文件
	virtual common::utility::ResultInfo getFile() override;
	// 发送配置文件
	virtual common::utility::ResultInfo sendFile() override;
	// 检查 DVI 信源
	virtual common::utility::ResultInfo checkDVISignal(const common::utility::Rect& rect, bool& isExistDVISignal) override;
	// 获取伽马列表
	virtual common::utility::ResultInfo getGammaList(std::map<enum_LedColorType, std::vector<unsigned int>>& gammaList) override;
	

	// 切换系数上传模式(稳定/快速)
	virtual common::utility::ResultInfo setUploadMode(enum_UploadCoefModeType mode, int timeout = 2000) override;
	// 上传系数
	virtual common::utility::ResultInfo uploadCoefficients(const ST_UploadCoefficientsParam& uploadCoefficientsParam) override;


	// 回读画质引擎
	virtual common::utility::ResultInfo readImageEngine(bool& isExistState, bool& isOpenState) override;
	// 固化画质引擎
	virtual common::utility::ResultInfo writeImageEngine(enum_ImageEngineMode mode) override;
	// 固化配置文件
	virtual common::utility::ResultInfo writeConfigFile() override;
	// flash检测
	virtual common::utility::ResultInfo checkFlashStatus() override;


	// 用于判断是否支持写入模组 ID 的操作
	virtual common::utility::ResultInfo supportWriteModuleID(bool& isSupportWriteModuleID) override;
	// 写入模组ID
	virtual common::utility::ResultInfo writeModuleID(ST_LocationInfo cabLocationinfo, int moduleWidth, int moduleHeight, const std::vector<std::string>& idList) override;
	// 读取模组ID
	virtual common::utility::ResultInfo readModuleID() override;
	// 写入箱体ID
	virtual common::utility::ResultInfo writeCabinetID(const std::map<std::string, std::string> cabinetIDMap) override;
	// 读取箱体ID
	virtual common::utility::ResultInfo readCabinetID(std::vector<std::string>& cabIdList) override;
	// 用于判断是否支持硬件打屏
	virtual common::utility::ResultInfo supportHWDisplayScreen(bool& isSupportHWDisplay) override;
	// 获取屏体模式
	virtual common::utility::ResultInfo getCurrentScreenMode(ST_ScreenOperationMode& screenOperationMode) override;
	// 获取屏亚像素信息
	virtual common::utility::ResultInfo getCurrentSubPixInfo(ST_SubPixInfo& subPixInfo) override;
	// 获取屏体列表
	virtual common::utility::ResultInfo getScreenList(enum_ScreenOperationMode screenMode, ST_DisplayScreenList& displayScreenList) override;
	// 获取操作通信口
	virtual common::utility::ResultInfo getComPort(int index, std::string& comPort) override;

protected:
	virtual common::utility::ResultInfo setLowAshuniformity(int rValue, int gValue, int bValue);	//设置低灰均匀性
	virtual common::utility::ResultInfo setLowCorrectionState(bool isOpen);						//设置低灰校正开关
	virtual common::utility::ResultInfo setLowCorrectionMode(int num);								//设置低灰模式
	virtual common::utility::ResultInfo setCurrentGammaValue(int layer, int value);				//设置低灰校正灰阶gamma
	virtual common::utility::ResultInfo getModuleTopologyInfo(int& totalRow, int& totalCol, std::vector<ST_ModuleTopologyInfo>& moduleTopologyInfo);	//获取模组拓补信息
	virtual common::utility::ResultInfo getImageEngineExistState(bool& state);						//获取画质引擎存在状态
	virtual common::utility::ResultInfo getImageEngineOpenState(bool& state);						//获取画质引擎开启状态

	virtual common::utility::ResultInfo uploadNormalCoef(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray);	//上传常规系数
	virtual common::utility::ResultInfo uploadMultiLayerCoef(const ST_LocationInfo& locaitonInfo, const std::vector<unsigned int>& uploadLayerList, const std::vector<unsigned char>& layerCoefs);	//上传全灰阶系数
	virtual common::utility::ResultInfo uploadLowGrayCoef(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray);	//上传低灰系数
	virtual common::utility::ResultInfo uploadLowGrayCoef_9005_9937(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray);	//9005/9937芯片低灰阶校正 系数上传
	virtual common::utility::ResultInfo uploadAdaptiveCoeff(const ST_LocationInfo& locaitonInfo, const std::vector<unsigned int>& grayLayerList, const std::vector<unsigned int>& adaptGrayLayerList, const std::vector<std::vector<uint16_t>>& multiCoefArray, const std::vector<std::vector<uint16_t>>& adaptCoefArray);	//上传自适应系数

	common::utility::ResultInfo hWDisplayScreen(ST_HWDisplayParameterBase* hwDisplayParameter, std::shared_ptr<CorrectionReplyPack> replyPack);
	common::utility::ResultInfo setDisplayCustomBitmap(const ST_LocationInfo& locaitonInfo, std::string bmpFileName, enum_BmpShowModeType bmpShowMode, std::shared_ptr<CorrectionReplyPack> replyPack);
	std::string CreateBitmap(const ST_DisplayLoactionInfo& displayInfo, const ST_DisplayUnitInfo& uinitInfo);

private:
	bool sendFile(const std::string& strFilePath, std::shared_ptr<CorrectionReplyPack> replyPack);
	bool sendCorrectionData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, enum_LCTCommands correctionCMD, int timeout = CommandTimeOut::SENDER_SAVEPARAMS_TIMEOUT);
	bool sendMixtureData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, const std::string& filePath, enum_LCTCommands correctionCMD, int timeout = CommandTimeOut::SENDER_SAVEMORE_TIMEOUT);
	bool passthroughData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, enum_PassthroughDataType passType);
	bool sendFilePack(ST_OperFileInfo& sendFileInfo, std::shared_ptr<CorrectionReplyPack> replyPack);
	bool getFilePack(const ST_OperFileInfo& sendFileInfo, std::shared_ptr<CorrectionReplyPack> replyPack);

	std::string saveByteCofeFile(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray, bool bBrightDarkLineFix = false);
    std::string saveByteCofeFile(std::string filePath, const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray, bool bBrightDarkLineFix = false);
	std::string saveMultiLayerCoefFile(const ST_LocationInfo& locationInfo, const std::vector<unsigned int>& uploadLayerList, const std::vector<unsigned char>& layerCoefs);
    std::string savedAdaptCoefFile(const ST_LocationInfo& locationInfo, const std::vector<unsigned int>& updateLayerList, const std::vector<unsigned int>& updateWhiteLayerList, const std::vector<std::vector<uint16_t>>& multiLayerCoefs, const std::vector<std::vector<uint16_t>>& layerCoefs);

private:
	//数据初始化
	common::utility::ResultInfo initData();

	//计算系数上传时的超时时间
	int getUploadlCoefTimeout(const ST_LocationInfo& locaitonInfo);
protected:
	//生成错误信息
	virtual bool makeError(long rtn, const std::string& strFun, std::string& rtnStr) override;

private:
	std::atomic_bool			m_connStatus_ = false;

	LCTDataCodec				m_dataCodec;							//数据编解码

	bool						m_bIsSupportHWDisplay = false; 			//是否支持硬件打屏
	bool						m_bImageEngineModeIsOver = false;
	bool						m_bImageEngineModeResult = false;
	bool						m_bIsSupportCheckerBoard = false;		//是否支持棋盘格打屏
	bool						m_isAdaptiveCorrect = false;			//是否自适应
	bool						m_bWriteFlashIsOver = false; 			//系数固化是否完成

	ST_DeviceConfigInfo			m_deviceConfigInfo;						//设备配置信息
	ST_SubPixInfo				m_subPixInfo;							//当前屏亚像素信息
	ST_ScreenOperationMode		m_curScreenMode;						//当前屏体模式
	ST_DisplayScreenList		m_curScreenList;						//当前屏体列表

	ST_DisplayLoactionInfo      m_DisplayInfo;
	ST_DisplayUnitInfo          m_UinitInfo;

	std::vector<ST_DisplayScreenInfo> m_vecScreenInfo;               	//单屏信息
	std::vector<ST_DisplayCombScreenInfo> m_vecCombScreenInfo;       	//组合屏信息
};

}

}

#endif
