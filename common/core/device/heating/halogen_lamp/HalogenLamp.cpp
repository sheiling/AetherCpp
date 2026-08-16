#include "halogenlamp.h"
#include <time.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <array>

//NCP_COMPONENT_INFO_BEGIN(motioncontrol::HalogenLamp, MOTIONCONTROL_HALOGEN_LAMP_COMP_ID)
//NCP_COMPONENT_INFO_VERSION("0.1.0")
//NCP_COMPONENT_INFO_END(motioncontrol::HalogenLamp)

namespace common
{
namespace device
{

const uint16_t TIME_READ_GAP = 5;
const uint16_t TIME_SEND_GAP = 50;
const uint16_t READ_BUFFER_SIZE = 256;

uint16_t  CRC16(unsigned char* arr_buff, unsigned int len) {
    uint16_t crc = 0xFFFF;
    char i, j;
    for (j = 0; j < len; j++) {
        crc = crc ^ arr_buff[j];
        for (i = 0; i < 8; ++i) {
            if ((crc & 0x0001) > 0) {
                crc = crc >> 1;
                crc = crc ^ 0xA001;
            }
            else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

// 16位大小端转换
inline uint16_t swapEndian16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

inline uint32_t swapEndian32(uint32_t value) {
    return ((value >> 24) & 0x000000FF) |
        ((value >> 8) & 0x0000FF00) |
        ((value << 8) & 0x00FF0000) |
        ((value << 24) & 0xFF000000);
}

void printBytesAsHex(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec << std::endl;  // Reset to decimal output
}

HalogenLamp::HalogenLamp() {

}

HalogenLamp::~HalogenLamp() {
    deviceSwitch(false);
    _serialPort.close();
}

bool HalogenLamp::initSerialPort(const LampDevInfo& lampParam) {
    if (_serialPort.isOpen()) {
        _serialPort.close();
    }

    _stationId = _devInfo.slaveId;
    _serialPort.init(_devInfo.port.c_str(), _devInfo.baudrate);
    _serialPort.setOperateMode(itas109::OperateMode::SynchronousOperate);
    //_serialPort.setReadBufferSize(1024);
    if (!_serialPort.open()) {
        return false;
    }
    return prepareDevice();
}

bool HalogenLamp::prepareDevice() {
    if (!deviceSwitch(true) ||
        !setDeviceBatteryMode() ||
        !setDeviceCccvPriority()) {
        return false;
    }
    return true;
}

void HalogenLamp::setBatteryMode(BatteryMode mode) {
    if (_batteryMode != mode) {
        _batteryMode = mode;
    }
}

void HalogenLamp::setU(float u) {
    _u = u;
}

void HalogenLamp::setI(float i) {
    _i = i;
}

//void HalogenLamp::setP(float p) {
//    _p = p;
//}

float HalogenLamp::getU() {
    return getFloatValue(_addrGetU);
}

float HalogenLamp::getI() {
    return getFloatValue(_addrGetI);
}

//float HalogenLamp::getP() {
//    return getFloatValue(_addrGetP);
//}

bool HalogenLamp::deviceSwitch(bool on) {
    bool ret{false};
    if (on) {
        std::vector<uint16_t> vector{0x0000, 0x0001};
        ret = setRegistersValues(_addrSwitch, vector);
    }
    else {
        std::vector<uint16_t> vector{0x0000, 0x0000};
        ret = setRegistersValues(_addrSwitch, vector);
    }
    //std::cout << __FUNCTION__ << ret << '\n';
    return ret;
}

bool HalogenLamp::setDeviceBatteryMode() {
    std::vector<uint16_t> vector{0x0000};
    if (_batteryMode == BatteryMode::Mode_VI) {
        vector.push_back(0x0000);
    }
    else if (_batteryMode == BatteryMode::Mode_CP) {
        vector.push_back(0x007F);
    }
    else if (_batteryMode == BatteryMode::Mode_SEQ) {
        vector.push_back(0x0080);
    }
    bool ret = setRegistersValues(_addrBatteryMode, vector);
    //std::cout << __FUNCTION__ << ret << '\n';
    return ret;
}

bool HalogenLamp::setDeviceCccvPriority() {
    if (_batteryMode == BatteryMode::Mode_VI) {
        std::vector<uint16_t> vector{0x0000, 0x0001};//CV优先
        bool ret = setRegistersValues(_addrCccvPriority, vector);
        //std::cout << __FUNCTION__ << ret << '\n';
        return ret;
    }
    return true;
}

bool HalogenLamp::setDeviceU() {
    bool ret = true;
    if (_batteryMode == BatteryMode::Mode_VI) {
        ret = setFloatValue(_addrVISetU, _u);
    }
    //else if (_batteryMode == BatteryMode::Mode_CP) {
    //    ret = setFloatValue(_addrCPSetU, _u);
    //}
    //std::cout << __FUNCTION__ << ret << '\n';
    return ret;
}

bool HalogenLamp::setDeviceI() {
    bool ret = true;
    if (_batteryMode == BatteryMode::Mode_VI) {
        ret = setFloatValue(_addrVISetI, _i);
    }
    //else if (_batteryMode == BatteryMode::Mode_CP) {
    //    ret = setFloatValue(_addrCPSetI, _i);
    //}
    //std::cout << __FUNCTION__ << ret << '\n';
    return ret;
}

//bool HalogenLamp::setDeviceP() {
//    bool ret = true;
//    if (_batteryMode == BatteryMode::Mode_CP) {
//        ret = setFloatValue(_addrCPSetP, _p);
//    }
//    //std::cout << __FUNCTION__ << ret << '\n';
//    return ret;
//}

bool HalogenLamp::setFloatValue(const uint16_t _addr, const float& value) {
    uint32_t hexValue = *reinterpret_cast<uint32_t*>(&const_cast<float&>(value));
    uint16_t hexHigh = (hexValue >> 16);
    uint16_t hexLow = (hexValue & 0x0000FFFF);
    std::vector<uint16_t> vector{hexHigh, hexLow};
    bool ret = setRegistersValues(_addr, vector);
    //std::cout << __FUNCTION__ << ret << '\n';
    return ret;
}

float HalogenLamp::getFloatValue(const uint16_t _addr) {
    std::vector<uint8_t> vector{};
    if (!getRegistersValues(_addr, vector)) {
        return 0.0;
    }
    if (vector.size() != 4) {
        return 0.0;
    }
    uint16_t high = (vector.at(0) << 8) | vector.at(1);
    uint16_t low = (vector.at(2) << 8) | vector.at(3);
    uint32_t hex = (high << 16) | low;
    float value = 0.0;
    memcpy_s(&value, 4, &hex, 4);
    return value;
}

bool HalogenLamp::getRegistersValues(const uint16_t& regAddr, std::vector<uint8_t>& value, const uint16_t regCount) {
    _serialPort.flushReadBuffers();
    _serialPort.flushWriteBuffers();

    std::array<uint8_t, 8> sendBuf = {0};
    sendBuf[0] = _stationId;
    sendBuf[1] = _funcCodeRead;

    uint16_t addr = swapEndian16(regAddr);
    memcpy_s(sendBuf.data() + 2, 2, &addr, 2);

    uint16_t count = swapEndian16(regCount);
    memcpy_s(sendBuf.data() + 4, 2, &count, 2);

    uint16_t crc = CRC16(sendBuf.data(), 6);
    memcpy_s(sendBuf.data() + 6, 2, &crc, 2);
    if (!sendData(sendBuf.data(), 8)) {
        //std::cout << "send data failed" << '\n';
        return false;
    }

    //printBytesAsHex(sendBuf.data(), 8);

    clock_t startRead = clock();
    std::array<uint8_t, READ_BUFFER_SIZE> readBuf{0};
    int length = 0, targetLen = 5 + 2 * regCount;
    do {
        readData(readBuf.data(), length);
        if (clock() - startRead > 200) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_READ_GAP));
    } while (length < targetLen);

    //printBytesAsHex(readBuf.data(), length);

    if (length < targetLen) {
        return false;
    }

    //处理粘包情况
    int16_t startPos = -1;
    for (int i = 0; i < length; i++) {
        if (readBuf.at(i) == _stationId && readBuf.at(i + 1) == _funcCodeRead) {
            startPos = i;
            break;
        }
    }
    //没找到或包不完整
    if (startPos < 0 || startPos + targetLen > length) {
        return false;
    }
    if (!checkRecvData(readBuf.data() + startPos, targetLen)) {
        return false;
    }
    for (uint16_t i = 0; i < regCount * 2; ++i) {
        value.push_back(readBuf.at(startPos + 3 + i));
    }
    //字节序是反的，如0.351的16进制：3EB3 8646，实际读回来的是8646 3EB3
    //因此需对vector内数据按2字节做反序
    uint8_t* left = value.data(); // 指向 vector 的开始
    uint8_t* right = value.data() + regCount * 2 - 2; // 指向倒数第二个元素
    while (left < right) {
        // 交换成对的元素
        std::swap(left[0], right[0]); // 交换第一对元素
        std::swap(left[1], right[1]); // 交换第二对元素
        left += 2;
        right -= 2;
    }
    return true;
}

bool HalogenLamp::setRegistersValues(const uint16_t& regAddr, const std::vector<uint16_t>& value, const uint16_t regCount) {
    int valueCount = value.size();
    if (valueCount != regCount) {
        return false;
    }
    std::vector<uint16_t> reverseValue{value};//需要将原数据逆序
    std::reverse(reverseValue.begin(), reverseValue.end());

    _serialPort.flushReadBuffers();
    _serialPort.flushWriteBuffers();

    uint16_t totalLen = 9 + 2 * regCount;
    uint8_t* sendBuf = new uint8_t[totalLen];
    sendBuf[0] = _stationId;
    sendBuf[1] = _funcCodeWrite;

    uint16_t addr = swapEndian16(regAddr);
    memcpy_s(sendBuf + 2, 2, &addr, 2);

    uint16_t count = swapEndian16(regCount);
    memcpy_s(sendBuf + 4, 2, &count, 2);

    sendBuf[6] = static_cast<uint8_t>(2 * regCount);
    for (int i = 0; i < valueCount; ++i) {
        uint16_t tmp = swapEndian16(reverseValue.at(i));
        memcpy_s(sendBuf + 7 + 2 * i, 2, &tmp, 2);
    }

    uint16_t crc = CRC16(sendBuf, totalLen - 2);
    memcpy_s(sendBuf + totalLen - 2, 2, &crc, 2);

    bool ret = sendData(sendBuf, totalLen);
    delete[]sendBuf;
    return ret;
}

bool HalogenLamp::sendData(const unsigned char* data, const  int& length) {
    if (!_serialPort.isOpen()) {
        return false;
    }
    int ret = _serialPort.writeData(data, length);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SEND_GAP));
    if (ret < 0) {
        return false;
    }
    _serialPort.flushBuffers();
    return true;
}

bool HalogenLamp::readData(unsigned char* data, int& length) {
    if (!_serialPort.isOpen())
        return false;
    length = _serialPort.readAllData(data);

    std::cout << "read data length: " << length << '\n';
    printBytesAsHex(data, length);

    if (length <= 0) {
        return false;
    }
    return true;
}

bool HalogenLamp::checkRecvData(unsigned char* data, const int& length) {
    if (length < 5) {
        return false;
    }
    uint16_t dataLen = length - 2;
    unsigned char* arrData = new unsigned char[dataLen];
    memcpy_s(arrData, dataLen, data, dataLen);

    unsigned char crcData[2];
    memcpy_s(crcData, 2, data + dataLen, 2);
    int crc = CRC16(arrData, dataLen);
    int number = (crcData[1] << 8) | crcData[0];
    if (crc != number) {//校验失败
        return false;
    }

    return true;
}

bool HalogenLamp::setVoltage(float value) {
    std::cout << "--setVoltage: " << value << std::endl;
    setU(value);
    return setDeviceU();
}

bool HalogenLamp::setCurrent(float value) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << "--setCurrent:" << value << std::endl;
    setI(value);
    return setDeviceI();
}

bool HalogenLamp::setParam(float voltage, float current) {
    std::lock_guard<std::mutex> lock(_mutex);
    bool ret = false;
    if (voltage < 0 || 12 < voltage) {
        std::cout << "电压值异常:" << voltage << '\n';
    }
    else {
        setU(voltage);
        ret = setDeviceU();
    }
    if (current < 0 || 8.33 < current) {
        std::cout << "电流值异常:" << current << '\n';
    }
    else {
        setI(current);
        ret &= setDeviceI();
    }
    //if (p < 0) {
    //    std::cout << "功率异常:" << p << '\n';
    //}
    //else {
    //    setP(p);
    //    setDeviceP();
    //}

    return ret;
}

bool HalogenLamp::getVoltage(float& value) {
    std::lock_guard<std::mutex> lock(_mutex);
    value = getU();
    return true;
}

bool HalogenLamp::getCurrent(float& value) {
    std::lock_guard<std::mutex> lock(_mutex);
    value = getI();
    return true;
}
}

}