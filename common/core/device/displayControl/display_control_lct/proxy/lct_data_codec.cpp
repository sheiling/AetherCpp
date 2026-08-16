#include "lct_data_codec.h"
#include <common/utility/string_tool/string_format/string_format.hpp>
#include <common/utility/file_tool/logger/hlog.h>
#include <random>

namespace common
{
namespace device
{
std::string createUuid()
{
	std::mt19937 rng;
	std::random_device rd;
	// 如果硬件随机源不可用，回退到时间种子
	if (rd.entropy() == 0) {
		rng.seed(std::time(nullptr));
	}
	else {
		rng.seed(rd());     // 设置种子（使用硬件随机设备）
	}
	std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);  // 选择分布类型 均匀整数分布
	return std::to_string(dist(rng));
}
std::string toBase64(const std::string& input) {
	const std::string base64Chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string encoded;
	int val = 0;
	int bits = 0;
	const size_t len = input.size();

	encoded.reserve(((len + 2) / 3) * 4); // 预分配空间

	for (size_t i = 0; i < len; ++i) {
		val = (val << 8) | static_cast<uint8_t>(input[i]);
		bits += 8;

		while (bits >= 6) {
			encoded.push_back(base64Chars[(val >> (bits - 6)) & 0x3F]);
			bits -= 6;
		}
	}

	// 处理剩余的 bits
	if (bits > 0) {
		encoded.push_back(base64Chars[(val << (6 - bits)) & 0x3F]);
	}

	// 添加填充字符 '='
	while (encoded.size() % 4 != 0) {
		encoded.push_back('=');
	}

	return encoded;
}
std::string fromBase64(const std::string& encoded) {
	const std::string base64Chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::vector<uint8_t> decodedBytes;
	int val = 0, bits = 0;

	for (char c : encoded) {
		if (c == '=') break; // 忽略填充字符
		size_t pos = base64Chars.find(c);
		if (pos == std::string::npos) continue; // 忽略非法字符

		val = (val << 6) | pos;
		bits += 6;
		if (bits >= 8) {
			decodedBytes.push_back((val >> (bits - 8)) & 0xFF);
			bits -= 8;
		}
	}
	return std::string(decodedBytes.begin(), decodedBytes.end());
}
std::string toLatin1(const std::string& utf8Str) {
	std::string result;
	for (char c : utf8Str) {
		// 仅保留 0x00-0xFF 的字符（Latin-1 范围）
		if (static_cast<unsigned char>(c) <= 0xFF) {
			result += c;
		}
		else {
			result += '?'; // 替换无法表示的字符
		}
	}
	return result;
}
LCTDataCodec::LCTDataCodec()
{

}
LCTDataCodec::LCTDataCodec(const LCTDataCodec& codec)
{

}
LCTDataCodec LCTDataCodec::operator =(const LCTDataCodec& codec)
{
	return *this;
}


//序列化显示屏信息
std::string LCTDataCodec::SerializeDisplayScreenInfo(ST_DisplayScreenInfo info)
{
	std::string str = std::to_string(info.m_Index) + m_SplitTag
		+ std::to_string(info.m_ScreenGeometry.x) + m_SplitTag
		+ std::to_string(info.m_ScreenGeometry.y) + m_SplitTag
		+ std::to_string(info.m_ScreenGeometry.width) + m_SplitTag
		+ std::to_string(info.m_ScreenGeometry.height) + m_SplitTag
		+ std::to_string((int)info.m_ScreenType) + m_SplitTag
		+ std::to_string(info.m_ScanSize.width) + m_SplitTag
		+ std::to_string(info.m_ScanSize.height) + m_SplitTag
		+ std::to_string(info.m_MoudelSize.width) + m_SplitTag
		+ std::to_string(info.m_MoudelSize.height);
	return str;
}

//反序列化显示屏信息
bool LCTDataCodec::DeserializeDisplayScreenInfo(std::string sData, ST_DisplayScreenInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	if (strArray.size() < 7)
	{
		hloge("[ark][device][display_control]---sData:%s", sData.c_str());
		return false;
	}

	info.m_Index = std::stoi(strArray[0]);
	info.m_ScreenGeometry.x = std::stoi(strArray[1]);
	info.m_ScreenGeometry.y = std::stoi(strArray[2]);
	info.m_ScreenGeometry.width = std::stoi(strArray[3]);
	info.m_ScreenGeometry.height = std::stoi(strArray[4]);
	//info.m_ScreenType = (enum_DisplayScreenType)std::stoi(strArray[5]);
	if (strArray[5] == "standard") {
		info.m_ScreenType = enum_DisplayScreenType::standard;
	}

	if (strArray.size() >= 10)
	{
		info.m_ScanSize.width = std::stoi(strArray[6]);
		info.m_ScanSize.height = std::stoi(strArray[7]);
		info.m_MoudelSize.width = std::stoi(strArray[8]);
		info.m_MoudelSize.height = std::stoi(strArray[9]);
	}
	if (strArray.size() >= 11)
	{
		int length = std::stoi(strArray[10]);
		std::string jsonStr = "";
		for (int i = 11; i < length + 11 && i < strArray.size(); i++)
		{
			jsonStr += strArray[i];
		}

		nlohmann::json j;
		try {
			j = nlohmann::json::parse(jsonStr);
		}
		catch (std::exception& e) {
			hloge("[ark][device][display_control]---JSON解析失败:[%s],错误信息[%s]", jsonStr, e.what());
			return false;
		}
		if (j.is_array())
		{
			for(json var : j)
			{
				ST_MrvInfo mrvinfo;
				if (var.contains("MrvID") && !var["MrvID"].is_null())
				{
					mrvinfo.m_MrvID = var["MrvID"].get<std::string>();
				}
				if (var.contains("IsValid") && !var["IsValid"].is_null())
				{
					mrvinfo.m_IsValid = var["IsValid"].get<bool>();
				}
				if (var.contains("Guid") && !var["Guid"].is_null())
				{
					mrvinfo.m_Guid = var["Guid"].get<std::string>();
				}
				if (var.contains("XInPort") && !var["XInPort"].is_null())
				{
					mrvinfo.m_XInPort = var["XInPort"].get<int>();
				}
				if (var.contains("YInPort") && !var["YInPort"].is_null())
				{
					mrvinfo.m_YInPort = var["YInPort"].get<int>();
				}
				if (var.contains("ConnectIndex") && !var["ConnectIndex"].is_null())
				{
					mrvinfo.m_ConnectIndex = var["ConnectIndex"].get<int>();
				}
				if (var.contains("Height") && !var["Height"].is_null())
				{
					mrvinfo.m_Height = var["Height"].get<int>();
				}
				if (var.contains("Width") && !var["Width"].is_null())
				{
					mrvinfo.m_Width = var["Width"].get<int>();
				}
				if (var.contains("PortIndex") && !var["PortIndex"].is_null())
				{
					mrvinfo.m_PortIndex = var["PortIndex"].get<int>();
				}
				if (var.contains("SenderIndex") && !var["SenderIndex"].is_null())
				{
					mrvinfo.m_SenderIndex = var["SenderIndex"].get<int>();
				}
				if (var.contains("X") && !var["X"].is_null())
				{
					mrvinfo.m_X = var["X"].get<int>();
				}
				if (var.contains("Y") && !var["Y"].is_null())
				{
					mrvinfo.m_Y = var["Y"].get<int>();
				}

				info.m_MrvInfoList.push_back(mrvinfo);
			}

		}
	}

	return true;
}
//解析通讯口设备状态信息
bool LCTDataCodec::DeserializeDeviceStatusInfo(std::string sData, std::vector<ST_DeviceStatusInfo>& infoList)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.is_array())
	{
		for (auto& element : j.array())
		{
			if (element.is_object())
			{
				ST_DeviceStatusInfo info;
				info.portName = element.value("PortName", "");
				info.status = element.value("Status", 0);
				infoList.push_back(info);
			}
		}
	}

	return true;
}

