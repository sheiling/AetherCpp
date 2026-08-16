#ifndef __COMMON_UTILITY_STRING_FORMAT_H__
#define __COMMON_UTILITY_STRING_FORMAT_H__

#include <vector>
#include <string>
#include <sstream>
#include <Windows.h>
#include <iomanip>
#include <algorithm>

namespace common
{

namespace utility
{

class StringFormat {
public:
	StringFormat() = default;
	~StringFormat() = default;
public:
	static std::vector<std::string> splitString(const std::string& str, const std::string& symbol = ",") {
		std::vector<std::string> result;
		std::string::size_type start = 0;
		std::string::size_type end = str.find(symbol);
		
		while (end != std::string::npos) {
			result.push_back(str.substr(start, end - start));
			start = end + symbol.length();
			end = str.find(symbol, start);
		}
		if (start < str.length()) { // 处理最后一个元素后面可能有分隔符的情况
			result.push_back(str.substr(start));
		}
		return result;
	}
	static std::string jointString(const std::vector<std::string>& vec, const std::string& symbol = ",") {
		std::string result = "";
		
		for (auto s : vec)
		{
			if (!result.empty())
				result += symbol;
			result += s;
		}
		return result;
	}
	static std::string leftString(const std::string& str, int npos) {
		if (str.length() < npos || npos <= 0)
			return "";
		return str.substr(0, npos);
	}
	static std::string rightString(const std::string& str, int npos) {
		if (str.length() < npos || npos <= 0)
			return "";
		return str.substr(str.length() - npos, npos);
	}
	template<typename ... Args>
	static std::string string_format(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}
	// UTF8字符串转成GBK字符串
	static std::string U2G(const std::string& utf8)
	{
	    int nwLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	    wchar_t* pwBuf = new wchar_t[nwLen + 1];//加1用于截断字符串 
	    memset(pwBuf, 0, nwLen * 2 + 2);
	    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, pwBuf, nwLen);
	    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	    char* pBuf = new char[nLen + 1];
	    memset(pBuf, 0, nLen + 1);
	    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	    std::string retStr = pBuf;
	    delete[]pBuf;
	    delete[]pwBuf;
	    pBuf = NULL;
	    pwBuf = NULL;
	    return retStr;
	}
	// GBK字符串转成UTF8字符串
	static std::string G2U(const std::string& gbk)
	{
	    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
	    wchar_t* pwBuf = new wchar_t[nwLen + 1];//加1用于截断字符串 
	    ZeroMemory(pwBuf, nwLen * 2 + 2);
	    ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, pwBuf, nwLen);
	    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	    char* pBuf = new char[nLen + 1];
	    ZeroMemory(pBuf, nLen + 1);
	    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	    std::string retStr(pBuf);
	    delete[]pwBuf;
	    delete[]pBuf;
	    pwBuf = NULL;
	    pBuf = NULL;
	    return retStr;
	}
	static std::string WString2String(const std::wstring& wstr) {
		if (wstr.empty()) {
			return {};
		}
		const int byteLen = ::WideCharToMultiByte(
			CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (byteLen <= 0) {
			return {};
		}
		std::string result(static_cast<size_t>(byteLen - 1), '\0');
		::WideCharToMultiByte(
			CP_UTF8, 0, wstr.c_str(), -1, result.data(), byteLen, nullptr, nullptr);
		return result;
	}
	static std::wstring String2WString(const std::string& mbs) {
		if (mbs.empty()) {
			return {};
		}
		const int wideLen = ::MultiByteToWideChar(
			CP_UTF8, 0, mbs.c_str(), -1, nullptr, 0);
		if (wideLen <= 0) {
			return {};
		}
		std::wstring result(static_cast<size_t>(wideLen - 1), L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0, mbs.c_str(), -1, result.data(), wideLen);
		return result;
	}

	static std::string Byte2HexStr(unsigned char* mData, int length) {
		std::string strTemp = "";

    	for (int i = 0; i < length; i++)
    	{

			std::stringstream ss;
    		ss << std::hex << std::setw(2) << std::setfill('0') << (int)mData[i];
    		std::string hex_str = ss.str();
    		transform(hex_str.begin(), hex_str.end(), hex_str.begin(), ::toupper);

    	    strTemp += hex_str;
    	    strTemp += " ";
    	}

    	return strTemp;
	}

};

}

}

#endif
