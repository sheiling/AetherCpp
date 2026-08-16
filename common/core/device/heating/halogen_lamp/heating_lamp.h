#pragma once

#include "../heating_base.h"
#include "HalogenLamp.h"

namespace common
{
namespace device
{

class HeatingLamp : public IHeatingBase {
public:
	HeatingLamp();
	~HeatingLamp();

	void reflectFromJson(const json& j) {
		_lamp.reflectFromJson(j);
	}

	void reflectToJson(json& j)const {
		_lamp.reflectToJson(j);
	}

	virtual bool init();
	virtual bool work();
	virtual bool stop();
private:
	HalogenLamp _lamp;
};

}

}