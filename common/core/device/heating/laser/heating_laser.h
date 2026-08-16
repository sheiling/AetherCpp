#pragma once

#include "../heating_base.h"
#include <string>
#include <common/utility/json_tool/reflector.h>

namespace common
{
namespace device

struct LaserDeviceData {
	std::string changeLaserIO0 = "ѡ�񼤹⹤��IO0";
	std::string changeLaserIO1 = "ѡ�񼤹⹤��IO1";
	std::string enableLaserIO = "����ʹ��IO";
	std::string openLaserIO = "�򿪼���IO";
	std::string isOpenLaserIO = "�ж��Ƿ�򿪼���IO";
	std::string isLaserOverIO = "�ж��Ƿ�ִ�����IO";
	std::string closeLaserIO0 = "�رռ���IO0";
	std::string closeLaserIO1 = "�رռ���IO1";
	void reflectFromJson(const nlohmann::json& j) {
		FROM_JSON(j, changeLaserIO0);
		FROM_JSON(j, changeLaserIO1);
		FROM_JSON(j, enableLaserIO);
		FROM_JSON(j, openLaserIO);
		FROM_JSON(j, isOpenLaserIO);
		FROM_JSON(j, isLaserOverIO);
		FROM_JSON(j, closeLaserIO0);
		FROM_JSON(j, closeLaserIO1);
	}
	void reflectToJson(nlohmann::json& j) const {
		TO_JSON(j, changeLaserIO0);
		TO_JSON(j, changeLaserIO1);
		TO_JSON(j, enableLaserIO);
		TO_JSON(j, openLaserIO);
		TO_JSON(j, isOpenLaserIO);
		TO_JSON(j, isLaserOverIO);
		TO_JSON(j, closeLaserIO0);
		TO_JSON(j, closeLaserIO1);
	}
};

class HeatingLaser : public IHeatingBase{
public:
	HeatingLaser();
	~HeatingLaser();
	virtual void reflectFromJson(const nlohmann::json& j);
	virtual void reflectToJson(nlohmann::json& j) const;

	virtual bool init();
	virtual bool work();
	virtual bool stop();
private:
	LaserDeviceData m_deviceData;
};

}

}