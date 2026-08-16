#include "motion_controller.h"
#include "motion_error_codes.h"
#include "motion_control_impl/motion_card_registry.h"
#include <Config.h>
#include <common/include/macros.h>
#include <Logger.h>
#include <messagecenter/message_center.h>

namespace common
{
namespace device
{
namespace {
const bool kMotionCodersRegistered = ::common::errors::motion_module::RegisterMotionCoders();
} // namespace

ErrorPtr MotionController::InitDevice()
{
	for (auto& card : m_motionCardPool_)
	{
		const ErrorPtr err = card.second->InitDevice();
		SetCardConnectedState(card.first, err == nullptr);
		if (err)
			return err;
	}
	return nullptr;
}
void MotionController::Release()
{
	for (auto& card : m_motionCardPool_)
	{
		card.second->Release();
		SetCardConnectedState(card.first, false);
	}
}

void MotionController::SetCardConnectedState(const std::string& cardName, bool connected)
{
	const auto it = cardConnectedState_.find(cardName);
	if (it != cardConnectedState_.end() && it->second == connected) {
		return;
	}
	cardConnectedState_[cardName] = connected;
	MessageCenter::Center::Notify(
		EVENT_MOTION_CONN_STATE,
		QString::fromStdString(cardName),
		connected);
}

bool MotionController::GetCardConnectedState(const std::string& cardName)
{
	if (getMotionCardHandler(cardName) == nullptr)
	{
		return false;
	}
	const auto it = cardConnectedState_.find(cardName);
	return (it != cardConnectedState_.end()) ? it->second : false;
}

ErrorPtr MotionController::TotalAxis(uint32_t& axisNum)
{
	axisNum = axisToCardMap_.size();
	return nullptr;
}
ErrorPtr MotionController::AxisEnable(const std::string& axisName, bool enable)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisEnable(axisName, enable);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisIsEnable(const std::string& axisName, bool& enable)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisIsEnable(axisName, enable);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisMove(const std::string& axisName, const common::device::motion::AxisMoveParams& params)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisMove(axisName, params);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisLinear(const std::vector<std::string>& axisNames, const common::device::motion::AxisLinearParams& params)
{
	if (axisNames.size() != params.dists.size() || axisNames.size() < 2)
	{
		return common::errors::New(common::errors::motion_module::kInvalidAxisCount, "Wrong number of axis.");
	}

	std::vector<std::shared_ptr<common::device::motion::MotionCardBase>> vecCard;
	bool sameCard = true;
	std::string firstCardName = "";
	for (const auto& axisName : axisNames)
	{
		auto card = getMotionCardHandlerByAxis(axisName);
		if (card == nullptr)
			return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());

		//判断所有轴是否为同一张卡上面的轴
		if (sameCard)
		{
			auto axisNameIter = axisToCardMap_.find(axisName);
			if (axisNameIter == axisToCardMap_.end())
				return common::errors::Newf(common::errors::motion_module::kAxisNameEmpty, "The axis[%s] name is empty.", axisName.c_str());

			//暂存第一个轴的卡名称 判断后续轴对应的卡名称是否与第一个轴一样
			if (firstCardName.empty())
				firstCardName = axisNameIter->second;
			else if (firstCardName != axisNameIter->second)
				sameCard = false;
		}
		//保存所有的卡实例
		vecCard.push_back(card);
	}
	if (vecCard.empty() || vecCard.size() != axisNames.size())
		return common::errors::New(common::errors::motion_module::kInvalidCardCount, "Wrong number of card.");

	//同一张卡上面的轴 直接调用直线插补接口
	if (sameCard)
	{
		return vecCard[0]->AxisLinear(axisNames, params);
	}

