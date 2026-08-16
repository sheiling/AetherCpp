#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CARD_REGISTRY_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_CARD_REGISTRY_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "motion_card_base.h"

namespace common::device::motion {

using MotionCardFactoryFn = std::function<std::unique_ptr<MotionCardBase>()>;

class MotionCardRegistry {
public:
	static MotionCardRegistry& instance();

	void registerFactory(MotionCardDevType type, MotionCardFactoryFn factory);
	std::unique_ptr<MotionCardBase> create(MotionCardDevType type) const;

private:
	MotionCardRegistry();
	void registerBuiltins();

	std::unordered_map<MotionCardDevType, MotionCardFactoryFn> m_factories_;
};

} // namespace common::device::motion

#endif
