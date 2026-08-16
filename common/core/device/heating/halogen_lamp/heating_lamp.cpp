#include "heating_lamp.h"
namespace common
{
namespace device

HeatingLamp::HeatingLamp() {}

HeatingLamp::~HeatingLamp() {}

bool HeatingLamp::init() {
	return _lamp.initSerialPort({});
}

bool HeatingLamp::work() {
	//����ʵ��������п���
	return true;
}

bool HeatingLamp::stop() {
	return true;
}

}
}