	for (size_t i = 0; i < vecCard.size(); i++)
	{
		common::device::motion::AxisMoveParams moveParams;
		moveParams.dist = params.dists[i];
		moveParams.posiMode = params.posiMode;
		moveParams.wait = false;
		vecCard[i]->AxisMove(axisNames[i], moveParams);
	}
	std::vector<ErrorPtr> vecResultInfo;
	for (size_t i = 0; i < vecCard.size(); i++)
	{
		vecResultInfo.push_back(vecCard[i]->AxisWaitMoveDone(axisNames[i], params.timeout));
	}
	for (const auto& result : vecResultInfo)
	{
		if (result)
			return result;
	}
	return nullptr;
}
ErrorPtr MotionController::AxisVmove(const std::string& axisName, uint16_t dir, double velRatio)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisVmove(axisName, dir, velRatio);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisWaitMoveDone(const std::string& axisName, int timeout)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisWaitMoveDone(axisName, timeout);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisStop(const std::string& axisName, uint8_t mode)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisStop(axisName, mode);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisEmgStop()
{
	for (auto& card : m_motionCardPool_)
	{
		CHECK_RTN_INFO(card.second->AxisEmgStop());
	}
	return nullptr;
}
ErrorPtr MotionController::AxisReset(const std::string& axisName, int timeout)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisReset(axisName, timeout);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisAlarmClear(const std::string& axisName)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisAlarmClear(axisName);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisAlarmClear()
{
	for (auto& axis : axisToCardMap_)
	{
		CHECK_RTN_INFO(AxisAlarmClear(axis.first));
	}
	return nullptr;
}
ErrorPtr MotionController::AxisReadStatus(const std::string& axisName, common::device::axis::Axis::AxisSta& axisStatus)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisReadStatus(axisName, axisStatus);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisGetEncoder(const std::string& axisName, double& value)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisGetEncoder(axisName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisGetPosition(const std::string& axisName, double& value)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisGetPosition(axisName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisSetEncoderAndPosition(const std::string& axisName, double value)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisSetEncoderAndPosition(axisName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisSetSoftlimit(const std::string& axisName, const common::device::axis::Axis::AxisSoftlimit& axisSoftlimit)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisSetSoftlimit(axisName, axisSoftlimit);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisEnableLtc(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisEnableLtc(axisName, axisLtc);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisGetLtcPos(const std::string& axisName, const common::device::axis::Axis::AxisLtc& axisLtc, std::vector<double>& vecPos)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr)
	{
		return card->AxisGetLtcPos(axisName, axisLtc, vecPos);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisMoveWithPositionCompare(
	const std::string& axisName, const common::device::motion::AxisMoveWithPositionCompareParams& params)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card != nullptr) {
		return card->AxisMoveWithPositionCompare(axisName, params);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", axisName.c_str());
}
ErrorPtr MotionController::AxisLinearWithPositionCompare(const common::device::motion::LinearPositionCompareParams& params)
{
	auto xCard = getMotionCardHandlerByAxis(params.axisXName);
	auto yCard = getMotionCardHandlerByAxis(params.axisYName);
	if (xCard == nullptr || yCard == nullptr) {
		return common::errors::New(common::errors::motion_module::kAxisCardNotMapped, "linear axis not mapped to card");
	}
	if (xCard != yCard) {
		return common::errors::New(common::errors::motion_module::kInvalidCardCount,
			"linear position compare requires X/Y on same motion card");
	}
	return xCard->AxisLinearWithPositionCompare(params);
}
ErrorPtr MotionController::DisablePositionCompare(const common::device::motion::PositionCompareDisableParams& params)
{
	auto card = getMotionCardHandlerByAxis(params.compareAxisName);
	if (card != nullptr) {
		return card->DisablePositionCompare(params);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Axis[%s] to card is null.", params.compareAxisName.c_str());
}
ErrorPtr MotionController::WriteIO(const std::string& ioName, bool value)
{
	auto card = getMotionCardHandlerByIO(ioName);
	if (card != nullptr)
	{
		return card->WriteIO(ioName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kIoCardNotMapped, "IO[%s] to card is null.", ioName.c_str());
}
ErrorPtr MotionController::ReadIO(const std::string& ioName, bool& value)
{
	auto card = getMotionCardHandlerByIO(ioName);
	if (card != nullptr)
	{
		return card->ReadIO(ioName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kIoCardNotMapped, "IO[%s] to card is null.", ioName.c_str());
}
ErrorPtr MotionController::ReadDoPorts(const std::string& cardName, std::vector<uint32_t>& vecValue)
{
	auto card = getMotionCardHandler(cardName);
	if (card != nullptr)
	{
		return card->ReadDoPorts(vecValue);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Card[%s] not found.", cardName.c_str());
}
ErrorPtr MotionController::ReadDiPorts(const std::string& cardName, std::vector<uint32_t>& vecValue)
{
	auto card = getMotionCardHandler(cardName);
	if (card != nullptr)
	{
		return card->ReadDiPorts(vecValue);
	}
	return common::errors::Newf(common::errors::motion_module::kAxisCardNotMapped, "Card[%s] not found.", cardName.c_str());
}
ErrorPtr MotionController::WriteAD(const std::string& adName, double value)
{
	auto card = getMotionCardHandlerByAD(adName);
	if (card != nullptr)
	{
		return card->WriteAD(adName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kAdCardNotMapped, "AD[%s] to card is null.", adName.c_str());
}
ErrorPtr MotionController::ReadAD(const std::string& adName, double& value)
{
	auto card = getMotionCardHandlerByAD(adName);
	if (card != nullptr)
	{
		return card->ReadAD(adName, value);
	}
	return common::errors::Newf(common::errors::motion_module::kAdCardNotMapped, "AD[%s] to card is null.", adName.c_str());
}

std::vector<std::string> MotionController::getAllCardName()
{
	std::vector<std::string> vec;
	for (auto card : m_motionCardPool_)
		vec.push_back(card.first);
	return vec;
}
std::vector<std::string> MotionController::getAllAxisName()
{
	std::vector<std::string> vec;
	for (auto axis : axisToCardMap_)
		vec.push_back(axis.first);

	return vec;
}
std::vector<std::string> MotionController::getAllIoName()
{
	std::vector<std::string> vec;
	for (auto io : ioToCardMap_)
		vec.push_back(io.first);

	return vec;
}
std::vector<std::string> MotionController::getAllAdName()
{
	std::vector<std::string> vec;
	for (auto ad : adToCardMap_)
		vec.push_back(ad.first);

	return vec;
}
common::device::axis::Axis::AxisData MotionController::getAxisParams(const std::string& axisName)
{
	common::device::axis::Axis::AxisData axisData;
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card)
	{
		card->getAxisParams(axisName, axisData);
	}
	return axisData;
}
common::device::axis::Axis::MoveParameters MotionController::getAxisMoveParams(const std::string& axisName)
{
	common::device::axis::Axis::MoveParameters moveParameters;
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card)
	{
		card->getMoveParameters(axisName, moveParameters);
	}
	return moveParameters;
}
void MotionController::setAxisParams(const std::string& axisName, common::device::axis::Axis::AxisData& axisData)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card)
	{
		card->setAxisParams(axisName, axisData);
	}
}
void MotionController::setAxisMoveParams(const std::string& axisName, common::device::axis::Axis::MoveParameters& moveParameters)
{
	auto card = getMotionCardHandlerByAxis(axisName);
	if (card)
	{
		card->setMoveParameters(axisName, moveParameters);
	}
}
common::device::deviceIO::DeviceIO::DeviceIoData MotionController::getDeviceIoData(const std::string& ioName)
{
	common::device::deviceIO::DeviceIO::DeviceIoData deviceIoData;
	auto card = getMotionCardHandlerByIO(ioName);
	if (card)
	{
		card->getIoParams(ioName, deviceIoData);
	}
	return deviceIoData;

}
void MotionController::setDeviceIoData(const std::string& ioName, common::device::deviceIO::DeviceIO::DeviceIoData& deviceIoData)
{
	auto card = getMotionCardHandlerByIO(ioName);
	if (card)
	{
		card->setIoParams(ioName, deviceIoData);
	}
}
common::device::deviceAD::DeviceAD::DeviceAdData MotionController::getDeviceAdData(const std::string& adName)
{
	common::device::deviceAD::DeviceAD::DeviceAdData deviceAdData;
	auto card = getMotionCardHandlerByAD(adName);
	if (card)
	{
		card->getAdParams(adName, deviceAdData);
	}
	return deviceAdData;
}
void MotionController::setDeviceAdData(const std::string& adName, common::device::deviceAD::DeviceAD::DeviceAdData& deviceAdData)
{
	auto card = getMotionCardHandlerByAD(adName);
	if (card)
	{
		card->setAdParams(adName, deviceAdData);
	}
}


void MotionController::reflectFromJson(const json& j)
{
	//初始化卡参数
	if (!j.contains("Card") || j["Card"].is_null())
		return;
	const json& cardObj = j["Card"];
	for (auto it = cardObj.begin(); it != cardObj.end(); ++it)
	{
		std::string key = it.key();
		const json& obj = it.value();
		if (obj.contains("type") && !obj["type"].is_null())
		{
			auto type_name = obj.at("type").get<common::device::motion::MotionCardDevType>();
			std::unique_lock<std::shared_mutex> lock(m_motionCardMutex_);
			std::unique_ptr<common::device::motion::MotionCardBase> motionCard = createInstance(key, type_name);
			if (motionCard)
			{
				motionCard->reflectFromJson(obj);
				m_motionCardPool_[key] = std::move(motionCard);
			}
		}
	}

	//初始化轴 IO AD模块参数
	std::unordered_map<std::string, std::shared_ptr<common::device::axis::Axis>> axisDataMap;
	std::unordered_map<std::string, std::shared_ptr<deviceIO::DeviceIO>> ioDataMap;
	std::unordered_map<std::string, std::shared_ptr<deviceAD::DeviceAD>> adDataMap;
	fromJson(j, "Axis", axisDataMap);
	fromJson(j, "DeviceIo", ioDataMap);
	fromJson(j, "DeviceAd", adDataMap);

	//设置数据到对应卡实例
	setAxisToCard(axisDataMap);
	setIoToCard(ioDataMap);
	setAdToCard(adDataMap);

}
void MotionController::reflectToJson(json& j)const
{
	// 多卡共用 j["Axis"]/IO/AD：须先清空再逐卡 saveDataToJson；单段 toJson(map) 不会 erase 旧键。
	j["Card"]     = json::object();
	j["Axis"]     = json::object();
	j["DeviceIo"] = json::object();
	j["DeviceAd"] = json::object();

	toJson(j["Card"], m_motionCardPool_);
	for (const auto &card : m_motionCardPool_)
	{
		if (card.second)
			card.second->saveDataToJson(j);
	}
}
bool MotionController::addDevice(const std::string& moduleName, const std::string& key, common::device::motion::MotionCardDevType type, const std::string& cardName)
{
	bool rtn = false;
	if (moduleName == "Card")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionCardMutex_);
		std::unique_ptr<common::device::motion::MotionCardBase> motionCard = createInstance(key, type);
		if (motionCard)
			m_motionCardPool_[key] = std::move(motionCard);
	}
	else
	{
		if (moduleName == "Axis")
		{
			std::unordered_map<std::string, std::shared_ptr<common::device::axis::Axis>> axisDataMap;
			std::shared_ptr<common::device::axis::Axis> axis = std::make_shared<common::device::axis::Axis>();
			common::device::axis::Axis::AxisData axisData = axis->getAxisParams();
			axisData.m_cardName = cardName;
			axis->setAxisParams(axisData);
			axisDataMap[key] = std::move(axis);

			rtn = setAxisToCard(axisDataMap);
		}
		else if (moduleName == "DeviceIO")
		{
			std::unordered_map<std::string, std::shared_ptr<deviceIO::DeviceIO>> ioDataMap;
			std::shared_ptr<common::device::deviceIO::DeviceIO> io = std::make_shared<common::device::deviceIO::DeviceIO>();
			common::device::deviceIO::DeviceIO::DeviceIoData ioData = io->getDeviceIoData();
			ioData.m_cardName = cardName;
			io->setDeviceIoData(ioData);
			ioDataMap[key] = std::move(io);

			rtn = setIoToCard(ioDataMap);
		}
		else if (moduleName == "DeviceAD")
		{
			std::unordered_map<std::string, std::shared_ptr<deviceAD::DeviceAD>> adDataMap;
			std::shared_ptr<common::device::deviceAD::DeviceAD> ad = std::make_shared<common::device::deviceAD::DeviceAD>();
			common::device::deviceAD::DeviceAD::DeviceAdData adData = ad->getDeviceAdData();
			adData.m_cardName = cardName;
			ad->setDeviceAdData(adData);
			adDataMap[key] = std::move(ad);

			rtn = setAdToCard(adDataMap);

		}
	}
	return rtn;
}
bool MotionController::removeDevice(const std::string& moduleName, const std::string& key)
{
	bool rtn = false;
	if (moduleName == "Card")
	{
		std::unique_lock<std::shared_mutex> lock(m_motionCardMutex_);
		auto it = m_motionCardPool_.find(key);
		if (it != m_motionCardPool_.end())
		{
			m_motionCardPool_.erase(it);
			rtn = true;
		}
		else {
			LOG_ERROR("[common][device][motion]--{} failed, name={} is inexistence", moduleName, key);
		}
	}
	else
	{
		std::shared_ptr<common::device::motion::MotionCardBase> card = nullptr;
		if (moduleName == "Axis")
			card = getMotionCardHandlerByAxis(key);
		else if (moduleName == "DeviceIO")
			card = getMotionCardHandlerByIO(key);
		else if (moduleName == "DeviceAD")
			card = getMotionCardHandlerByAD(key);

		if (card)
		{
			rtn = card->remove(moduleName, key);
			if (moduleName == "Axis")
			{
				std::unique_lock<std::shared_mutex> lock(m_axisToCardMutex_);
				auto iter = axisToCardMap_.find(key);
				if (iter != axisToCardMap_.end())
					axisToCardMap_.erase(iter);
			}
			else if (moduleName == "DeviceIO")
			{
				std::unique_lock<std::shared_mutex> lock(m_ioToCardMutex_);
				auto iter = ioToCardMap_.find(key);
				if (iter != ioToCardMap_.end())
					ioToCardMap_.erase(iter);
			}
			else if (moduleName == "DeviceAD")
			{
				std::unique_lock<std::shared_mutex> lock(m_adToCardMutex_);
				auto iter = adToCardMap_.find(key);
				if (iter != adToCardMap_.end())
					adToCardMap_.erase(iter);
			}
		}
	}
	return rtn;
}


std::unique_ptr<common::device::motion::MotionCardBase> MotionController::createInstance(const std::string& key, common::device::motion::MotionCardDevType type)
{
	std::unique_ptr<common::device::motion::MotionCardBase> motionCard = nullptr;

	if (m_motionCardPool_.find(key) == m_motionCardPool_.end())
	{
		if (CFG_GET("virtual_debug.virtual_motion_enable", bool, false)) {
			type = common::device::motion::MotionCardDevType::VMC;
		}
		motionCard = common::device::motion::MotionCardRegistry::instance().create(type);
		if (!motionCard)
		{
			LOG_ERROR("[common][device][motion]--不支持的控制卡类型={}", static_cast<int>(type));
		}
	}
	else
	{
		LOG_ERROR("[common][device][motion]--failed, name={} is already existed", key);
	}
	return motionCard;
}
const std::shared_ptr<common::device::motion::MotionCardBase> MotionController::getMotionCardHandler(const std::string& key) const
{
	std::shared_lock<std::shared_mutex> lock(m_motionCardMutex_);
	std::shared_ptr<common::device::motion::MotionCardBase> motionCard = nullptr;
	auto iter = m_motionCardPool_.find(key);
	if (iter != m_motionCardPool_.end()) {
		motionCard = iter->second;
	}
	return motionCard;
}
const std::shared_ptr<common::device::motion::MotionCardBase> MotionController::getMotionCardHandlerByAxis(const std::string& key)const
{
	std::shared_lock<std::shared_mutex> lock(m_axisToCardMutex_);
	std::shared_ptr<common::device::motion::MotionCardBase> motionCard = nullptr;
	auto iter = axisToCardMap_.find(key);
	if (iter != axisToCardMap_.end())
		motionCard = getMotionCardHandler(iter->second);

	return motionCard;
}
const std::shared_ptr<common::device::motion::MotionCardBase> MotionController::getMotionCardHandlerByIO(const std::string& key)const
{
	std::shared_lock<std::shared_mutex> lock(m_ioToCardMutex_);
	std::shared_ptr<common::device::motion::MotionCardBase> motionCard = nullptr;
	auto iter = ioToCardMap_.find(key);
	if (iter != ioToCardMap_.end())
		motionCard = getMotionCardHandler(iter->second);

	return motionCard;
}
const std::shared_ptr<common::device::motion::MotionCardBase> MotionController::getMotionCardHandlerByAD(const std::string& key)const
{
	std::shared_lock<std::shared_mutex> lock(m_adToCardMutex_);
	std::shared_ptr<common::device::motion::MotionCardBase> motionCard = nullptr;
	auto iter = adToCardMap_.find(key);
	if (iter != adToCardMap_.end())
		motionCard = getMotionCardHandler(iter->second);

	return motionCard;
}
bool MotionController::setAxisToCard(std::unordered_map<std::string, std::shared_ptr<common::device::axis::Axis>>& axisDataMap)
{
	bool rtn = true;
	//添加卡到轴名称的映射 并且将轴数据设置到对应的卡实例里面
	for (auto& axisData : axisDataMap)
	{
		//查找卡名称对应的指针
		std::string cardName = axisData.second->getAxisParams().m_cardName;
		std::shared_ptr<common::device::motion::MotionCardBase> motionCard = getMotionCardHandler(cardName);
		if (motionCard == nullptr)
		{
			rtn = false;
			LOG_ERROR("[common][device][motion]--get motion card:{} failed.", cardName);
			continue;
		}

		{
			std::unique_lock<std::shared_mutex> lock(m_axisToCardMutex_);
			//判断是否重名
			if (axisToCardMap_.end() != axisToCardMap_.find(axisData.first))
			{
				rtn = false;
				LOG_ERROR("[common][device][motion]--axis name[{}] is already existed", axisData.first);
				continue;
			}
			axisToCardMap_[axisData.first] = cardName;
		}

		if (!motionCard->addAxis("Axis", axisData.first, std::move(axisData.second)))
			rtn = false;
	}
	return rtn;
}
bool MotionController::setIoToCard(std::unordered_map<std::string, std::shared_ptr<deviceIO::DeviceIO>>& ioDataMap)
{
	bool rtn = true;
	//添加卡到IO名称的映射 并且将IO数据设置到对应的卡实例里面
	for (auto& ioData : ioDataMap)
	{
		//查找卡名称对应的指针
		std::string cardName = ioData.second->getDeviceIoData().m_cardName;
		std::shared_ptr<common::device::motion::MotionCardBase> motionCard = getMotionCardHandler(cardName);
		if (motionCard == nullptr)
		{
			rtn = false;
			LOG_ERROR("[common][device][motion]--get motion card:{} failed.", cardName);
			continue;
		}

		{
			std::unique_lock<std::shared_mutex> lock(m_ioToCardMutex_);
			//判断是否重名
			if (ioToCardMap_.end() != ioToCardMap_.find(ioData.first))
			{
				rtn = false;
				LOG_ERROR("[common][device][motion]--io name[{}] is already existed", ioData.first);
				continue;
			}
			ioToCardMap_[ioData.first] = cardName;
		}

		if (!motionCard->addDeviceIO("DeviceIO", ioData.first, std::move(ioData.second)))
			rtn = false;
	}
	return true;
}
bool MotionController::setAdToCard(std::unordered_map<std::string, std::shared_ptr<deviceAD::DeviceAD>>& adDataMap)
{
	bool rtn = true;
	//添加卡到AD模块名称的映射 并且将AD模块数据设置到对应的卡实例里面
	for (auto& adData : adDataMap)
	{
		//查找卡名称对应的指针
		std::string cardName = adData.second->getDeviceAdData().m_cardName;
		std::shared_ptr<common::device::motion::MotionCardBase> motionCard = getMotionCardHandler(cardName);
		if (motionCard == nullptr)
		{
			rtn = false;
			LOG_ERROR("[common][device][motion]--get motion card:{} failed.", cardName);
			continue;
		}

		{
			std::unique_lock<std::shared_mutex> lock(m_adToCardMutex_);
			//判断是否重名
			if (adToCardMap_.end() != adToCardMap_.find(adData.first))
			{
				rtn = false;
				LOG_ERROR("[common][device][motion]--ad name[{}] is already existed", adData.first);
				continue;
			}
			adToCardMap_[adData.first] = cardName;
		}

		if (!motionCard->addDeviceAD("DeviceAD", adData.first, std::move(adData.second)))
			return false;
	}
	return true;
}

} // end of namespace device

}
