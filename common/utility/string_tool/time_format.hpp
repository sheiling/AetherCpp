#ifndef __COMMON_UTILITY_TIME_FORMAT_H__
#define __COMMON_UTILITY_TIME_FORMAT_H__

#include <iostream>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace common
{

namespace utility
{
class TimeFormat {
public:
	TimeFormat() = default;
	~TimeFormat() = default;
public:
	static uint64_t GetCurrentMs() {
		auto now = std::chrono::system_clock::now();
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto value = now_ms.time_since_epoch().count();
		return value;
	}

	// 获取当前时间 仅年月日  %Y[format]%m[format]%d格式
	static std::string GetCurrentDayStr(std::string format = "") {
		// 获取当前系统时间
		auto now = std::chrono::system_clock::now();
		// 将时间转换为时间戳
		auto time_t_now = std::chrono::system_clock::to_time_t(now);
		// 将时间戳转换为本地时间
		std::tm tm_now{};
		if (!FillLocalTm(time_t_now, tm_now)) {
			return {};
		}
		// 将日期格式化为 "YYYYmmDD" 的字符串
		char date_str[11] = { 0 };
		if (format.empty()) {
			std::strftime(date_str, sizeof(date_str), "%Y%m%d", &tm_now);
		}
		else if (format == ":") {
			std::strftime(date_str, sizeof(date_str), "%Y:%m:%d", &tm_now);
		}
		else if (format == "-") {
			std::strftime(date_str, sizeof(date_str), "%Y-%m-%d", &tm_now);
		}
		else if (format == ".") {
			std::strftime(date_str, sizeof(date_str), "%Y.%m.%d", &tm_now);
		}
		else {
			std::strftime(date_str, sizeof(date_str), "%Y%m%d", &tm_now);
		}
		return std::string(date_str);
	}

	// 获取当前时间  YYYYmmDDHHMMSS格式
	static std::string GetCurrentTimeStr() {
		// 获取当前系统时间
		auto now = std::chrono::system_clock::now();
		// 将时间转换为时间戳
		auto time_t_now = std::chrono::system_clock::to_time_t(now);
		// 将时间戳转换为本地时间
		std::tm tm_now{};
		if (!FillLocalTm(time_t_now, tm_now)) {
			return {};
		}
		// 将日期格式化为 "YYYYmmDDHHMMSS" 的字符串
		char date_str[11] = { 0 };
		std::strftime(date_str, sizeof(date_str), "%Y%m%d", &tm_now);
		char time_str[11] = { 0 };
		std::strftime(time_str, sizeof(time_str), "%H%M%S", &tm_now);
		return (std::string(date_str) + std::string(time_str));
	}

	// 获取当前时间
	static std::string getCurrentTime(std::string format = "%Y-%m-%d_%H-%M-%S") {
		auto now = std::chrono::system_clock::now();
		std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm{};
		if (!FillLocalTm(timestamp, local_tm)) {
			return {};
		}
		char buffer[80];
		std::strftime(buffer, sizeof(buffer), format.c_str(), &local_tm);
		return std::string(buffer);
	}

	enum TimeStampType {
		TYPE_S = 0,
		TYPE_MS = 1,
		TYPE_US = 2,
		TYPE_NS = 3,
	};

	static std::string GetCurrentTimeStamp(TimeStampType timeStampType) {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm now_tm{};
		if (!FillLocalTm(now_time_t, now_tm)) {
			return {};
		}
		char buffer[128];
		strftime(buffer, sizeof(buffer), "%F %T", &now_tm);
		std::ostringstream ss;
		ss.fill('0');
		std::chrono::milliseconds ms;
		std::chrono::microseconds cs;
		std::chrono::nanoseconds ns;
		switch (timeStampType) {
			case TYPE_S:
			ss << buffer;
			break;
			case TYPE_MS:
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
			ss << buffer << ":" << std::setw(3) << ms.count();
			break;
			case TYPE_US:
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
			cs = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
			ss << buffer << ":" << ms.count() << ":" << cs.count() % 1000;
			break;
			case TYPE_NS:
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
			cs = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
			ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()) % 1000000000;
			ss << buffer << ":" << ms.count() << ":" << cs.count() % 1000 << ":" << ns.count() % 1000;
			break;
			default:
			ss << buffer;
			break;
		}
		return ss.str();
	}

	static std::string UnixTimeToString(int64_t unixTamp, char const* format) {
		const std::time_t stamp = static_cast<std::time_t>(unixTamp);
		std::tm stamp_tm{};
		if (!FillLocalTm(stamp, stamp_tm)) {
			return {};
		}
		std::ostringstream os;
		os << std::put_time(&stamp_tm, format);
		return os.str();
	}

	//从字符串格式化成时间戳
	static __time64_t FormatTimeForStr(std::string time_string, std::string format = "%Y-%m-%d_%H-%M-%S")
	{
		std::istringstream is(time_string);
		tm time_tm = {};
		is >> std::get_time(&time_tm, format.c_str());

		return mktime(&time_tm);
	}

	//获取时间  带毫秒
    static std::string getCurrentTimeMs(std::string format = "%Y-%m-%d %H:%M:%S") {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm local_tm{};
        if (!FillLocalTm(now_time_t, local_tm)) {
            return {};
        }

        std::ostringstream oss;
        oss << std::put_time(&local_tm, format.c_str());
        oss << "." << std::setw(3) << std::setfill('0') << now_ms.count();
        return oss.str();
	}

private:
	/** 线程安全地将 time_t 转为本地 tm（Windows: localtime_s；POSIX: localtime_r） */
	static bool FillLocalTm(std::time_t timestamp, std::tm& out) {
#if defined(_MSC_VER)
		return ::localtime_s(&out, &timestamp) == 0;
#elif defined(__unix__) || defined(__APPLE__)
		return ::localtime_r(&timestamp, &out) != nullptr;
#else
		std::tm* p = std::localtime(&timestamp);
		if (!p) {
			return false;
		}
		out = *p;
		return true;
#endif
	}
};

}
}

#endif
