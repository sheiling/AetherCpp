#include "wonsor_rangefinder.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <thread>

namespace common
{
namespace device
{

bool isApproximatelyZero(double x, double epsilon = 1e-4) {
	return std::abs(x) < epsilon;
}

/*单精度浮点数（32位） 16进制转10进制*/
double hex32_10(unsigned  char* pData) {
	//符号位 1
	int sign = int(pData[0] >> 7);
	//指数位 8
	int e = int(((pData[0] & 0x7F) << 1) | ((pData[1] >> 7) & 0x01)) - 127;
	//小数位 23
	int im = pData[3] & 0xFF;
	im |= (pData[2] << 8) & 0xFF00;
	im |= (pData[1] << 16) & 0x7F0000;
	double m = pow(2, -23) * im + 1;
	double res = pow(-1, sign) * m * pow(2, e);
	return res;
}


WonsorRangefinder::WonsorRangefinder() {

}

bool WonsorRangefinder::openRangeFinder() {
	_serial.init(_devInfo.serialInfo.com_.c_str(), _devInfo.serialInfo.baud_, 
				 itas109::Parity(_devInfo.serialInfo.parity_), itas109::DataBits(_devInfo.serialInfo.dataBit_),
				 itas109::StopBits(_devInfo.serialInfo.stopBit_));
	_serial.setOperateMode(itas109::OperateMode::SynchronousOperate);
	return _serial.open();
}

bool WonsorRangefinder::measureDistance(float& value) {
	if (!_serial.isOpen()) {
		return false;
	}
	unsigned char sendData[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xc4, 0x0b };
	int bytesSent = _serial.writeData(sendData, sizeof(sendData));
	if (bytesSent <= 0) {
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	unsigned char readData[1024]{ 0 };
	double res = 0.0;
	int bytesRead = 0;
	clock_t startRead = clock();
	do {
		bytesRead = _serial.readAllData(readData);
		if (bytesRead >= 9) {
			break;
		}
		if (clock() - startRead > 20) {
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	} while (bytesRead < 9);

	if (bytesRead < 9) {
		std::cout << "Failed to read data." << std::endl;
		return false;
	}
	auto startPos = -1;
	for (int i = 0; i < bytesRead; ++i) {
		if (static_cast<uint8_t>(readData[i]) == 0x01 && 
			static_cast<uint8_t>(readData[i + 1]) == 0x03 && 
			static_cast<uint8_t>(readData[i + 2]) == 0x04) {
			startPos = i;
		}
	}
	if (startPos < 0) {
		std::cout << "激光测距仪读数帧头信息不对: ";
		for (int i = 0; i < bytesRead; ++i) {
			std::cout << std::hex << static_cast<int>(readData[i]) << " ";
		}
		std::cout << std::endl;
		return false;
	}

	unsigned char dataBuf[4] = { 0 };
	dataBuf[0] = readData[startPos + 5];
	dataBuf[1] = readData[startPos + 6];
	dataBuf[2] = readData[startPos + 3];
	dataBuf[3] = readData[startPos + 4];
	res = hex32_10(dataBuf);
	if (isnan(res)) {
		return false;
	}
	if (std::abs(res) >= 999) {
		return false;
	}
	if (isApproximatelyZero(res)) {//认为是错误的值，不给
		std::cout << "激光测距仪约等于0" << std::endl;
		//value = 1;
		return false;
	}
	else {
		value = res * 1000;
	}
	return true;
}

}

}
