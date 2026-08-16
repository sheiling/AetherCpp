#include "motion_card_registry.h"

#include <Logger.h>

#ifdef BUILD_MOTION_LEISHINE_DMC
#include "drivers/leadshine_dmc/leadshine_dmc_motion_card.h"
#endif
#ifdef BUILD_MOTION_LEISHINE_NMC
#include "drivers/leadshine_nmc/leadshine_nmc_motion_card.h"
#endif
#ifdef BUILD_MOTION_SOLID
#include "drivers/solid/solid_motion_card.h"
#endif
#ifdef BUILD_MOTION_GAOCHUAN_IO
#include "drivers/gaochuan_io/gaochuan_io_motion_card.h"
#endif
#include "drivers/virtual_motion/virtual_motion_card.h"

namespace common::device::motion {

MotionCardRegistry& MotionCardRegistry::instance()
{
	static MotionCardRegistry registry;
	return registry;
}

MotionCardRegistry::MotionCardRegistry()
{
	registerBuiltins();
}

void MotionCardRegistry::registerFactory(MotionCardDevType type, MotionCardFactoryFn factory)
{
	m_factories_[type] = std::move(factory);
}

std::unique_ptr<MotionCardBase> MotionCardRegistry::create(MotionCardDevType type) const
{
	const auto it = m_factories_.find(type);
	if (it == m_factories_.end()) {
		LOG_ERROR("[common][device][motion]--控制卡工厂未注册，类型={}", static_cast<int>(type));
		return nullptr;
	}
	auto card = it->second();
	LOG_INFO("[common][device][motion]--控制卡实例创建成功，类型={}", static_cast<int>(type));
	return card;
}

void MotionCardRegistry::registerBuiltins()
{
#ifdef BUILD_MOTION_LEISHINE_DMC
	registerFactory(MotionCardDevType::LeiShineDmc, []() {
		return std::make_unique<LeadshineDmcMotionCard>();
	});
#endif
#ifdef BUILD_MOTION_LEISHINE_NMC
	registerFactory(MotionCardDevType::LeiShineNmc, []() {
		return std::make_unique<LeadshineNmcMotionCard>();
	});
#endif
#ifdef BUILD_MOTION_SOLID
	registerFactory(MotionCardDevType::Solid, []() {
		return std::make_unique<SolidMotionCard>();
	});
#endif
#ifdef BUILD_MOTION_GAOCHUAN_IO
	registerFactory(MotionCardDevType::GaoChuanIO, []() {
		return std::make_unique<GaoChuanIoMotionCard>();
	});
#endif
	registerFactory(MotionCardDevType::VMC, []() {
		return std::make_unique<VirtualMotionCard>();
	});
}

} // namespace common::device::motion
