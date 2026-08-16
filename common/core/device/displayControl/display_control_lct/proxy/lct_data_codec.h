#ifndef _LCT_DATA_CODEC_HH
#define _LCT_DATA_CODEC_HH

#include "../../display_control_data.h"

namespace common
{
namespace device
{

class LCTDataCodec
{

public:
	explicit LCTDataCodec();
	LCTDataCodec(const LCTDataCodec& codec);
	LCTDataCodec operator =(const LCTDataCodec& codec);

	//显示屏信息
	std::string     SerializeDisplayScreenInfo(ST_DisplayScreenInfo info);
	bool            DeserializeDisplayScreenInfo(std::string sData, ST_DisplayScreenInfo& info);

	//解析设备通讯口状态查询
	bool            DeserializeDeviceStatusInfo(std::string sData, std::vector<ST_DeviceStatusInfo>& infoList);

	//显示区域
	std::string     SerializeDisplaySection(ST_DisplayLoactionInfo info);
	bool            DeserializeDisplaySection(std::string sData, ST_DisplayLoactionInfo& info);

	//逐点显示区域
	std::string     SerializePointDisplaySection(ST_DisplayLoactionInfo info);
	bool            DeserializePointDisplaySection(std::string sData, ST_DisplayLoactionInfo& info);

	//图片附加信息
	std::string     SerializeCustomBitmapAddInfo(const ST_LocationInfo& locaitonInfo, enum_BmpShowModeType bmpShowMode);
	bool            DeserializeCustomBitmapAddInfo(std::string sData, ST_LocationInfo& locaitonInfo, enum_BmpShowModeType& bmpShowMode);

	std::string     SerializeScreenInfo(const ST_LocationInfo& info);
	// 9005/9937芯片低灰校正
	std::string     SerializeScreenLCInfo(const ST_LocationInfo& info);

	//棋盘格打屏
	std::string     SerializeCheckerBoardInfo(const ST_DisplayCheckerBoard& checkerBoardInfo);

	//
	std::string     SerializeSolidifyScanInfo(ST_SolidifyAddressInfo info);
	bool            DeserializeSolidifyScanInfo(std::string sData, ST_SolidifyAddressInfo& info);

	//
	std::string     SerialReloadCoefficients(const ST_LocationInfo& info);
	bool            DeserializeReloadCoefficients(std::string sData, ST_LocationInfo& info);

	//
	std::string     SerialCorrectMode(bool isEnableCorrect, enum_CorrectMode correctMode, bool isChromaCorrentionLowGray);
	bool            DeserialCorrectMode(std::string sData, bool& isEnableCorrect, enum_CorrectMode& correctMode, bool& isChromaCorrentionLowGray);

	std::string     SerialCorrectModeLowLum(bool isEnableCorrect, enum_CorrectMode correctMode);

	//
	std::string     SerialGetRectAverCoefs(const ST_LocationInfo& info);
	bool            DeserialGetRectAverCoefs(std::string sData, ST_LocationInfo& info);

	//
	std::string     SerialFirstScannerIDData(ST_ScannerID scannerID);
	bool            DeserialFirstScannerIDData(std::string sData, ST_ScannerID& scannerID);

	//
	std::string     SerialModuleFlashOperation(const ST_LocationInfo& info, enum_ModuleFlashOperation operation);
	bool            DeserialModelFlashOperation(std::string sData, enum_ModuleFlashOperation& operation, ST_LocationInfo& info);

	//序列化反序列化Json格式Rect
    std::string     SerialRect(common::utility::Rect rect);
    bool            DeserialRect(std::string sData, common::utility::Rect& rect);

	//序列化反序列化Json格式Size
    std::string     SerialSize(common::utility::Size size);
    bool            DeserialSize(std::string sData, common::utility::Size& size);

	//序列化反序列化Json格式Point
    std::string     SerialPoint(common::utility::Point point);
    bool            DeserialPoint(std::string sData, common::utility::Point& point);

	//序列化打屏数据
	std::string     SerialHWDisplayParameterPattern(ST_HWDisplayParameterPattern info);
	std::string     SerialHWDisplayParameterBlock(ST_HWDisplayParameterBlock info);
	std::string     SerialHWDisplayParameterClose(ST_HWDisplayParameterClose info);

	//
	std::string     SerialPassthroughPack(ST_PassthroughPack pack);
	bool            DeserialPassthroughPack(std::string sData, ST_PassthroughPack& pack);

	//序列化亚像素打屏数据
	std::string     SerialSubPixHWDisplayParameterPattern(ST_HWDisplayParameterPatternExtend& info);			//逐点
	std::string     SerialSubPixCheckerBoardHWDisplayParameterPattern(ST_DisplayCheckerBoardExtend info);		//棋盘格

	//反序列化亚像素信息
	bool            DeserializeSubPixInfo(std::string sData, ST_SubPixInfo& subPixInfo);

	//序列化相机亮暗线系数信息
	std::string     SerializeDarkLineCamera(const std::string& coeffData);

	//反序列化画质引擎存在信息
	bool            DeserializeImageEngineExistInfo(std::string sData, bool& state);
	//反序列化画质引擎开启信息
	bool            DeserializeImageEngineOpenInfo(std::string sData, bool& state);
	//反序列化箱体编号信息
	bool            DeserializeCabinetId(std::string sData, std::vector<std::string>& cabIdList);
	//反序列化模组拓扑信息
	bool            DeserializeModuleTopologyInfo(std::string sData, std::vector<ST_ModuleTopologyInfo>& moduleTopologyInfo);
	//序列化模组编号写入信息
	std::string     SerializeWriteModuleId(ST_LocationInfo info, int moduleWidth, int moduleHeight, const std::vector<std::string>& idList);
	//序列化箱体编号写入信息
	std::string     SerializeWriteCabinetId(ST_DisplayScreenInfo info, const std::map<std::string, std::string> cabinetIDMap);

	
	//序列化回形屏打屏信息
	std::string     SerializeDisplayKB(ST_DisplayKBImage info);

	//序列化Gamma信息
	std::string     SerialGammaInfo(ST_GammaInfo gammaInfo);


	std::string     SerialNotifyRectAverCoefs(uint16_t* m_AvgCoef);
	bool            DeserializeNotifyRectAverCoefs(const std::string& sData, std::vector<uint16_t>& coefs);

	bool            DeserializeDisplayCountNew(std::vector<std::string>& diplayList, std::string strDisplayJson);
	bool            DeserializeComboScreenInfo(ST_DisplayCombScreenInfo& outInfo, std::string data);
	bool            DeserializeModuleFlashNotifyEx(std::string sData, ST_ModuleFlashNotifyParamsEx& ParamsEx);
	bool            DeserializeGammaTable(std::map<enum_LedColorType, std::vector<unsigned int>>& gammaMap, common::device::CommResults& result, std::string data);
	bool            DeserializeFeaturesSupport(std::map<enum_LCTSupportFeatureList, bool>& featureList, std::string data);//解析是否支持全灰阶校正



private:
	const std::string m_SplitTag = "/";
	const std::string m_SplitContentTag = ",";

};

}
}

#endif