//序列化显示区域信息
std::string LCTDataCodec::SerializeDisplaySection(ST_DisplayLoactionInfo info)
{
	int lum = info.m_intensity / 255.0 * 100;
	int color = (int)info.m_color + 1;		//1-r  2-g  3-b  4-w
	std::string str = std::to_string(info.getBegPixCol()) + m_SplitTag
		+ std::to_string(info.getBegPixRow()) + m_SplitTag
		+ std::to_string(info.getEndPixCol()) + m_SplitTag
		+ std::to_string(info.getEndPixRow()) + m_SplitTag
		+ "0" + m_SplitTag
		+ std::to_string(lum) + m_SplitTag
		+ std::to_string(color) + m_SplitTag
		+ std::to_string(info.m_pattern);
	return str;
}
//反序列化显示区域信息
bool LCTDataCodec::DeserializeDisplaySection(std::string sData, ST_DisplayLoactionInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	info.setBegPixCol(std::atoi(strArray[0].c_str()));
	info.setBegPixRow(std::atoi(strArray[1].c_str()));
	info.setEndPixCol(std::atoi(strArray[2].c_str()));
	info.setEndPixRow(std::atoi(strArray[3].c_str()));
	//空数据 strArray[4]默认均为0
	info.m_intensity = std::atoi(strArray[5].c_str());
	info.m_color = enum_LedColorType(std::atoi(strArray[6].c_str()) - 1);
	info.m_pattern = std::atoi(strArray[7].c_str());

	return true;
}

std::string LCTDataCodec::SerializePointDisplaySection(ST_DisplayLoactionInfo info)
{
	//测试pointbypoint
	int lum = info.m_intensity;
	int color = (int)info.m_color + 1;		//0-black 1-r  2-g  3-b  4-w
	std::string str = std::to_string(lum) + m_SplitTag
		+ std::to_string(color) + m_SplitTag
		+ m_SplitContentTag + std::to_string(info.getBegPixCol()) + m_SplitContentTag
		+ std::to_string(info.getBegPixRow()) + m_SplitContentTag
		+ std::to_string(info.getEndPixCol()) + m_SplitContentTag
		+ std::to_string(info.getEndPixRow()) + m_SplitContentTag;

	return str;
}

bool  LCTDataCodec::DeserializePointDisplaySection(std::string sData, ST_DisplayLoactionInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	info.m_intensity = std::atoi(strArray[0].c_str());
	info.m_color = enum_LedColorType(std::atoi(strArray[1].c_str()) - 1);
	info.setBegPixCol(std::atoi(strArray[2].c_str()));
	info.setBegPixRow(std::atoi(strArray[3].c_str()));
	info.setEndPixCol(std::atoi(strArray[4].c_str()));
	info.setEndPixRow(std::atoi(strArray[5].c_str()));

	return true;
}

//序列化自定义图片附加信息
std::string LCTDataCodec::SerializeCustomBitmapAddInfo(const ST_LocationInfo& locaitonInfo, enum_BmpShowModeType bmpShowMode)
{
	int width = locaitonInfo.getEndPixCol() - locaitonInfo.getBegPixCol() + 1;
	int height = locaitonInfo.getEndPixRow() - locaitonInfo.getBegPixRow() + 1;
	std::string str(std::to_string(locaitonInfo.getBegPixCol()) + m_SplitTag
		+ std::to_string(locaitonInfo.getBegPixRow()) + m_SplitTag
		+ std::to_string(width) + m_SplitTag
		+ std::to_string(height) + m_SplitTag
		+ std::to_string((int)bmpShowMode));
	return str;
}
//反序列化自定义图片附加信息
bool LCTDataCodec::DeserializeCustomBitmapAddInfo(std::string sData, ST_LocationInfo& locaitonInfo, enum_BmpShowModeType& bmpShowMode)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	int x = std::atoi(strArray[0].c_str());
	int y = std::atoi(strArray[1].c_str());
	int width = std::atoi(strArray[2].c_str());
	int height = std::atoi(strArray[3].c_str());
	locaitonInfo.setBegPixCol(x);
	locaitonInfo.setBegPixRow(y);
	locaitonInfo.setEndPixCol(x + width - 1);
	locaitonInfo.setEndPixRow(y + height - 1);
	bmpShowMode = (enum_BmpShowModeType)std::atoi(strArray[4].c_str());

	return true;
}

//序列化显示屏信息，为灯板writefalsh用
std::string LCTDataCodec::SerializeScreenInfo(const ST_LocationInfo& info)
{
	std::string str;
	str = std::to_string(info.getBegPixCol()) + m_SplitTag
		+ std::to_string(info.getBegPixRow()) + m_SplitTag
		+ std::to_string(info.getEndPixCol()) + m_SplitTag
		+ std::to_string(info.getEndPixRow());
	return str;
}

// 9005/9937芯片低灰阶校正
std::string LCTDataCodec::SerializeScreenLCInfo(const ST_LocationInfo& info)
{
	std::string str;
	str = std::to_string(info.getBegPixCol()) + m_SplitTag
		+ std::to_string(info.getBegPixRow()) + m_SplitTag
		+ std::to_string(info.getEndPixCol() - info.getBegPixCol() + 1) + m_SplitTag
		+ std::to_string(info.getEndPixRow() - info.getBegPixRow() + 1);
	str += m_SplitTag + "false"; // 添加是否使能参数占位，视芯低灰校正方案详细设计标识无用
	str += m_SplitTag + "1"; // 添加校正模式参数占位，视芯低灰校正方案详细设计标识无用
	return str;
}

