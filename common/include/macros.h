#ifndef _COMMON_MACROS_H_
#define _COMMON_MACROS_H_

#include <common/utility/errors/include/error.h>
#include <Logger.h>

#include <common/core/messagecenter/message_center.h>

#include <spdlog/fmt/fmt.h>

#define CHECK_RTN_INFO(pFun)\
do\
{\
    common::errors::ErrorPtr FUN_Rtn_ResultInfo = pFun;\
    if(FUN_Rtn_ResultInfo)\
        return FUN_Rtn_ResultInfo;\
}while(false)

#define CHECK_RTN_INFO_LOG(pFun)\
do\
{\
    common::errors::ErrorPtr FUN_Rtn_ResultInfo = pFun;\
    if(FUN_Rtn_ResultInfo){\
        LOG_ERROR("{}", FUN_Rtn_ResultInfo->Message());\
        return FUN_Rtn_ResultInfo;\
    }\
}while(false)

#define CHECK_RTN_LONG(pFun)\
do\
{\
    long pFUN_Rtn = pFun;\
    if(pFUN_Rtn != 0)\
        return pFUN_Rtn;\
}while(false)

#define CHECK_BREAK_INFO(pFun)\
{\
	common::errors::ErrorPtr FUN_Rtn_ResultInfo = pFun; if (FUN_Rtn_ResultInfo) break;\
} 

// 写日志并上报 UI 运行日志（EVENT_RUNTIME_LOG：级别 + 消息正文）
#define LOG_TRACE_REPORT(...) \
do { \
    LOG_TRACE(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("TRACE"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#define LOG_DEBUG_REPORT(...) \
do { \
    LOG_DEBUG(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("DEBUG"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#define LOG_INFO_REPORT(...) \
do { \
    LOG_INFO(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("INFO"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#define LOG_WARN_REPORT(...) \
do { \
    LOG_WARN(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("WARN"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#define LOG_ERROR_REPORT(...) \
do { \
    LOG_ERROR(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("ERROR"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#define LOG_CRITICAL_REPORT(...) \
do { \
    LOG_CRITICAL(__VA_ARGS__); \
    MessageCenter::Center::Notify(EVENT_RUNTIME_LOG, QStringLiteral("CRITICAL"), \
        QString::fromStdString(fmt::format(__VA_ARGS__))); \
} while (0)

#endif
