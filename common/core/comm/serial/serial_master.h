#ifndef __COMMON_CORE_COMM_SERIAL_SERIAL_MASTER_H__
#define __COMMON_CORE_COMM_SERIAL_SERIAL_MASTER_H__

#include <common/3rdparty/serial/include/serial/serial.h>
#include "serial_global.h"
#include "serial_master_opts.h"

namespace common
{

namespace serial
{

class SERIAL_COMM_EXPORT SerialMaster {
public:
	SerialMaster() = default;
	~SerialMaster() = default;
public:
	int init(const SerialInfo& _opts);
	int write(const std::string& _send);
	//读取指定大小数据
	int read(std::string& _recv, uint32_t expectedSize = 1, uint64_t timeout = 6000);
	//读取数据直到读到结束符号为止
	int read(std::string& _recv, const std::string& terminator, uint64_t timeout = 6000);
	//循环读取数据 直到连续三次cycleTime都没有读到数据为止
	int readLoop(std::string& _recv, uint32_t readOnceSize = 10, uint32_t cycleTime = 500, uint64_t timeout = 6000);
private:
	int _read(std::string& _recv, const std::string& terminator, uint32_t expectedSize, uint32_t readOnceSize, uint32_t cycleTime, uint64_t timeout);
    ::serial::Serial mSerial_;
};

}
}
#endif