std::string LCTDataCodec::SerializeCheckerBoardInfo(const ST_DisplayCheckerBoard& checkerBoardInfo)
{
	//	举例："1/0,0,105,105/30,30/90,90/255,255,255"
	std::string str = std::to_string(checkerBoardInfo.m_flag) + m_SplitTag
		+ std::to_string(checkerBoardInfo.m_screenArea.x) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_screenArea.y) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_screenArea.width) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_screenArea.height) + m_SplitTag
		+ std::to_string(checkerBoardInfo.m_checkerBoardSize.width) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_checkerBoardSize.height) + m_SplitTag
		+ std::to_string(checkerBoardInfo.m_endWAndEndH.width) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_endWAndEndH.height) + m_SplitTag
		+ std::to_string(checkerBoardInfo.m_rgbValue.r) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_rgbValue.g) + m_SplitContentTag
		+ std::to_string(checkerBoardInfo.m_rgbValue.b);

	return str;
}

std::string LCTDataCodec::SerializeSolidifyScanInfo(ST_SolidifyAddressInfo info)
{
	std::string str = std::to_string(info.m_sendAddr) + m_SplitTag
		+ std::to_string(info.m_portAddr) + m_SplitTag
		+ std::to_string(info.m_scanAddr);
	return str;
}

bool LCTDataCodec::DeserializeSolidifyScanInfo(std::string sData, ST_SolidifyAddressInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	info.m_sendAddr = std::atoi(strArray[0].c_str());
	info.m_portAddr = std::atoi(strArray[1].c_str());
	info.m_scanAddr = std::atoi(strArray[2].c_str());

	return true;
}

std::string LCTDataCodec::SerialReloadCoefficients(const ST_LocationInfo& info)
{
	int width = info.getEndPixCol() - info.getBegPixCol() + 1;
	int height = info.getEndPixRow() - info.getBegPixRow() + 1;
	std::string str = std::to_string(info.getBegPixCol()) + m_SplitTag
		+ std::to_string(info.getBegPixRow()) + m_SplitTag
		+ std::to_string(width) + m_SplitTag
		+ std::to_string(height);
	return str;
}

bool LCTDataCodec::DeserializeReloadCoefficients(std::string sData, ST_LocationInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	if (strArray.size() < 4)
	{
		return false;
	}
	int x = std::atoi(strArray[0].c_str());
	int y = std::atoi(strArray[1].c_str());
	int width = std::atoi(strArray[2].c_str());
	int height = std::atoi(strArray[3].c_str());
	info.setBegPixCol(x);
	info.setBegPixRow(y);
	info.setEndPixCol(x + width - 1);
	info.setEndPixRow(y + height - 1);

	return true;
}

std::string LCTDataCodec::SerialCorrectModeLowLum(bool isEnableCorrect, enum_CorrectMode correctMode)
{
	std::string str;
	if (isEnableCorrect)
	{
		str = "True" + m_SplitTag + std::to_string((int)correctMode) + m_SplitTag;
	}
	else
	{
		str = "False" + m_SplitTag + std::to_string((int)correctMode) + m_SplitTag;
	}
	return str;
}

std::string LCTDataCodec::SerialCorrectMode(bool isEnableCorrect, enum_CorrectMode correctMode, bool isChromaCorrentionLowGray)
{
	int modeIndex = 0;
	switch (correctMode)
	{
		case enum_CorrectMode::CorrectModeChroma:	modeIndex = 0; break;
		case enum_CorrectMode::CorrectModeLum:		modeIndex = 1; break;
		default: break;
	}
	std::string str;
	if (isEnableCorrect)
	{
		str = "True" + m_SplitTag + std::to_string(modeIndex) + m_SplitTag;
	}
	else
	{
		str = "False" + m_SplitTag + std::to_string(modeIndex) + m_SplitTag;
	}
	if (isChromaCorrentionLowGray)
	{
		str = str + "True";
	}
	else
	{
		str = str + "False";
	}
	return str;
}

bool LCTDataCodec::DeserialCorrectMode(std::string sData, bool& isEnableCorrect, enum_CorrectMode& correctMode, bool& isChromaCorrentionLowGray)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	isEnableCorrect = (bool)std::stoi(strArray[0].c_str());
	if (std::stoi(strArray[1]) == 0)
	{
		correctMode = enum_CorrectMode::CorrectModeChroma;
	}
	else
	{
		correctMode = enum_CorrectMode::CorrectModeLum;
	}
	isChromaCorrentionLowGray = (bool)std::stoi(strArray[2]);

	return true;
}

std::string LCTDataCodec::SerialGetRectAverCoefs(const ST_LocationInfo& info)
{
	return SerialReloadCoefficients(info);
}

bool LCTDataCodec::DeserialGetRectAverCoefs(std::string sData, ST_LocationInfo& info)
{
	return DeserializeReloadCoefficients(sData, info);
}

std::string LCTDataCodec::SerialFirstScannerIDData(ST_ScannerID scannerID)
{
	std::string str;
	if (scannerID.m_isValid)
	{
		str = "True";
	}
	else
	{
		str = "False";
	}
	str = str + m_SplitTag
		+ scannerID.m_userModeID + m_SplitTag
		+ scannerID.m_scannerGUID;
	return str;
}

bool LCTDataCodec::DeserialFirstScannerIDData(std::string sData, ST_ScannerID& scannerID)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	if (strArray.size() < 3)
	{
		return false;
	}
	scannerID.m_isValid = (bool)std::stoi(strArray[0].c_str());
	scannerID.m_userModeID = strArray[1];
	scannerID.m_scannerGUID = strArray[2];

	return true;
}

std::string LCTDataCodec::SerialModuleFlashOperation(const ST_LocationInfo& info, enum_ModuleFlashOperation operation)
{
	int width = info.getEndPixCol() - info.getBegPixCol() + 1;
	int height = info.getEndPixRow() - info.getBegPixRow() + 1;
	std::string str = std::to_string(info.getBegPixCol()) + m_SplitTag
		+ std::to_string(info.getBegPixRow()) + m_SplitTag
		+ std::to_string(width) + m_SplitTag
		+ std::to_string(height) + m_SplitTag
		+ std::to_string((int)operation) + m_SplitTag
		+ std::to_string(19469);
	return str;
}

bool LCTDataCodec::DeserialModelFlashOperation(std::string sData, enum_ModuleFlashOperation& operation, ST_LocationInfo& info)
{
	std::vector<std::string> strArray = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	int x = std::stoi(strArray[0].c_str());
	int y = std::stoi(strArray[1].c_str());
	int width = std::stoi(strArray[2].c_str());
	int height = std::stoi(strArray[3].c_str());
	info.setBegPixCol(x);
	info.setBegPixRow(y);
	info.setEndPixCol(x + width - 1);
	info.setEndPixRow(y + height - 1);
	operation = (enum_ModuleFlashOperation)std::stoi(strArray[4].c_str());

	return true;
}

