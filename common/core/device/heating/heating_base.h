#pragma once
#include <common/3rdparty/nlohmann/json.hpp>

namespace common
{
namespace device
{

enum class HeatToolType {
	Lazer,
	HalogenLamp,
};

class IHeatingBase {
public:
	IHeatingBase() {}
	virtual ~IHeatingBase() = default;
	virtual bool init() = 0;
	virtual bool work() = 0;
	virtual bool stop() = 0;
	virtual void reflectFromJson(const nlohmann::json& j) = 0;
	virtual void reflectToJson(nlohmann::json& j) const = 0;

private:

};

}

}