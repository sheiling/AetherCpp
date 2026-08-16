#include "heating_laser.h"
#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/container/application.h>

namespace common
{
namespace device

HeatingLaser::HeatingLaser() {}

HeatingLaser::~HeatingLaser() {}

void HeatingLaser::reflectFromJson(const nlohmann::json& j) {
	m_deviceData.reflectFromJson(j);
}

void HeatingLaser::reflectToJson(nlohmann::json& j) const {
	m_deviceData.reflectToJson(j);
}

bool HeatingLaser::init() {
	return true;
}

bool HeatingLaser::work() {
	//ѡ�񼤹⹤��
	using namespace std;
	
	shared_ptr<common::device::DeviceManager> manager = CAPP->make<common::device::DeviceManager>();
	shared_ptr<common::device::MotionController> motion = manager->MotionHandler();
	bool bValue{ false };
	deviceIO::DeviceIO::DeviceIoData ioData;
	ioData.m_logic = 0;
	ioData.m_extend = 0;
	ioData.m_port = 40;
	motion->setDeviceIoData(m_deviceData.changeLaserIO0, ioData);
	motion->WriteIO(m_deviceData.changeLaserIO0, bValue); // (1,8,0) port = 1 * 32 + 8

	ioData.m_port = 41;
	bValue = true;
	motion->setDeviceIoData(m_deviceData.changeLaserIO1, ioData);
	motion->WriteIO(m_deviceData.changeLaserIO1, bValue); // (1,9,1) port = 1 * 32 + 9

	this_thread::sleep_for(chrono::milliseconds(200));

	//����ʹ��
	ioData.m_port = 44;
	bValue = true;
	motion->setDeviceIoData(m_deviceData.enableLaserIO, ioData);
	motion->WriteIO(m_deviceData.enableLaserIO, bValue); //(1,12,1) port = 1 * 32 + 12

	this_thread::sleep_for(chrono::milliseconds(500));

	//�򿪼���
	ioData.m_port = 38;
	bValue = true;
	motion->setDeviceIoData(m_deviceData.openLaserIO, ioData);
	motion->WriteIO(m_deviceData.openLaserIO, bValue); // (1,6,1) port = 1 * 32 + 6

	//�жϼ����Ƿ��,3000ms��
	auto readOpenIOFunc = [&motion, this]()->bool {
		bool isOpen{ false };
		motion->ReadIO(m_deviceData.openLaserIO, isOpen);
		if (!isOpen) {//��ʧ��
			return false;
		}
		return true;
	};
	int timeout = 1000 * 3;
	if (!common::utility::waitForCondition(readOpenIOFunc, chrono::milliseconds(timeout))) {
		return false;
	}

	//�ж��Ƿ�ִ�����
	auto readOverIOFunc = [&motion, this]() {
		bool isRunning{ true };
		motion->ReadIO(m_deviceData.isLaserOverIO, isRunning);
		if (!isRunning) {//�ѽ���
			return true;
		}
		return false;
	};
	if (!common::utility::waitForCondition(readOverIOFunc, chrono::milliseconds(timeout))) {
		return false;
	}

	//�رռ���
	ioData.m_port = 38;
	bValue = false;
	motion->setDeviceIoData(m_deviceData.openLaserIO, ioData);
	motion->WriteIO(m_deviceData.openLaserIO, bValue); // (1,6,0) port = 1 * 32 + 6

	//����ʹ�ܹر�
	ioData.m_port = 44;
	bValue = false;
	motion->setDeviceIoData(m_deviceData.enableLaserIO, ioData);
	motion->WriteIO(m_deviceData.enableLaserIO, bValue); //(1,12,0) port = 1 * 32 + 12

	return true;
}

bool HeatingLaser::stop() {
	return true;
}

}

}