//序列化Rect
std::string LCTDataCodec::SerialRect(ark::utility::Rect rect)
{
	std::string x = std::to_string(rect.x);
	std::string y = std::to_string(rect.y);
	std::string w = std::to_string(rect.width);
	std::string h = std::to_string(rect.height);
	std::string str = x + ", " + y + ", " + w + ", " + h;
	return str;
}
//反序列化Rect
bool LCTDataCodec::DeserialRect(std::string sData, ark::utility::Rect& rect)
{
	auto strList = ark::utility::StringFormat::splitString(sData, ", ");
	//rect.setRect(strList[0], strList[1].c_str(), strList[2].c_str(), strList[3].c_str());
	rect.x = std::atoi(strList[0].c_str());
	rect.y = std::atoi(strList[1].c_str());
	rect.width = std::atoi(strList[2].c_str());
	rect.height = std::atoi(strList[3].c_str());

	return true;
}
//序列化Size
std::string LCTDataCodec::SerialSize(ark::utility::Size size)
{
	std::string str = std::to_string(size.width) + ", " + std::to_string(size.height);
	return str;
}
//反序列化Size
bool LCTDataCodec::DeserialSize(std::string sData, ark::utility::Size& size)
{
	auto strList = ark::utility::StringFormat::splitString(sData, ", ");
	size.width = std::atoi(strList[0].c_str());
	size.height = std::atoi(strList[1].c_str());

	return true;
}
//序列化Point
std::string LCTDataCodec::SerialPoint(ark::utility::Point point)
{
	std::string str = std::to_string(point.x) + ", " + std::to_string(point.y);
	return str;
}
//反序列化Point
bool LCTDataCodec::DeserialPoint(std::string sData, ark::utility::Point& point)
{
	auto strList = ark::utility::StringFormat::splitString(sData, ", ");
	point.x = std::atoi(strList[0].c_str());
	point.y = std::atoi(strList[1].c_str());

	return true;
}

//序列化隔点打屏
std::string LCTDataCodec::SerialHWDisplayParameterPattern(ST_HWDisplayParameterPattern info)
{

	json jsonObj;
	json jsonObjSon;

	jsonObjSon["IntervalPixelsX"] = info.m_IntervalPixelsX;
	jsonObjSon["IntervalPixelsY"] = info.m_IntervalPixelsY;
	jsonObjSon["Position"] = SerialPoint(info.m_Position);
	jsonObjSon["DisplayRectInfo"] = SerialRect(info.m_Rect);
	jsonObjSon["BrightValue"] = info.m_BrightValue;
	jsonObjSon["RedBrightValue"] = info.m_RedBrightValue;
	jsonObjSon["GreenBrightValue"] = info.m_GreenBrightValue;
	jsonObjSon["BlueBrightValue"] = info.m_BlueBrightValue;
	jsonObjSon["PatternType"] = info.m_PatternType;

	jsonObj["PatternType"] = info.m_PatternType;
	jsonObj["PatternParameter"] = jsonObjSon;
	jsonObj["BlockParameter"] = nullptr;

	std::string str = jsonObj.dump();
	return str;
}
//序列化隔块打屏
std::string LCTDataCodec::SerialHWDisplayParameterBlock(ST_HWDisplayParameterBlock info)
{

	json jsonObj;
	json jsonObjSon;

	jsonObjSon["IntervalPixelsX"] = info.m_IntervalPixelsX;
	jsonObjSon["IntervalPixelsY"] = info.m_IntervalPixelsY;
	jsonObjSon["Position"] = SerialPoint(info.m_Position);
	jsonObjSon["DisplayRectInfo"] = SerialRect(info.m_Rect);
	jsonObjSon["BrightValue"] = info.m_BrightValue;
	jsonObjSon["RedBrightValue"] = info.m_RedBrightValue;
	jsonObjSon["GreenBrightValue"] = info.m_GreenBrightValue;
	jsonObjSon["BlueBrightValue"] = info.m_BlueBrightValue;
	jsonObjSon["PatternType"] = info.m_PatternType;
	jsonObjSon["PatternParameter"] = nullptr;
	jsonObjSon["BlockParameter"] = jsonObjSon;

	std::string str = jsonObjSon.dump();
	return str;
}

std::string LCTDataCodec::SerialHWDisplayParameterClose(ST_HWDisplayParameterClose info)
{
	json jsonObjSon;
	jsonObjSon["PatternType"] = info.m_PatternType;

	std::string str = jsonObjSon.dump();
	return str;
}
std::string LCTDataCodec::SerialPassthroughPack(ST_PassthroughPack pack)
{

	json jsonObjSon;
	jsonObjSon["RequestType"] = pack.m_RequestType;
	jsonObjSon["ComPort"] = pack.m_ComPort;
	jsonObjSon["SenderIndex"] = pack.m_SenderIndex;
	jsonObjSon["PortAddr"] = pack.m_PortAddr;
	jsonObjSon["ScanAddr"] = pack.m_ScanAddr;
	jsonObjSon["Data"] = pack.m_Data;
	jsonObjSon["DataLength"] = pack.m_DataLength;
	jsonObjSon["PackSN"] = pack.m_PackSN;
	jsonObjSon["RegisterAddr"] = ((int64_t)pack.m_RegisterAddr) & 0XFFFFFFFF;
	jsonObjSon["TimeOut"] = pack.m_TimeOut;
	jsonObjSon["PassthroughRequestResult"] = pack.m_PassthroughRequestResult;
	jsonObjSon["UserToken"] = nullptr;

	std::string str = jsonObjSon.dump();
	return str;
}

