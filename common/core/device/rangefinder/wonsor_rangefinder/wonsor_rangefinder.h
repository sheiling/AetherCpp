#pragma once
#include <common/3rdparty/cserialport/include/CSerialPort/SerialPort.h>
#include <common/core/comm/serial/serial_master_opts.h>
#include <common/utility/json_tool/reflector.h>
#include "../rangefinder_global.h"

namespace common
{
namespace device
{

struct WonsorRangefinderDevInfo {
	uint16_t devNO = 0;                                 // 设备编号
	common::serial::SerialInfo serialInfo;				// 串口信息
	void reflectFromJson(const json& j) {
		getFromJson(j, "devNO", devNO);
		fromJson(j, "serialInfo", serialInfo);
	}
	void reflectToJson(json& j)const {
		j["devNO"] = devNO;
		toJson(j["serialInfo"], serialInfo);
	}
};

class RANGEFINDER_EXPORT WonsorRangefinder {
public:
	WonsorRangefinder();
	~WonsorRangefinder() = default;

	void reflectFromJson(const json& j) {
		fromJson(j, "devInfo", _devInfo);
	}
	void reflectToJson(json& j)const {
		toJson(j["devInfo"], _devInfo);
	}

	bool openRangeFinder();
	bool measureDistance(float& value);
	//void setSlaveId(const uint16_t& id);

private:
	//uint16_t _slaveID = 1;//代码中没用到，暂时不要，需要时从_devInfo中取devNO
	WonsorRangefinderDevInfo _devInfo;
	itas109::CSerialPort _serial;
};

}
}
