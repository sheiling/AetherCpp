#ifndef _SOFTWARE_SCREEN_BASE_HH
#define _SOFTWARE_SCREEN_BASE_HH

#include <stdint.h>
#include <string>
#include <common/utility/json_tool/reflector.h>

namespace common
{
namespace device
{

enum SoftwareScreenDevType {
	SWSDN_WIN32 = 0,                  // windows窗口打屏
	SWSDN_QT = 1,                     // qt窗口打屏
	SWSDN_UNKNOWN                     // 未知型号
};
NLOHMANN_JSON_SERIALIZE_ENUM(SoftwareScreenDevType, {
{ SoftwareScreenDevType::SWSDN_WIN32,         "SWSDN_WIN32" },
{ SoftwareScreenDevType::SWSDN_QT,            "SWSDN_QT" },
{ SoftwareScreenDevType::SWSDN_UNKNOWN,       "SWSDN_UNKNOWN" }
	});


// 软件打屏设备信息
struct SoftwareScreenDevInfo {
	uint16_t devNO = 0;										// 设备编号
	void reflectFromJson(const json& j)
	{
		getFromJson(j, "devNO", devNO);
	}
	void reflectToJson(json& j)const
	{
		j["devNO"] = devNO;
	}
};


enum SoftScreenColor
{
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_BLACK,
	COLOR_WHITE
};

enum SoftScreenMode
{
	FULL_MODE,				// 纯色打屏
	ODD_MODE,				// 奇数棋盘格
	EVEN_MODE				// 偶数棋盘格
};

struct SoftScreenParam
{
    int brightness = 255;						// 打屏灰阶
	int width = 10;								// 打屏宽度
	int height = 10;							// 打屏高度
	int interval = 1;							// 隔点数
	SoftScreenColor color = COLOR_WHITE;		// 打屏颜色
	SoftScreenMode mode = FULL_MODE;		    // 打屏模式
};

class SoftwareScreenBase {
public:
    SoftwareScreenBase() = default;
    virtual ~SoftwareScreenBase() = default;
	void reflectFromJson(const json& j)
	{
		fromJson(j, "connOpts", m_softwareScreenDevInfo_);
		getFromJson(j, "type", m_typeName);
	}
	void reflectToJson(json& j)const
	{
		toJson(j["connOpts"], m_softwareScreenDevInfo_);
		j["type"] = m_typeName;
	}
public:
    virtual bool Init() = 0;
    virtual void Release() = 0;
    virtual void Display(const SoftScreenParam &screenParams) = 0;
protected:
	SoftwareScreenDevInfo m_softwareScreenDevInfo_;		// 设备信息  
	SoftwareScreenDevType m_typeName;					// 设备型号
};

}

}

#endif