bool LCTDataCodec::DeserialPassthroughPack(std::string sData, ST_PassthroughPack& pack)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.is_object())
	{
		if (j.contains("RequestType"))
		{
			pack.m_RequestType = (enum_RequestType)j["RequestType"].get<int>();
		}
		if (j.contains("ComPort"))
		{
			pack.m_ComPort = j["RequestType"].get<std::string>();
		}
		if (j.contains("SenderIndex"))
		{
			pack.m_SenderIndex = j["SenderIndex"].get<short>();
		}
		if (j.contains("PortAddr"))
		{
			pack.m_PortAddr = j["PortAddr"].get<short>();
		}
		if (j.contains("ScanAddr"))
		{
			pack.m_ScanAddr = j["ScanAddr"].get<uint16_t>();
		}
		if (j.contains("Data"))
		{
			pack.m_Data = fromBase64(toLatin1(j["Data"].get<std::string>()));
		}
		if (j.contains("DataLength"))
		{
			pack.m_DataLength = j["DataLength"].get<int>();
		}
		if (j.contains("PackSN"))
		{
			pack.m_PackSN = j["PackSN"].get<std::string>();
		}
		if (j.contains("RegisterAddr"))
		{
			pack.m_RegisterAddr = j["RegisterAddr"].get<uint32_t>();
		}
		if (j.contains("TimeOut"))
		{
			pack.m_TimeOut = j["TimeOut"].get<int>();
		}
		if (j.contains("PassthroughRequestResult"))
		{
			pack.m_PassthroughRequestResult = (enum_PassthroughRequestResult)j["RegisterAddr"].get<int>();
		}
	}

	return true;
}
//序列化Gamma信息
std::string LCTDataCodec::SerialGammaInfo(ST_GammaInfo gammaInfo)
{
	std::string gammaTable = gammaInfo.m_GammaTable;
	std::string gammaArray = toLatin1(gammaTable);
	gammaArray = toBase64(gammaArray);
	std::string gamma(gammaArray);

	json jsonObjSon;
	jsonObjSon["GammaValue"] = gammaInfo.m_GammaValue;
	jsonObjSon["GammaTable"] = gamma;


	std::string str = jsonObjSon.dump();
	return str;
}

std::string LCTDataCodec::SerialNotifyRectAverCoefs(uint16_t* m_AvgCoef)
{
	//暂不实现
	return "";
}
//
///**
//* @brief        反序列化获取的矩阵平均校正系数
//* @return       None
//* @param[in]    原始字符串
//* @param[out]   平均系数
//* @note         None
//* @author       zhangzhen 2017.11.06
//*/
bool LCTDataCodec::DeserializeNotifyRectAverCoefs(const std::string& sData, std::vector<uint16_t>& coefs)
{

	std::vector<std::string> strList = ark::utility::StringFormat::splitString(sData, m_SplitTag);
	if (strList.size() != 19)
	{
		return false;
	}
	strList.erase(strList.begin());
	coefs = std::vector<uint16_t>(9);
	for (int i = 0; i < 9; i++)
	{
		coefs[i] = std::atoi(strList[2 * i].c_str()) + std::atoi(strList[2 * i + 1].c_str()) * 256;
	}
	return true;
}
//
///**
//* @brief        反序列化大屏新接口-显示屏个数
//* @return       None
//* @param[in]    原始Json格式数据字符串
//* @param[out]   显示屏信息List
//* @note         None
//* @author       zhangzhen 2017.11.06
//*/
bool LCTDataCodec::DeserializeDisplayCountNew(std::vector<std::string>& diplayList, std::string strDisplayJson)
{

	if (strDisplayJson == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(strDisplayJson);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.is_array())
	{
		for (auto& element : j.array())
		{
			if (element.is_object())
			{
				diplayList.push_back(element.get<std::string>());
			}
		}
	}

	return true;
}

bool LCTDataCodec::DeserializeComboScreenInfo(ST_DisplayCombScreenInfo& outInfo, std::string data)
{

	if (data == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(data);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.is_object())
	{
		for (auto& [key, value] : j.items())
		{

			if (key.compare("Index") == 0)
			{
				outInfo.m_index = value.get<int>();
			}
			else if (key.compare("ScreenName") == 0)
			{
				outInfo.m_screenName = value.get<std::string>();
			}
			else if (key.compare("ScreenSN") == 0)
			{
				outInfo.m_screenSN = value.get<std::string>();
			}
			else if (key.compare("ScreenX") == 0)
			{
				outInfo.m_screenGeometry.x = value.get<int>();;
			}
			else if (key.compare("ScreenY") == 0)
			{
				outInfo.m_screenGeometry.y = value.get<int>();;
			}
			else if (key.compare("ScreenWidth") == 0)
			{
				outInfo.m_screenGeometry.width = value.get<int>();;
			}
			else if (key.compare("ScreenHeight") == 0)
			{
				outInfo.m_screenGeometry.height = value.get<int>();;
			}
			else if (key.compare("DisplayScreenInfos") == 0)
			{
				//QJsonArray array = value.toArray();
				//auto array = value.array();

				for (int i = 0; i < value.size(); i++)
				{
					if (value[i].is_object())
					{
						ST_DisplayOneScreenInfo screenInfo;
						//auto childObj = value[i];
						//QJsonObject::iterator childObjIt = childObj.begin();
						for (auto& [k, v] : value[i].items())
						{
							//QJsonValue childValue = childObjIt.value();
							if (k.compare("ScreenX") == 0)
							{
								screenInfo.ScreenX = v.get<int>();
							}
							else if (k.compare("ScreenY") == 0)
							{
								screenInfo.ScreenY = v.get<int>();
							}
							else if (k.compare("DisplayScreenInfo") == 0)
							{
								if (v.is_object())
								{
									ST_DisplayScreenInfo screenSubInfo;
									//QJsonObject grandsonObj = childValue.toObject();
									//QJsonObject::iterator grandsonObjIt = grandsonObj.begin();
									for (auto& [k1, v1] : v.items())
									{
										if (k1.compare("Index") == 0)
										{
											screenSubInfo.m_Index = v1.get<int>();
										}
										else if (k1.compare("CommPort") == 0)
										{
											screenSubInfo.m_CommPort = v1.get<std::string>();
										}
										else if (k1.compare("ScreenX") == 0)
										{
											screenSubInfo.m_ScreenGeometry.x = v1.get<int>();
										}
										else if (k1.compare("ScreenY") == 0)
										{
											screenSubInfo.m_ScreenGeometry.y = v1.get<int>();
										}
										else if (k1.compare("ScreenWidth") == 0)
										{
											screenSubInfo.m_ScreenGeometry.width = v1.get<int>();
										}
										else if (k1.compare("ScreenHeight") == 0)
										{
											screenSubInfo.m_ScreenGeometry.height = v1.get<int>();
										}
										else if (k1.compare("ScanWidth") == 0)
										{
											screenSubInfo.m_ScanSize.width = v1.get<int>();
										}
										else if (k1.compare("ScanHeigth") == 0)
										{
											screenSubInfo.m_ScanSize.height = v1.get<int>();
										}
										else if (k1.compare("MoudelWidth") == 0)
										{
											screenSubInfo.m_MoudelSize.width = v1.get<int>();
										}
										else if (k1.compare("MoudelHeight") == 0)
										{
											screenSubInfo.m_MoudelSize.height = v1.get<int>();
										}
										else if (k1.compare("ScreenType") == 0)
										{
											screenSubInfo.m_ScreenType = enum_DisplayScreenType(v1.get<int>());
										}
										else if (k1.compare("MrvInfoList") == 0)
										{
											//后续添加;
										}
										else {}
									}

									screenInfo.m_DisplayScreenInfo = screenSubInfo;
								}
							}
						}

						outInfo.m_displayScreenInfos.push_back(screenInfo);
					}
				}
			}
		}
	}
	return true;
}
//
///**
//* @brief        反序列化写入灯板FLashEx结果通知
//* @return       None
//* @param[in]    原始Json格式数据字符串
//* @param[out]   灯板Flash写入结果及具体信息
//* @note         None
//* @author       zhangzhen 2017.11.07
//*/
bool LCTDataCodec::DeserializeModuleFlashNotifyEx(std::string sData, ST_ModuleFlashNotifyParamsEx& ParamsEx)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.contains("IsAllOK"))
	{
		if (j["IsAllOK"].get<bool>())
		{
			ParamsEx.m_isAllOK = true;
			return false;
		}
		else
		{
			ParamsEx.m_isAllOK = false;
		}
	}

	if (j.contains("AllErrorInfo"))
	{
		auto value = j["AllErrorInfo"];
		if (value.is_array())
		{

			for (auto& v : value)
			{
				ST_ErrorModuleFlashInfo  errorInfo;
				if (v.contains("Result"))
				{
					errorInfo.m_ModuleFlashOperationRes = enum_ModuleFlashOperationRes(v["Result"].get<int>());
				}
				if (v.contains("ModuleRectInScreen"))
				{
					DeserialRect(v["ModuleRectInScreen"].get<std::string>(), errorInfo.m_Rect);
				}
				if (v.contains("PortIndex"))
				{
					errorInfo.m_PortIndex = v["PortIndex"].get<int>();
				}
				if (v.contains("ScannerIndex"))
				{
					errorInfo.m_ScannerIndex = v["ScannerIndex"].get<int>();
				}
				if (v.contains("SenderIndex"))
				{
					errorInfo.m_SenderIndex = v["SenderIndex"].get<int>();
				}
				ParamsEx.m_AllErrorInfo.push_back(errorInfo);
			}
		}
	}

	return true;
}

