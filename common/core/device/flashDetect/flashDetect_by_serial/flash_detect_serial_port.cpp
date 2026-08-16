#include "flash_detect_serial_port.h"

#include <Windows.h>
#include <Logger.h>
#include <cstring>

namespace common
{
namespace device
{

namespace
{
// 统一做一次句柄显式转换，减少重复 static_cast。
HANDLE ToHandle(void* handle) {
    return static_cast<HANDLE>(handle);
}
} // namespace

FlashDetectSerialPort::FlashDetectSerialPort()
    : handle_(nullptr), useSynchronousIo_(false) {}

FlashDetectSerialPort::~FlashDetectSerialPort() {
    Close();
}

bool FlashDetectSerialPort::Open(const char* portName, const Parameters& param) {
    // 支持同步/异步两种模式，默认使用 Overlapped 异步。
    const DWORD flags = useSynchronousIo_ ? 0 : FILE_FLAG_OVERLAPPED;
    HANDLE hCom = CreateFileA(portName,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              flags,
                              NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        LOG_ERROR("[FlashDetectSerialPort] open failed, port={}, err={}", portName, GetLastError());
        return false;
    }

    // 配置串口收发缓冲区大小。
    if (!SetupComm(hCom, 1024, 1024)) {
        LOG_ERROR("[FlashDetectSerialPort] setup comm buffer failed, port={}, err={}", portName, GetLastError());
        CloseHandle(hCom);
        return false;
    }

    // 配置串口参数（波特率、数据位、校验位、停止位）。
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = param.baudRate;
    dcb.ByteSize = static_cast<BYTE>(param.dataBits);
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    switch (param.parity) {
        case Parity::NoParity: dcb.Parity = NOPARITY; break;
        case Parity::OddParity: dcb.Parity = ODDPARITY; break;
        case Parity::EvenParity: dcb.Parity = EVENPARITY; break;
    }

    switch (param.stopBits) {
        case StopBits::One: dcb.StopBits = ONESTOPBIT; break;
        case StopBits::OneAndHalf: dcb.StopBits = ONE5STOPBITS; break;
        case StopBits::Two: dcb.StopBits = TWOSTOPBITS; break;
    }

    if (!SetCommState(hCom, &dcb)) {
        LOG_ERROR("[FlashDetectSerialPort] set comm state failed, port={}, err={}", portName, GetLastError());
        CloseHandle(hCom);
        return false;
    }

    // 读写超时策略：避免异常情况下阻塞线程过久。
    COMMTIMEOUTS timeout;
    timeout.ReadIntervalTimeout = 100;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.ReadTotalTimeoutConstant = 500;
    timeout.WriteTotalTimeoutMultiplier = 2000;
    timeout.WriteTotalTimeoutConstant = 50;
    SetCommTimeouts(hCom, &timeout);

    PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
    handle_ = hCom;
    LOG_INFO("[FlashDetectSerialPort] open success, port={}, sync_io={}", portName, useSynchronousIo_);
    return true;
}

void FlashDetectSerialPort::Close() {
    HANDLE hCom = ToHandle(handle_);
    if (hCom != nullptr && hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(hCom);
        LOG_INFO("[FlashDetectSerialPort] close success");
    }
    handle_ = nullptr;
}

int FlashDetectSerialPort::Send(const void* buffer, int len) {
    HANDLE hCom = ToHandle(handle_);
    if (hCom == nullptr || hCom == INVALID_HANDLE_VALUE || buffer == nullptr || len <= 0) {
        LOG_ERROR("[FlashDetectSerialPort] send invalid args, len={}, handle_valid={}",
                  len,
                  hCom != nullptr && hCom != INVALID_HANDLE_VALUE);
        return 0;
    }

    if (useSynchronousIo_) {
        // 同步发送：WriteFile 直接返回写入结果。
        DWORD writeLen = static_cast<DWORD>(len);
        if (!WriteFile(hCom, buffer, writeLen, &writeLen, NULL)) {
            LOG_ERROR("[FlashDetectSerialPort] sync write failed, err={}", GetLastError());
            return 0;
        }
        return static_cast<int>(writeLen);
    }

    // 异步发送：通过 Overlapped + 事件等待完成。
    OVERLAPPED osWrite;
    memset(&osWrite, 0, sizeof(osWrite));
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL) {
        LOG_ERROR("[FlashDetectSerialPort] create write event failed, err={}", GetLastError());
        return 0;
    }

    DWORD writeLen = 0;
    if (!WriteFile(hCom, buffer, static_cast<DWORD>(len), &writeLen, &osWrite)) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            DWORD waitRet = WaitForSingleObject(osWrite.hEvent, 1000);
            if (waitRet == WAIT_OBJECT_0) {
                GetOverlappedResult(hCom, &osWrite, &writeLen, TRUE);
            } else if (waitRet == WAIT_TIMEOUT) {
                LOG_WARN("[FlashDetectSerialPort] async write timeout");
            } else {
                LOG_ERROR("[FlashDetectSerialPort] async write wait failed, wait_ret={}", waitRet);
            }
        } else {
            LOG_ERROR("[FlashDetectSerialPort] async write failed, err={}", err);
        }
    }

    CloseHandle(osWrite.hEvent);
    return static_cast<int>(writeLen);
}

int FlashDetectSerialPort::Receive(void* buffer, int maxLen) {
    HANDLE hCom = ToHandle(handle_);
    if (hCom == nullptr || hCom == INVALID_HANDLE_VALUE || buffer == nullptr || maxLen <= 0) {
        LOG_ERROR("[FlashDetectSerialPort] receive invalid args, max_len={}, handle_valid={}",
                  maxLen,
                  hCom != nullptr && hCom != INVALID_HANDLE_VALUE);
        return 0;
    }

    if (useSynchronousIo_) {
        // 同步读取：ReadFile 直接返回读取结果。
        DWORD readLen = static_cast<DWORD>(maxLen);
        if (!ReadFile(hCom, buffer, readLen, &readLen, NULL)) {
            LOG_ERROR("[FlashDetectSerialPort] sync read failed, err={}", GetLastError());
            return 0;
        }
        return static_cast<int>(readLen);
    }

    // 异步读取：通过 Overlapped + 事件等待完成。
    OVERLAPPED osRead;
    memset(&osRead, 0, sizeof(osRead));
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osRead.hEvent == NULL) {
        LOG_ERROR("[FlashDetectSerialPort] create read event failed, err={}", GetLastError());
        return 0;
    }

    DWORD readLen = 0;
    if (!ReadFile(hCom, buffer, static_cast<DWORD>(maxLen), &readLen, &osRead)) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            DWORD waitRet = WaitForSingleObject(osRead.hEvent, 2000);
            if (waitRet == WAIT_OBJECT_0) {
                GetOverlappedResult(hCom, &osRead, &readLen, TRUE);
            } else if (waitRet == WAIT_TIMEOUT) {
                LOG_WARN("[FlashDetectSerialPort] async read timeout");
            } else {
                LOG_ERROR("[FlashDetectSerialPort] async read wait failed, wait_ret={}", waitRet);
            }
        } else {
            LOG_ERROR("[FlashDetectSerialPort] async read failed, err={}", err);
        }
    }

    CloseHandle(osRead.hEvent);
    return static_cast<int>(readLen);
}

} // namespace device
} // namespace common
