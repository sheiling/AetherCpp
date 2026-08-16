#include "serial_master.h"
#include <iostream>
#include <thread>
#include <common/utility/string_tool/time_format.hpp>
#include <common/utility/string_tool/string_format.hpp>

namespace common
{

namespace serial
{
int SerialMaster::init(const SerialInfo& _opts) {
	mSerial_.setPort(_opts.com_);
	mSerial_.setBaudrate(_opts.baud_);
	auto timeout = ::serial::Timeout::simpleTimeout(_opts.rspTimeout_);
	mSerial_.setTimeout(timeout);
	mSerial_.setBytesize((::serial::bytesize_t)_opts.dataBit_);
	mSerial_.setParity((::serial::parity_t)_opts.parity_);
	mSerial_.setStopbits((::serial::stopbits_t)_opts.stopBit_);

	try {		
		mSerial_.open();
		if (mSerial_.isOpen() == false) {
			return 1;
		}
	}
	catch (const std::exception&) {
		return -1;
	}
	return 0;
}

int SerialMaster::write(const std::string& _send) {
	int ret = 0;
	try {
		ret = static_cast<int>(mSerial_.write(_send));
	} catch (const std::exception&) {
		ret = -1;
	}
	return ret;
}


int SerialMaster::read(std::string& _recv, uint32_t expectedSize, uint64_t timeout) {
	return _read(_recv, "", expectedSize, 1, 0, timeout);
}
int SerialMaster::read(std::string& _recv, const std::string& terminator, uint64_t timeout) {
	return _read(_recv, terminator, 0, 1, 0, timeout);
}
int SerialMaster::readLoop(std::string& _recv, uint32_t readOnceSize, uint32_t cycleTime, uint64_t timeout) {
	return _read(_recv, "", 0, readOnceSize, cycleTime, timeout);
}

int SerialMaster::_read(std::string& _recv, const std::string& terminator, uint32_t expectedSize, uint32_t readOnceSize, uint32_t cycleTime, uint64_t timeout)
{
	uint64_t beginTm = utility::TimeFormat::GetCurrentMs();

	std::string readOnce;			//一次读到的数据	
	uint32_t readSize = 0;			//已经读到的个数
	int delayTime = 50;				//循环延时时长
	int ret = 0;											
	int notReadTimes = 0;			//没有读到数据的次数计数
	if (readOnceSize <= 0)
		readOnceSize = 1;

	do {
		// 超时判断
		uint64_t currentTm = utility::TimeFormat::GetCurrentMs();
		if (timeout < (currentTm - beginTm)) {
			return 0;
		}
		try {
			readOnce = "";
			ret = static_cast<int>(mSerial_.read(readOnce, readOnceSize));
		}
		catch (const std::exception&) {
			ret = -1;
			break;
		}
		if (ret < 0)		//读取失败
			break;

		readSize += ret;
		_recv += readOnce;
		if (!terminator.empty()) {
			if (terminator == common::utility::StringFormat::rightString(
					_recv, static_cast<int>(terminator.length())))
				break;
		}
		else {
			if (expectedSize > 0 && cycleTime <= 0) {
				if(readSize >= expectedSize)
					break;
			}
			else if (cycleTime > 0 && expectedSize <= 0) {
				delayTime = cycleTime;

				if (ret > 0)
				{
					// 有读到数据的时候不再延时  立即读下一次
					notReadTimes = 0;
					continue;
				}
				else 
					notReadTimes++;

				//连续3次没有数据 则返回
				if (notReadTimes >= 3){
					break;
				}
			}
			else {
				// 读一次数据就返回
				break;
			}
		} 

		if (delayTime > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
		}
	} while (1);

	return readSize;
}

}

}