//解析收到的gamma表回包
bool LCTDataCodec::DeserializeGammaTable(std::map<enum_LedColorType, std::vector<unsigned int>>& gammaMap, ark::device::CommResults& result, std::string data)
{
	if (data == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(data);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.contains("Result"))
	{
		result = CommResults::OK;
		if (j["Result"].get<std::string>().compare("ok") != 0)
		{
			result = CommResults::RecvErrResp;
		}
	}
	if (j.contains("R"))
	{
		if (j["R"].is_array())
		{
			std::vector<unsigned int> redGammaList;
			for (auto& a : j["R"])
			{
				redGammaList.push_back(a.get<int>());
			}

			gammaMap.insert(make_pair(enum_LedColorType::LedColor_Red, redGammaList));
		}
	}
	if (j.contains("G"))
	{
		if (j["G"].is_array())
		{
			std::vector<unsigned int> redGammaList;
			for (auto& a : j["G"])
			{
				redGammaList.push_back(a.get<int>());
			}

			gammaMap.insert(make_pair(enum_LedColorType::LedColor_Green, redGammaList));
		}
	}
	if (j.contains("B"))
	{
		if (j["B"].is_array())
		{
			std::vector<unsigned int> redGammaList;
			for (auto& a : j["B"])
			{
				redGammaList.push_back(a.get<int>());
			}

			gammaMap.insert(make_pair(enum_LedColorType::LedColor_Blue, redGammaList));
		}
	}

	return true;
}

bool LCTDataCodec::DeserializeFeaturesSupport(std::map< enum_LCTSupportFeatureList, bool>& featureList, std::string data)
{

	if (data == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(data);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}

	if (j.contains("MultiLayerCorrection"))
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_MULTILAYER, j["MultiLayerCorrection"].get<bool>()));
	}
	if (j.contains("XBitMode"))
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_XBITMODE, j["XBitMode"].get<bool>()));
	}
	if (j.contains("FineGrayAdjustVer3"))//画质引擎3.0
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_GRAYSCALE, j["FineGrayAdjustVer3"].get<bool>()));
	}
	if (j.contains("Is18BitMode")) {
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_IS18BITMODE, j["Is18BitMode"].get<bool>()));
	}

	//自适应
	if (j.contains("AdaptiveCorrection")) {
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_ADAPTIVE, j["AdaptiveCorrection"].get<bool>()));
	}

	// add 亮暗线功能
	if (j.contains("BrightDarkLineFix"))
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_BrightDarkLineFix, j["BrightDarkLineFix"].get<bool>()));
	}
	if (j.contains("BrightDarkLineFixCoefUploadSpeedUp"))
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_BrightDarkLineFixCoefUploadSpeedUp, j["BrightDarkLineFixCoefUploadSpeedUp"].get<bool>()));
	}
	if (j.contains("SetBrightDarkLineFixState"))
	{
		featureList.insert(std::make_pair(enum_LCTSupportFeatureList::F_SetBrightDarkLineFixState, j["SetBrightDarkLineFixState"].get<bool>()));
	}

	return true;
}
//
//LCTDataCodec::LCTDataCodec(const LCTDataCodec& codec)
//{
//
//}
//
//LCTDataCodec LCTDataCodec::operator =(const LCTDataCodec& codec)
//{
//	return *this;
//}
//
//
std::string LCTDataCodec::SerialSubPixHWDisplayParameterPattern(ST_HWDisplayParameterPatternExtend& info)
{
	json jsonObj;
	json jsonObjSon;

	// 构建子对象 ST_SubPixInfo
	jsonObj["is_SubPix"] = static_cast<int>(info.m_SubPixInfo.m_isSubPix);
	jsonObj["SubLightNum"] = static_cast<int>(info.m_SubPixInfo.m_subPixLTNum);
	jsonObj["SubpixelLightPointMode"] = static_cast<int>(info.m_SubPixInfo.m_LTLayout);
	jsonObj["SubPixLightLoc"] = info.m_dispLightLocFlag; // 根据实际类型可能需要转换
	jsonObj["LightPointLayout"] = static_cast<int>(info.m_SubPixInfo.m_LPLayout);

	// 将子对象添加到父对象
	jsonObjSon["ST_SubPixInfo"] = jsonObj;

	// 添加其他字段
	jsonObjSon["IntervalPixelsX"] = info.m_IntervalPixelsX;
	jsonObjSon["IntervalPixelsY"] = info.m_IntervalPixelsY;
	jsonObjSon["Position"] = SerialPoint(info.m_Position);
	jsonObjSon["DisplayRectInfo"] = SerialRect(info.m_Rect);
	jsonObjSon["BrightValue"] = static_cast<int>(info.m_BrightValue);
	jsonObjSon["RedBrightValue"] = static_cast<int>(info.m_RedBrightValue);
	jsonObjSon["GreenBrightValue"] = static_cast<int>(info.m_GreenBrightValue);
	jsonObjSon["BlueBrightValue"] = static_cast<int>(info.m_BlueBrightValue);
	jsonObjSon["PatternType"] = static_cast<int>(enum_DisplayPatternType::SubpixelDotSpacingScreen);

	// 生成紧凑格式的 JSON 字符串
	std::string str = jsonObjSon.dump();
	return str;
}

