#include "gaochuan_io_motion_card.h"

#include <common/3rdparty/device/motion/GaoChuan_IO/include/sc_io.h>

#include <Logger.h>
#include <mutex>
#include <unordered_map>

namespace common
{
namespace device
{
namespace motion
{

namespace {

std::mutex g_scIoMutex;
std::unordered_map<std::string, HAND> g_openById;

long validateChannel(uint16_t channel)
{
	if (channel > 31)
		return static_cast<long>(RTN_LIB_PARA_ERROR);
	return static_cast<long>(RTN_CMD_SUCCESS);
}

static short readDoBitViaPort(HAND h, U16 channel, U16* outRaw)
{
	const U16 port = static_cast<U16>(channel / 16u);
	const unsigned bit = static_cast<unsigned>(channel % 16u);
	U16 buf[1] = { 0 };
	const short rtn = NIO_GetDO(h, port, 1, buf);
	if (rtn != RTN_CMD_SUCCESS)
		return rtn;
	*outRaw = static_cast<U16>((static_cast<unsigned>(buf[0]) >> bit) & 1u);
	return RTN_CMD_SUCCESS;
}

static short writeDoBitViaPort(HAND h, U16 channel, bool high)
{
	const U16 port = static_cast<U16>(channel / 16u);
	const unsigned bit = static_cast<unsigned>(channel % 16u);
	U16 buf[1] = { 0 };
	short rtn = NIO_GetDO(h, port, 1, buf);
	if (rtn != RTN_CMD_SUCCESS)
		return rtn;
	if (high)
		buf[0] = static_cast<U16>(buf[0] | static_cast<U16>(1u << bit));
	else
		buf[0] = static_cast<U16>(buf[0] & static_cast<U16>(0xFFFFu ^ (1u << bit)));
	return NIO_SetDO(h, port, 1, buf);
}

bool getOrOpenHandleLocked(const std::string& deviceId, HAND& outHandle)
{
	if (deviceId.empty())
		return false;
	auto it = g_openById.find(deviceId);
	if (it != g_openById.end()) {
		outHandle = it->second;
		return true;
	}
	HAND h = 0;
	const short rtn = NIO_OpenByID(reinterpret_cast<const B8*>(deviceId.c_str()), &h);
	if (rtn != RTN_CMD_SUCCESS || h == 0)
		return false;
	g_openById[deviceId] = h;
	outHandle = h;
	return true;
}

constexpr U16 kIoPortStart = 0;
constexpr U16 kIoPortCount = 2;

long readIoBit(const std::string& deviceId, uint16_t channel, bool isInputIo, bool& value)
{
	const long vchk = validateChannel(channel);
	if (vchk != RTN_CMD_SUCCESS)
		return vchk;

	std::lock_guard<std::mutex> lock(g_scIoMutex);
	HAND h = 0;
	if (!getOrOpenHandleLocked(deviceId, h))
		return static_cast<long>(RTN_ERR_OPEN);

	U16 raw = 0;
	short rtn = 0;
	if (isInputIo) {
		rtn = NIO_GetDIBit(h, channel, &raw);
	} else {
		rtn = NIO_GetDOBit(h, channel, &raw);
		if (rtn != RTN_CMD_SUCCESS)
			rtn = readDoBitViaPort(h, channel, &raw);
	}

	if (rtn == RTN_CMD_SUCCESS)
		value = (raw != 0);
	return static_cast<long>(rtn);
}

long writeIoBit(const std::string& deviceId, uint16_t channel, bool value)
{
	const long vchk = validateChannel(channel);
	if (vchk != RTN_CMD_SUCCESS)
		return vchk;

	std::lock_guard<std::mutex> lock(g_scIoMutex);
	HAND h = 0;
	if (!getOrOpenHandleLocked(deviceId, h))
		return static_cast<long>(RTN_ERR_OPEN);

	short rtn = writeDoBitViaPort(h, channel, value);
	if (rtn != RTN_CMD_SUCCESS) {
		const U16 vb = value ? static_cast<U16>(1) : static_cast<U16>(0);
		rtn = NIO_SetDOBit(h, channel, vb);
	}
	return static_cast<long>(rtn);
}

void closeDevice(const std::string& deviceId)
{
	std::lock_guard<std::mutex> lock(g_scIoMutex);
	auto it = g_openById.find(deviceId);
	if (it == g_openById.end())
		return;
	if (it->second != 0)
		NIO_Close(it->second);
	g_openById.erase(it);
}

long probeConnection(const std::string& deviceId)
{
	if (deviceId.empty())
		return static_cast<long>(RTN_LIB_PARA_ERROR);

	std::lock_guard<std::mutex> lock(g_scIoMutex);
	HAND h = 0;
	if (!getOrOpenHandleLocked(deviceId, h))
		return static_cast<long>(RTN_ERR_OPEN);

	U16 buf[kIoPortCount] = {0, 0};
	return static_cast<long>(NIO_GetDO(h, kIoPortStart, kIoPortCount, buf));
}

std::string formatRtn(long rtn)
{
	switch (rtn) {
	case RTN_CMD_SUCCESS:
		return "RTN_CMD_SUCCESS";
	case RTN_CMD_ERROR:
		return "RTN_CMD_ERROR";
	case RTN_LIB_PARA_ERROR:
		return "RTN_LIB_PARA_ERROR";
	case RTN_NO_SUPPORT:
		return "RTN_NO_SUPPORT";
	case RTN_INVALID_HANDLE:
		return "RTN_INVALID_HANDLE";
	case RTN_ERR_OPEN:
		return "RTN_ERR_OPEN";
	case RTN_ERR_CLOSE:
		return "RTN_ERR_CLOSE";
	default:
		return "unknown rtn=" + std::to_string(rtn);
	}
}

bool ioChannelOk(int bitNo)
{
	return bitNo >= 0 && bitNo <= 31;
}

using IoPortReadFn = short (*)(HAND, U16, U16, U16*);

long readIoPorts(IoPortReadFn readFn, std::vector<uint32_t>& vecValue)
{
	vecValue.clear();
	std::lock_guard<std::mutex> lock(g_scIoMutex);
	if (g_openById.empty())
		return static_cast<long>(RTN_ERR_OPEN);

	for (const auto& entry : g_openById) {
		if (entry.second == 0)
			return static_cast<long>(RTN_ERR_OPEN);
		U16 buf[kIoPortCount] = {0, 0};
		const short rtn = readFn(entry.second, kIoPortStart, kIoPortCount, buf);
		if (rtn != RTN_CMD_SUCCESS)
			return static_cast<long>(rtn);
		for (U16 i = 0; i < kIoPortCount; ++i)
			vecValue.push_back(static_cast<uint32_t>(buf[i]));
	}
	return static_cast<long>(RTN_CMD_SUCCESS);
}

} // namespace

long GaoChuanIoMotionCard::initDevice()
{
	if (nioDeviceId().empty()) {
		LOG_ERROR("[common][device][motion]---GaoChuan IO init failed, connectString (NIO device id) is empty");
		return static_cast<long>(RTN_LIB_PARA_ERROR);
	}
	return probeConnection(nioDeviceId());
}

void GaoChuanIoMotionCard::release()
{
	closeDevice(nioDeviceId());
}

long GaoChuanIoMotionCard::writeIO(int bitNo, bool value)
{
	return writeExtendIO(0, bitNo, value);
}

long GaoChuanIoMotionCard::readIO(int bitNo, bool& value, bool isInputIo)
{
	return readExtendIO(0, bitNo, value, isInputIo);
}

long GaoChuanIoMotionCard::readDoPorts(std::vector<uint32_t>& vecValue)
{
	return readIoPorts(NIO_GetDO, vecValue);
}

long GaoChuanIoMotionCard::readDiPorts(std::vector<uint32_t>& vecValue)
{
	return readIoPorts(NIO_GetDI, vecValue);
}

long GaoChuanIoMotionCard::writeExtendIO(int nodeNo, int bitNo, bool value)
{
	(void)nodeNo;
	if (!ioChannelOk(bitNo))
		return static_cast<long>(RTN_LIB_PARA_ERROR);
	return writeIoBit(nioDeviceId(), static_cast<uint16_t>(bitNo), value);
}

long GaoChuanIoMotionCard::readExtendIO(int nodeNo, int bitNo, bool& value, bool isInputIo)
{
	(void)nodeNo;
	if (!ioChannelOk(bitNo))
		return static_cast<long>(RTN_LIB_PARA_ERROR);
	return readIoBit(nioDeviceId(), static_cast<uint16_t>(bitNo), isInputIo, value);
}

bool GaoChuanIoMotionCard::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	if (rtn == 0)
		return true;

	rtnStr = formatRtn(rtn);
	LOG_ERROR("[common][device][motion]---{} failed, sdk_ret={}, sdk_errInfo={}", strFun, rtn, rtnStr);
	return false;
}

} // namespace motion
} // namespace device
} // namespace common
