#ifndef __COMMON_UTILITY_FILE_MANAGER_H__
#define __COMMON_UTILITY_FILE_MANAGER_H__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include<Windows.h>
#elif defined(linux) || defined(__linux)
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <corecrt_io.h>
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cctype>

namespace common
{
namespace utility
{

class FileManager {
public:
	FileManager() = default;
	~FileManager() = default;
public:
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

	static HMODULE getSelfModuleHandle() {
		MEMORY_BASIC_INFORMATION mbi;
		return (
			(::VirtualQuery(getSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
			? (HMODULE)mbi.AllocationBase : NULL
			);
	}

	static std::string getCurrentProgramDirectory() {
		std::string strCurrentPath = "";
		char curDirector[260] = { 0 };
		GetModuleFileNameA(getSelfModuleHandle(), curDirector, 260);
		strCurrentPath = curDirector;

		size_t nameStart = strCurrentPath.rfind("\\");
		strCurrentPath = strCurrentPath.substr(0, nameStart + 1);
		return strCurrentPath;
	}

#elif defined(linux) || defined(__linux)

	static std::string getCurrentProgramDirectory() {
		std::string strCurrentPath = "";
		char szCurWorkPath[256];
		memset(szCurWorkPath, '\0', 256);
		int nRet = readlink("/proc/self/exe", szCurWorkPath, 256);
		if (nRet > 256 || nRet < 0)
		{
			return strCurrentPath;
		}

		for (int i = nRet; i > 0; i--)
		{
			if (szCurWorkPath[i] == '/' || szCurWorkPath[i] == '\\')
			{
				szCurWorkPath[i] = '\0';
				break;
			}
		}

		strCurrentPath = szCurWorkPath;

		return strCurrentPath;
	}

#endif

	//获取当前路径  exe绝对路径
	static std::string getCurrentPath() {
		char buffer[MAX_PATH * 2 + 1] = { 0 };
#if defined(_WIN32)
		GetModuleFileNameA(NULL, buffer, sizeof(buffer));
#elif defined(__linux__)
		readlink("/proc/self/exe", buffer, sizeof(buffer));
#endif
		return std::string(buffer);
	}
	//获取当前目录  exe所在的目录  最后面带"\"
	static std::string getCurrentDir() {
		auto path = getCurrentPath();
#if defined(_WIN32)
		return path.substr(0, path.rfind('\\') + 1);
#elif defined(__linux__)
		return path.substr(0, path.rfind('/') + 1);
#endif			
	}

#if defined(_WIN32)
	//创建文件夹，递归创建路径
	static bool createDir(const std::string& path) {
		DWORD dwAttrib = GetFileAttributesA(path.c_str());

		// 如果路径已存在且是一个目录，直接返回true
		if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			return true;

		// 递归创建父目录
		size_t pos = path.find_last_of("\\/");
		if (pos != std::string::npos) {
			if (!createDir(path.substr(0, pos)))
				return false;
		}

		// 创建当前目录
		return CreateDirectoryA(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
	}
#elif defined(__linux__)
	//创建文件夹
	static bool createDir(const std::string& dirname)
	{
		if (access(dirname.c_str(), 0) == -1)
		{
			auto result = _mkdir(dirname.c_str());          // 创建新文件夹
			return result == 0 ? true : false;             //返回0 表示创建成功 否则创建失败
		}
		return false;
	}
#endif
	
	//判断文件夹是否存在
	static bool folderExist(const std::string& dirname) {
#if defined(_WIN32)
		DWORD dwAttrib = GetFileAttributesA(dirname.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#elif defined(__linux__)
		struct stat info;
		if (stat(dirname.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & S_IFDIR) != 0;
#else
		// Other platforms not implemented
		return false;
#endif
		
	}
	
	//删除文件夹（递归删除所有文件和子目录）
	static bool deleteDir(const std::string& dirname) {
		if (!folderExist(dirname)) {
			return false;
		}

		// 构建搜索路径
		std::string searchPath = dirname;
		if (searchPath.back() != '\\' && searchPath.back() != '/') {
			searchPath += "\\";
		}
		searchPath += "*";

		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

		if (hFind == INVALID_HANDLE_VALUE) {
			// 目录为空或无法访问，尝试直接删除
			return RemoveDirectoryA(dirname.c_str()) != 0;
		}

		bool success = true;
		do {
			// 跳过 "." 和 ".."
			if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
				continue;
			}

			std::string filePath = dirname;
			if (filePath.back() != '\\' && filePath.back() != '/') {
				filePath += "\\";
			}
			filePath += findData.cFileName;

			// 如果是目录，递归删除
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!deleteDir(filePath)) {
					success = false;
				}
			}
			// 如果是文件，删除文件
			else {
				// 如果是只读文件，先移除只读属性
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					SetFileAttributesA(filePath.c_str(), FILE_ATTRIBUTE_NORMAL);
				}
				if (DeleteFileA(filePath.c_str()) == 0) {
					success = false;
				}
			}
		} while (FindNextFileA(hFind, &findData) != 0);

		FindClose(hFind);

		// 删除目录本身
		if (success) {
			if (RemoveDirectoryA(dirname.c_str()) == 0) {
				success = false;
			}
		}

		return success;
	}

	//保存文件
	static bool saveToFile(const std::string& filename, const std::string& content) {
		std::string dir = getFolderPath(filename);
		if (!folderExist(dir)){
			if (!createDir(dir)) {
				return false;
			}
		}
		std::ofstream file(filename);
		if (file.is_open()) {
			file << content;
			file.close();
			return true; // 文件保存成功
		}
		else {
			std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
			return false; // 文件保存失败
		}
	}
	
	//判断文件是否存在
	static bool fileExist(const char* path) {
		std::ifstream f(path);
		return f.good();
	}
	
	//判断文件是否已经打开
	static bool fileIsOpen(const char* path) {
		if (FILE* file = fopen(path, "r")) {
			fclose(file);
			return true;
		}
		else {
			return false;
		}
	}
	
	//获取文件列表
	static std::vector<std::string> getFiles(std::string strDir)
	{
		std::vector<std::string> files;//存放文件名

#ifdef WIN32
		_finddata_t file;
		intptr_t lf;
		//输入文件夹路径
		std::string p;
		if ((lf = _findfirst(p.assign(strDir).append("\\*").c_str(), &file)) == -1) {
			std::cout << strDir << " not found!!!" << std::endl;
		}
		else {
			while (_findnext(lf, &file) == 0) {
				if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
					continue;
				if (file.attrib & _A_SUBDIR)  //跳过目录
					continue;
				files.push_back(file.name);
			}
		}
		_findclose(lf);
#endif
#ifdef linux
		DIR* dir;
		struct dirent* ptr;
		char base[1000];

		if ((dir = opendir(cate_dir.c_str())) == NULL)
		{
			perror("Open dir error...");
			exit(1);
		}

		while ((ptr = readdir(dir)) != NULL)
		{
			if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
				continue;
			else if (ptr->d_type == 8)    ///file
				//printf("d_name:%s/%s\n",basePath,ptr->d_name);
				files.push_back(ptr->d_name);
			else if (ptr->d_type == 10)    ///link file
				//printf("d_name:%s/%s\n",basePath,ptr->d_name);
				continue;
			else if (ptr->d_type == 4)    ///dir
			{
				files.push_back(ptr->d_name);
			}
		}
		closedir(dir);
#endif
		return files;
	}
	
	//获取文件夹列表  获取目录下所有子文件夹名称
	static std::vector<std::string> getFolders(const std::string& strDir) {
		std::vector<std::string> folders;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		_finddata_t file;
		intptr_t lf;
		std::string searchPath = strDir;
		if (!searchPath.empty() && searchPath.back() != '\\' && searchPath.back() != '/')
			searchPath += "\\*";
		else
			searchPath += "*";
		if ((lf = _findfirst(searchPath.c_str(), &file)) == -1) {
			std::cout << strDir << " not found!!!" << std::endl;
		}
		else {
			do {
				// 跳过 "." 和 ".."
				if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
					continue;
				// 只加入子目录（文件夹）
				if (file.attrib & _A_SUBDIR)
					folders.push_back(file.name);
			} while (_findnext(lf, &file) == 0);
			_findclose(lf);
		}
#elif defined(linux) || defined(__linux)
		DIR* dir;
		struct dirent* ptr;
		if ((dir = opendir(strDir.c_str())) == NULL) {
			perror("Open dir error...");
			return folders;
		}
		while ((ptr = readdir(dir)) != NULL) {
			// 跳过 "." 和 ".."
			if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
				continue;
			// 只加入类型为目录的项
#if defined(__APPLE__) || defined(__MACH__)
			if (ptr->d_type == DT_DIR)
				folders.push_back(ptr->d_name);
#else
			if (ptr->d_type == 4) // 4 == DT_DIR
				folders.push_back(ptr->d_name);
#endif
		}
		closedir(dir);
#endif
		return folders;
	}

	//获取文件路径的父目录
	static std::string getFolderPath(const std::string& path) {
		const size_t found = path.find_last_of("/\\");
		return (found == std::string::npos) ? path : path.substr(0, found);
	}
	
	//获取文件路径的文件名称(不带目录名)
	static std::string getFileName(const std::string& path) {
		const size_t found = path.find_last_of("/\\");
		return (found == std::string::npos) ? path : path.substr(found + 1);
	}
	
	//获取文件路径的文件名称(不带目录名且不带后缀名)
	static std::string getFileNameStem(const std::string& path) {
		//查找最后一个路径分隔符
    	size_t last_slash = path.find_last_of("/\\");
    	std::string filename = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
    	// 查找最后一个点（扩展名分隔符）
    	size_t last_dot = filename.find_last_of('.');
    	if (last_dot != std::string::npos && last_dot > 0) {
        	return filename.substr(0, last_dot);
    	}
    	return filename;
	}
	
	//获取文件路径的文件后缀名
	static std::string getFileSuffix(const std::string& path) {
		// 查找最后一个点（扩展名分隔符）
		size_t last_dot = path.find_last_of('.');
		if (last_dot != std::string::npos && last_dot > 0) {
			return path.substr(last_dot + 1);
		}
		return path;
	}
	
	//检查文件后缀名
	static bool checkSuffix(std::string str, std::string suffix)
	{
		auto pos = str.rfind(suffix);
		return pos != std::string::npos && pos == str.size() - suffix.size();
	}

	//路径优化
	static std::string normalizeWindowsPath(const std::string& input) 
	{
    	std::string normalized;
    	normalized.reserve(input.size());

    	// 1. 转换所有 '/' 为 Windows 标准反斜杠 '\'
    	std::string temp = input;
    	std::replace(temp.begin(), temp.end(), '/', '\\');

    	// 2. 处理 UNC 路径前缀的冗余反斜杠（如 \\\\server → \\server）
    	bool is_unc = temp.size() >= 2 && temp[0] == '\\' && temp[1] == '\\';
    	size_t start = 0;

    	if (is_unc) {
    	    normalized += "\\\\";
    	    start = 2;
    	    // 跳过 UNC 前缀后的冗余反斜杠（直到遇到非反斜杠字符）
    	    while (start < temp.size() && temp[start] == '\\') {
    	        start++;
    	    }
    	}

    	// 3. 遍历字符串，合并连续反斜杠为一个，并保留非反斜杠字符
    	bool prev_slash = false;
    	for (size_t i = start; i < temp.size(); ++i) {
    	    if (temp[i] == '\\') {
    	        if (!prev_slash) {
    	            normalized += '\\';
    	            prev_slash = true;
    	        }
    	    } else {
    	        normalized += temp[i];
    	        prev_slash = false;
    	    }
    	}

    	// 4. 处理盘符路径后的冗余反斜杠（如 C:\\\\ → C:\）
    	if (!is_unc && normalized.size() >= 3) {
    	    if (std::isalpha(normalized[0]) && normalized[1] == ':' && normalized[2] == '\\') {
    	        // 从位置 3 开始，确保之后的反斜杠被合并
    	        size_t pos = 3;
    	        while (pos < normalized.size() && normalized[pos] == '\\') {
    	            normalized.erase(pos, 1);
    	        }
    	    }
    	}

    	return normalized;
	}


	// 判断一个路径是相对路径还是绝对路径
	static bool isAbsolutePath(const std::string& path) {
		if (path.empty()) {
			return false;
		}
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		// Windows路径判断
		
		// 1. 驱动器路径：C:\ 或 C:/
		if (path.length() >= 3) {
			if (std::isalpha(static_cast<unsigned char>(path[0])) &&
				path[1] == ':' &&
				(path[2] == '\\' || path[2] == '/')) {
				return true;
			}
		}	
		
		// 2. UNC路径：\\server\share 或 //server/share
		if (path.length() >= 2) {
			if ((path[0] == '\\' && path[1] == '\\') ||
				(path[0] == '/' && path[1] == '/')) {
				return true;
			}
		}		
		
		// 3. 以反斜杠或正斜杠开头（根目录）：\path 或 /path
		if (path[0] == '\\' || path[0] == '/') {
			return true;
		}
#else
		// Unix/Linux路径判断：以 / 开头
		if (path[0] == '/') {
			return true;
		}
#endif

		return false;
	}

};

}

}
#endif