std::string LCTDataCodec::SerialSubPixCheckerBoardHWDisplayParameterPattern(ST_DisplayCheckerBoardExtend info)
{
	json jsonObj;
	json jsonObjSon;

	jsonObj["is_SubPix"] = int(info.m_SubPixInfo.m_isSubPix);
	jsonObj["SubLightNum"] = int(info.m_SubPixInfo.m_subPixLTNum);
	jsonObj["SubpixelLightPointMode"] = int(info.m_SubPixInfo.m_LTLayout);
	jsonObj["SubPixLightLoc"] = info.m_dispLightLocFlag;
	jsonObjSon["ST_SubPixInfo"] = jsonObj;
	jsonObjSon["Odevity"] = info.m_flag;
	jsonObjSon["PatternType"] = enum_DisplayPatternType::SubpixelChessboardScreen;
	json selectArea;
	selectArea["BegPixelCol"] = info.m_screenArea.x;
	selectArea["BegPixelRow"] = info.m_screenArea.y;
	selectArea["PixelWidth"] = info.m_screenArea.width;
	selectArea["PixelHeight"] = info.m_screenArea.height;
	jsonObjSon["SelectArea"] = selectArea;
	json checkerboardSize;
	checkerboardSize["Width"] = info.m_checkerBoardSize.width;
	checkerboardSize["Height"] = info.m_checkerBoardSize.height;
	jsonObjSon["CheckerboardSize"] = checkerboardSize;
	json checkerboardNormal;
	checkerboardNormal["AreaWidth"] = info.m_endWAndEndH.width;
	checkerboardNormal["AreaHeight"] = info.m_endWAndEndH.height;
	jsonObjSon["CheckerboardAreaNormal"] = checkerboardNormal;
	json RGBComponentValue;
	RGBComponentValue["RedBrightValue"] = info.m_rgbValue.r;
	RGBComponentValue["GreenBrightValue"] = info.m_rgbValue.g;
	RGBComponentValue["BlueBrightValue"] = info.m_rgbValue.b;
	jsonObjSon["RGBComponentValue"] = RGBComponentValue;

	std::string str = jsonObjSon.dump();
	return str;
}

bool LCTDataCodec::DeserializeSubPixInfo(std::string sData, ST_SubPixInfo& subPixInfo)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}


	if (j.contains("is_SubPix") && !j["is_SubPix"].is_null())
	{
		subPixInfo.m_isSubPix = (enum_SubPixSup)(j["is_SubPix"].get<int>());
	}
	if (j.contains("SubLightNum") && !j["SubLightNum"].is_null())
	{
		subPixInfo.m_subPixLTNum = (enum_SubPixLightNum)(j["SubLightNum"].get<int>());
	}
	if (j.contains("SubpixelLightPointMode") && !j["SubpixelLightPointMode"].is_null())
	{
		subPixInfo.m_LTLayout = (enum_LightLayout)(j["SubpixelLightPointMode"].get<int>());
	}
	if (j.contains("LightPointLayout") && !j["LightPointLayout"].is_null())
	{
		subPixInfo.m_LPLayout = (enum_LightPointLayout)(j["LightPointLayout"].get<int>());
	}

	return true;
}
//序列化相机亮暗线系数信息
std::string LCTDataCodec::SerializeDarkLineCamera(const std::string& coeffData)
{
	if (coeffData.empty())
	{
		return "";
	}
	json jsonObj;
	json jsonObjSon;

	//解析出缝隙条数和缝隙系数
	std::stringstream ss1(coeffData);
	std::string lineNum, coeff, rest;

	std::getline(ss1, lineNum, ',');
	std::getline(ss1, rest);
	std::stringstream ss2(rest);

	std::string token;
	while (std::getline(ss2, token, ',')) {
		coeff += token;
		coeff += ",";
	}
	coeff.pop_back(); //去掉最后一个逗号

	// 亮暗线个数 = 系数数据/5
	int SeamNumber = (std::atoi(lineNum.c_str())) / 5;
	std::string coefData = coeff;

	jsonObj["SeamNumber"] = SeamNumber;
	jsonObj["GapsDate"] = coefData;

	std::string str = jsonObj.dump();
	return str;
}
//反序列化画质引擎存在信息
bool LCTDataCodec::DeserializeImageEngineExistInfo(std::string sData, bool& state)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	bool colorMangement = false;
	bool fineGray = false;
	if (j.is_object()) {
		if (j.contains("ColorMangement")) {
			colorMangement = j["ColorMangement"].get<bool>();
		}
		if (j.contains("FineGray")) {
			fineGray = j["FineGray"].get<bool>();
		}
	}
	if (colorMangement || fineGray)
	{
		state = true;
	}
	else
	{
		state = false;
	}

	return true;
}
//反序列化画质引擎开启信息
bool LCTDataCodec::DeserializeImageEngineOpenInfo(std::string sData, bool& state)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	bool xbitSwitch = false;
	bool fineGraySwitch = false;
	bool eighteenbitSwitch = false;
	if (j.is_object()) {
		if (j.contains("XbitSwitch")) {
			xbitSwitch = j["XbitSwitch"].get<bool>();
		}
		if (j.contains("FineGraySwitch")) {
			fineGraySwitch = j["FineGraySwitch"].get<bool>();
		}
		if (j.contains("EighteenbitSwitch")) {
			eighteenbitSwitch = j["EighteenbitSwitch"].get<bool>();
		}
	}
	if (xbitSwitch || fineGraySwitch)
	{
		state = true;
	}
	else
	{
		state = false;
	}

	return true;
}
//反序列化箱体编号信息
bool LCTDataCodec::DeserializeCabinetId(std::string sData, std::vector<std::string>& idList)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	std::vector<std::string> cabIdList;
	if (j.is_array())
	{
		for (auto& a : j)
		{
			if (a.contains("UserModeID"))
			{
				cabIdList.push_back(a["UserModeID"].get<std::string>());
			}
		}
		idList = cabIdList;
	}

	return true;
}
//反序列化模组拓扑信息
bool LCTDataCodec::DeserializeModuleTopologyInfo(std::string sData, std::vector<ST_ModuleTopologyInfo>& moduleTopologyInfo)
{
	if (sData == "") {
		return false;
	}
	json j;
	try {
		j = nlohmann::json::parse(sData);
	}
	catch (std::exception& e) {
		hloge("[ark][device][display_control]---nlohmann parse json error, errMsg=%s", e.what());
		return false;
	}
	if (j.is_array())
	{
		for (auto& a : j)
		{
			int recvCardstartX = a["StartX"].get<int>();
			int recvCardstartY = a["StartY"].get<int>();

			if (a.contains("ModuleTopologyInfoList"))
			{
				for (auto& obj : a["ModuleTopologyInfoList"])
				{
					ST_ModuleTopologyInfo info;
					if (obj.contains("id")) {
						info.moduleID = obj["id"].get<std::string>();
					}
					//LCT只给了相对于接口卡的坐标，这里需要转换成相对于屏幕的坐标
					if (obj.contains("x")) {
						info.modulex = obj["x"].get<int>();
					}
					if (obj.contains("y")) {
						info.moduley = obj["y"].get<int>();
					}
					if (obj.contains("width")) {
						info.modulew = obj["width"].get<int>();
					}
					if (obj.contains("height")) {
						info.moduleh = obj["height"].get<int>();
					}
					moduleTopologyInfo.push_back(info);
				}
			}
		}
	}

	return true;
}
//序列化模组编号写入信息
std::string LCTDataCodec::SerializeWriteModuleId(ST_LocationInfo info, int moduleWidth, int moduleHeight, const std::vector<std::string>& idList)
{
	json jsonObj;
	json jsonObjPixelArea;
	json jsonObjModuleSize;
	//区域信息
	jsonObjPixelArea["BegPixelCol"] = info.getBegPixCol();
	jsonObjPixelArea["BegPixelRow"] = info.getBegPixRow();
	jsonObjPixelArea["EndPixelCol"] = info.getEndPixCol();
	jsonObjPixelArea["EndPixelRow"] = info.getEndPixRow();
	//模组信息
	jsonObjModuleSize["Width"] = moduleWidth;
	jsonObjModuleSize["Height"] = moduleHeight;

	json jsonArray = json::array();
	for (const std::string& str : idList) {
		jsonArray.push_back(str);
	}

	jsonObj["PixelArea"] = jsonObjPixelArea;
	jsonObj["ModuleSize"] = jsonObjModuleSize;
	jsonObj["ModuleIDArr"] = jsonArray;

	std::string str = jsonObj.dump();
	return str;
}
//序列化箱体编号写入信息
std::string LCTDataCodec::SerializeWriteCabinetId(ST_DisplayScreenInfo info, std::map<std::string, std::string> cabinetIDMap)
{
	for (int i = 0; i < info.m_MrvInfoList.size(); i++) 
	{
		info.m_MrvInfoList[i].m_Guid = createUuid();
		info.m_MrvInfoList[i].m_IsValid = true;
		std::string cabinetName = cabinetIDMap[info.m_MrvInfoList[i].m_MrvID];

		//XY坐标设置为0，解决一箱多卡问题
		info.m_MrvInfoList[i].m_MrvID = cabinetName + "@0@0@" +
			std::to_string(info.m_MrvInfoList[i].m_Width) + "@" +
			std::to_string(info.m_MrvInfoList[i].m_Height) + "@";

	}

	json jsonObjSon;
	jsonObjSon["Index"] = info.m_Index;
	if (info.m_CommPort == "" /*|| info.m_CommPort == NULL*/)
	{
		jsonObjSon["CommPort"] = nullptr;
	}
	else
	{
		jsonObjSon["CommPort"] = info.m_CommPort;
	}
	jsonObjSon["ScreenX"] = info.m_ScreenGeometry.x;
	jsonObjSon["ScreenY"] = info.m_ScreenGeometry.y;
	jsonObjSon["ScreenWidth"] = info.m_ScreenGeometry.width;
	jsonObjSon["ScreenHeight"] = info.m_ScreenGeometry.height;
	jsonObjSon["ScanWidth"] = info.m_ScanSize.width;
	jsonObjSon["ScanHeigth"] = info.m_ScanSize.height;
	jsonObjSon["MoudelWidth"] = info.m_MoudelSize.width;
	jsonObjSon["MoudelHeight"] = info.m_MoudelSize.height;
	jsonObjSon["ScreenType"] = info.m_ScreenType;
	if (info.m_MrvInfoList.empty())
	{
		jsonObjSon["MrvInfoList"] = nullptr;
	}
	else
	{
		std::vector<ST_MrvInfo>::iterator iter = info.m_MrvInfoList.begin();

		json jsonA = json::array();
		for (; iter != info.m_MrvInfoList.end(); iter++)
		{
			json jObjSon;
			if (iter->m_MrvID.length() == 0)
			{
				jObjSon["MrvID"] = nullptr;
			}
			else
			{
				jObjSon["MrvID"] = iter->m_MrvID;
			}
			jObjSon["IsValid"] = iter->m_IsValid;

			if (iter->m_Guid == "")
			{
				jObjSon["Guid"] = nullptr;
			}
			else
			{
				jObjSon["Guid"] = iter->m_Guid;
			}
			jObjSon["XInPort"] = iter->m_XInPort;
			jObjSon["YInPort"] = iter->m_YInPort;
			jObjSon["ConnectIndex"] = iter->m_ConnectIndex;
			jObjSon["Height"] = iter->m_Height;
			jObjSon["PortIndex"] = iter->m_PortIndex;
			jObjSon["SenderIndex"] = iter->m_SenderIndex;
			jObjSon["Width"] = iter->m_Width;
			jObjSon["X"] = iter->m_X;
			jObjSon["Y"] = iter->m_Y;
			jsonA.push_back(jObjSon);
		}
		jsonObjSon["MrvInfoList"] = jsonA;
	}

	std::string str = jsonObjSon.dump();
	return str;
}



//序列化回形屏信息
std::string LCTDataCodec::SerializeDisplayKB(ST_DisplayKBImage info)
{

	std::string str =
		std::to_string(info.m_point.x) + m_SplitTag
		+ std::to_string(info.m_point.y) + m_SplitTag
		+ std::to_string(info.m_size.width) + m_SplitTag
		+ std::to_string(info.m_size.height) + m_SplitTag
		+ std::to_string(info.m_color.r) + m_SplitTag
		+ std::to_string(info.m_color.g) + m_SplitTag
		+ std::to_string(info.m_color.b) + m_SplitTag
		+ std::to_string(info.m_innerpoint.x) + m_SplitTag
		+ std::to_string(info.m_innerpoint.y) + m_SplitTag
		+ std::to_string(info.m_innersize.width) + m_SplitTag
		+ std::to_string(info.m_innersize.height) + m_SplitTag
		+ std::to_string(info.m_innercolor.r) + m_SplitTag
		+ std::to_string(info.m_innercolor.g) + m_SplitTag
		+ std::to_string(info.m_innercolor.b);
	return str;
}


}

}
